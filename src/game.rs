extern crate sdl2;
extern crate byteorder;

#[cfg(target_os = "emscripten")]
use emscripten::emscripten;

use self::sdl2::EventPump;
use self::sdl2::event::{Event, WindowEvent};
use self::sdl2::keyboard::Keycode;
use self::sdl2::pixels::PixelFormatEnum;
use self::sdl2::rect::Rect;
use self::sdl2::render::{Canvas, Texture};
use self::sdl2::video::Window;
use self::byteorder::{ByteOrder, LittleEndian};
use std::time;
use std::process;
use render;
use render::{RenderSize, ResolutionTarget};
use map::Map;

pub struct GameState {
	pub map: Map,
	pub render_size: RenderSize,
	pub scroll_x: isize,
	pub scroll_y: isize,
	pub frame: usize
}

pub struct RenderState {
	canvas: Canvas<Window>,
	events: EventPump,
	screen_width: usize,
	screen_height: usize,
	resolution_target: ResolutionTarget,
	dest_size: RenderSize,
	render_buf: Vec<Vec<u16>>,
	texture: Texture,

	start_time: time::Instant,
	last_elapsed_secs: u64,
	last_frames: usize
}

fn init(title: &str, target: ResolutionTarget, map: &Map) -> (GameState, RenderState) {
	let mut screen_width = 1280;
	let mut screen_height = 720;

	// Create an SDL context
	let sdl = sdl2::init().unwrap();
	let video = sdl.video().unwrap();
	let window = video.window(title, screen_width as u32, screen_height as u32).resizable().build().unwrap();
	let window_size = window.size();
	screen_width = window_size.0 as usize;
	screen_height = window_size.1 as usize;

	let canvas = window.into_canvas().accelerated().present_vsync().build().unwrap();

	// Compute internal resolution based on provided target resolution information
	let (render_size, dest_size) = target.compute_render_sizes(screen_width, screen_height);

	// Create texture for rendering each frame
	let texture = canvas.create_texture_streaming(PixelFormatEnum::RGB555,
		render_size.width as u32, render_size.height as u32).unwrap();

	// Create buffer to hold rendered pixels at internal resolution
	let mut render_buf: Vec<Vec<u16>> = Vec::new();
	for _ in 0 .. render_size.height {
		let mut line = Vec::new();
		line.resize(render_size.width, 0);
		render_buf.push(line);
	}

	let start_time = time::Instant::now();
	let last_elapsed_secs = 0;
	let last_frames = 0;

	let events = sdl.event_pump().unwrap();

	let game = GameState {
		map: map.clone(),
		render_size,
		scroll_x: 0, scroll_y: 0,
		frame: 0,
	};
	let render_state = RenderState {
		canvas, events,
		screen_width, screen_height,
		resolution_target: target,
		dest_size,
		render_buf, texture,
		start_time, last_elapsed_secs, last_frames
	};
	(game, render_state)
}

fn next_frame(game: &mut GameState, render_state: &mut RenderState) {
	#[cfg(target_os = "emscripten")] {
		let (cur_width, cur_height) = emscripten::get_canvas_size();
		if (cur_width != render_state.screen_width) || (cur_height != render_state.screen_height) {
			render_state.canvas.window_mut().set_size(cur_width as u32, cur_height as u32).unwrap();
		}
	}

	for event in render_state.events.poll_iter() {
		match event {
			Event::Quit {..} |
			Event::KeyDown {keycode: Some(Keycode::Escape), ..} =>
				process::exit(0),

			Event::Window {win_event: WindowEvent::SizeChanged(width, height), ..} => {
				render_state.screen_width = width as usize;
				render_state.screen_height = height as usize;
				let (render_size, dest_size) = render_state.resolution_target.compute_render_sizes(
					render_state.screen_width, render_state.screen_height);

				game.render_size = render_size;
				render_state.dest_size = dest_size;

				render_state.texture = render_state.canvas.create_texture_streaming(PixelFormatEnum::RGB555,
					game.render_size.width as u32, game.render_size.height as u32).unwrap();

				render_state.render_buf = Vec::new();
				for _ in 0 .. game.render_size.height {
					let mut line = Vec::new();
					line.resize(game.render_size.width, 0);
					render_state.render_buf.push(line);
				}
			},

			_ => {}
		}
	}

	// Render game at internal resolution
	render::render_frame(&game.render_size, &mut render_state.render_buf, &game);

	// Copy rendered frame into SDL texture
	let render_buf = &render_state.render_buf;
	render_state.texture.with_lock(None, |buffer: &mut [u8], pitch: usize| {
		for y in 0..game.render_size.height {
			let dest_line = &mut buffer[y * pitch .. (y * pitch) + (game.render_size.width * 2)];
			let src_line = &render_buf[y];
			LittleEndian::write_u16_into(src_line, dest_line);
		}
	}).unwrap();

	// Present frame scaled to fit screen
	render_state.canvas.clear();
	render_state.canvas.copy(&render_state.texture, None, Rect::new(
		((render_state.screen_width - render_state.dest_size.width) / 2) as i32,
		((render_state.screen_height - render_state.dest_size.height) / 2) as i32,
		render_state.dest_size.width as u32, render_state.dest_size.height as u32)).unwrap();
	render_state.canvas.present();

	game.frame += 1;

	// Track framerate
	let elapsed_time = render_state.start_time.elapsed();
	let elapsed_secs = elapsed_time.as_secs();
	if elapsed_secs != render_state.last_elapsed_secs {
		render_state.last_elapsed_secs = elapsed_secs;
		let framerate = game.frame - render_state.last_frames;
		render_state.last_frames = game.frame;
		println!("FPS: {}", framerate);
	}
}

// Emscripten is event loop based, so state must be stored as a global variable or memory
// corruption will occur
#[cfg(target_os = "emscripten")]
static mut GAME: Option<GameState> = None;
#[cfg(target_os = "emscripten")]
static mut RENDER_STATE: Option<RenderState> = None;

pub fn run(title: &str, target: ResolutionTarget, map: &Map) {
	#[cfg(target_os = "emscripten")]
	let (game, render_state) = init(title, target, map);
	#[cfg(not(target_os = "emscripten"))]
	let (mut game, mut render_state) = init(title, target, map);

	#[cfg(target_os = "emscripten")]
	unsafe {
		GAME = Some(game);
		RENDER_STATE = Some(render_state);
	}

	#[cfg(target_os = "emscripten")]
	emscripten::set_main_loop_callback(|| {
		unsafe {
			match &mut GAME {
				Some(game) => {
					match &mut RENDER_STATE {
						Some(render_state) => {
							next_frame(game, render_state);
						},
						None => panic!("Invalid render state")
					};
				}
				None => panic!("Invalid game state")
			};
		}
	});

	#[cfg(not(target_os = "emscripten"))]
	loop { next_frame(&mut game, &mut render_state); }
}
