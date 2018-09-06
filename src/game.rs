extern crate sdl2;
extern crate byteorder;

#[cfg(target_os = "emscripten")]
use emscripten::emscripten;

use self::sdl2::EventPump;
use self::sdl2::event::{Event, WindowEvent};
use self::sdl2::keyboard::Keycode;
use self::sdl2::mouse::MouseButton;
use self::sdl2::joystick::{Joystick, HatState};
use self::sdl2::pixels::PixelFormatEnum;
use self::sdl2::rect::Rect;
use self::sdl2::render::{Canvas, Texture};
use self::sdl2::video::Window;
use self::byteorder::{ByteOrder, LittleEndian};
use std::process;
use std::cell::RefCell;
use std::collections::HashMap;
use std::rc::Rc;
use render;
use render::{RenderSize, ResolutionTarget};
use map::{Map, MapActor};
use ui::{UILayoutRef, UILayerRef};
use actor::{Actor, ActorRef};
use camera::Camera;
use asset::AssetNamespace;

pub struct HatBindings {
	left: String,
	right: String,
	up: String,
	down: String
}

#[derive(Clone)]
pub struct MapChangeEvent {
	map: Rc<Map>
}

#[derive(Clone)]
pub struct AddActorEvent {
	actor: ActorRef
}

#[derive(Clone)]
pub struct AddUILayoutEvent {
	layout: UILayoutRef
}

#[derive(Clone)]
pub struct RemoveUILayoutEvent {
	layout: UILayoutRef
}

#[derive(Clone)]
pub enum PendingEvent {
	MapChange(MapChangeEvent),
	AddActor(AddActorEvent),
	AddUILayout(AddUILayoutEvent),
	RemoveUILayout(RemoveUILayoutEvent),
	FadeOut,
	FadeIn
}

pub struct GameState {
	pub assets: AssetNamespace,
	pub map: Option<Map>,
	pub ui_layouts: Vec<UILayoutRef>,
	pub actors: Vec<ActorRef>,
	pub persistent_actors: Vec<ActorRef>,
	pub controlled_actor: Option<ActorRef>,
	pub camera: Option<Camera>,
	pub render_size: RenderSize,
	pub scroll_x: isize,
	pub scroll_y: isize,
	pub fade_alpha: u8,
	pub target_fade_alpha: u8,
	pub frame: usize,
	pub key_bindings: HashMap<Keycode, String>,
	pub axis_bindings: HashMap<u8, String>,
	pub button_bindings: HashMap<u8, String>,
	pub hat_bindings: HashMap<u8, HatBindings>,
	pub ui_key_bindings: HashMap<Keycode, String>,
	pub ui_axis_bindings: HashMap<u8, String>,
	pub ui_button_bindings: HashMap<u8, String>,
	pub ui_hat_bindings: HashMap<u8, HatBindings>,
	pub actor_loaders: HashMap<String, Box<Fn(&MapActor, &AssetNamespace) -> Option<Box<Actor>>>>,
	pub pending_events: RefCell<Vec<PendingEvent>>
}

pub struct RenderState {
	canvas: Canvas<Window>,
	events: EventPump,
	_joystick: Option<Joystick>,
	screen_width: usize,
	screen_height: usize,
	resolution_target: ResolutionTarget,
	dest_size: RenderSize,
	render_buf: Vec<Vec<u32>>,
	texture: Texture,
}

pub trait Game {
	fn init(&mut self, game_state: &mut GameState);
	fn title(&self) -> String;
	fn target_resolution(&self) -> ResolutionTarget;
	fn fade_in_on_start(&self) -> bool { true }

	fn tick(&mut self) {}
}

impl GameState {
	pub fn add_ui_layout(&self, layout: UILayoutRef) {
		self.pending_events.borrow_mut().push(PendingEvent::AddUILayout(AddUILayoutEvent {
			layout
		}));
	}

	pub fn remove_ui_layout(&self, layout: UILayoutRef) {
		self.pending_events.borrow_mut().push(PendingEvent::RemoveUILayout(RemoveUILayoutEvent {
			layout
		}));
	}

	pub fn is_ui_layout_present(&self, layout: &UILayoutRef) -> bool {
		for check in &self.ui_layouts {
			if Rc::ptr_eq(check, layout) {
				return true;
			}
		}
		false
	}

	pub fn add_actor(&self, actor: Box<Actor>) -> ActorRef {
		let actor_ref = ActorRef::new(RefCell::new(actor));
		self.pending_events.borrow_mut().push(PendingEvent::AddActor(AddActorEvent {
			actor: actor_ref.clone()
		}));
		actor_ref
	}

	pub fn add_persistent_actor(&mut self, actor: Box<Actor>) -> ActorRef {
		let actor_ref = ActorRef::new(RefCell::new(actor));
		self.actors.push(actor_ref.clone());
		self.persistent_actors.push(actor_ref.clone());
		actor_ref
	}

	pub fn register_actor_loader(&mut self, name: &str,
		handler: Box<Fn(&MapActor, &AssetNamespace) -> Option<Box<Actor>>>) {
		self.actor_loaders.insert(name.to_string(), handler);
	}

	fn load_map_now(&mut self, map: &Map) {
		self.actors.clear();
		self.map = Some(map.clone());
		for actor in &map.actors {
			if let Some(handler) = self.actor_loaders.get(&actor.actor_type) {
				if let Some(actor) = handler(actor, &self.assets) {
					self.actors.push(ActorRef::new(RefCell::new(actor)));
				}
			}
		}
	}

	pub fn load_map(&self, map: &Rc<Map>) {
		self.pending_events.borrow_mut().push(PendingEvent::MapChange(MapChangeEvent {
			map: map.clone(),
		}));
	}

	pub fn unload_map(&mut self) {
		self.actors.clear();
		self.map = None;
	}

	pub fn fade_out(&self) {
		self.pending_events.borrow_mut().push(PendingEvent::FadeOut);
	}

	pub fn fade_in(&self) {
		self.pending_events.borrow_mut().push(PendingEvent::FadeIn);
	}

	pub fn bind_key(&mut self, key: Keycode, button: &str) {
		self.key_bindings.insert(key, button.to_string());
	}

	pub fn bind_axis(&mut self, axis: u8, name: &str) {
		self.axis_bindings.insert(axis, name.to_string());
	}

	pub fn bind_button(&mut self, button: u8, name: &str) {
		self.button_bindings.insert(button, name.to_string());
	}

	pub fn bind_hat(&mut self, hat: u8, left: &str, right: &str, up: &str, down: &str) {
		self.hat_bindings.insert(hat, HatBindings {
			left: left.to_string(),
			right: right.to_string(),
			up: up.to_string(),
			down: down.to_string()
		});
	}

	pub fn bind_ui_key(&mut self, key: Keycode, button: &str) {
		self.ui_key_bindings.insert(key, button.to_string());
	}

	pub fn bind_ui_axis(&mut self, axis: u8, name: &str) {
		self.ui_axis_bindings.insert(axis, name.to_string());
	}

	pub fn bind_ui_button(&mut self, button: u8, name: &str) {
		self.ui_button_bindings.insert(button, name.to_string());
	}

	pub fn bind_ui_hat(&mut self, hat: u8, left: &str, right: &str, up: &str, down: &str) {
		self.ui_hat_bindings.insert(hat, HatBindings {
			left: left.to_string(),
			right: right.to_string(),
			up: up.to_string(),
			down: down.to_string()
		});
	}

	pub fn set_controlled_actor(&mut self, actor: &ActorRef) {
		self.controlled_actor = Some(actor.clone());
	}

	pub fn set_camera(&mut self, camera: Option<Camera>) {
		self.camera = camera;
	}

	fn get_ui_input_layers(&self) -> Vec<UILayerRef> {
		// Check layouts from top to bottom for input handling
		for layout in self.ui_layouts.iter().rev() {
			let layers = layout.borrow().layers();
			let mut input_layers = Vec::new();
			for layer in layers {
				if layer.borrow().input_handler.is_some() {
					input_layers.push(layer);
				}
			}
			if input_layers.len() > 0 {
				return input_layers;
			}
		}
		Vec::new()
	}

	fn key_down(&self, key: Keycode) {
		let ui_input_layers = self.get_ui_input_layers();
		if ui_input_layers.len() > 0 {
			// Direct input at active UI handlers
			let action = if let Some(action) = self.ui_key_bindings.get(&key) {
				Some(action.clone())
			} else if let Some(action) = self.key_bindings.get(&key) {
				Some(action.clone())
			} else {
				None
			};

			if let Some(action) = action {
				for layer in ui_input_layers {
					let layer_ref = layer.borrow();
					if let Some(input_handler) = &layer_ref.input_handler {
						input_handler.on_button_down(&action, &self);
					}
				}
			}
			return;
		}

		if let Some(action) = self.key_bindings.get(&key) {
			if let Some(actor) = &self.controlled_actor {
				actor.borrow_mut().on_button_down(action, &self);
			}
		}
	}

	fn key_up(&self, key: Keycode) {
		let ui_input_layers = self.get_ui_input_layers();
		if ui_input_layers.len() > 0 {
			// Direct input at active UI handlers
			let action = if let Some(action) = self.ui_key_bindings.get(&key) {
				Some(action.clone())
			} else if let Some(action) = self.key_bindings.get(&key) {
				Some(action.clone())
			} else {
				None
			};

			if let Some(action) = action {
				for layer in ui_input_layers {
					let layer_ref = layer.borrow();
					if let Some(input_handler) = &layer_ref.input_handler {
						input_handler.on_button_up(&action, &self);
					}
				}
			}
			return;
		}

		if let Some(action) = self.key_bindings.get(&key) {
			if let Some(actor) = &self.controlled_actor {
				actor.borrow_mut().on_button_up(action, &self);
			}
		}
	}

	fn axis_changed(&self, axis: u8, value: i16) {
		let mut adjusted_value = 0.0;
		if value < -0x7000 {
			adjusted_value = -1.0;
		} else if value < -0x1000 {
			adjusted_value = (value + 0x1000) as f32 / 0x6000 as f32;
		} else if value > 0x7000 {
			adjusted_value = 1.0;
		} else if value > 0x1000 {
			adjusted_value = (value - 0x1000) as f32 / 0x6000 as f32;
		}

		let ui_input_layers = self.get_ui_input_layers();
		if ui_input_layers.len() > 0 {
			// Direct input at active UI handlers
			let action = if let Some(action) = self.ui_axis_bindings.get(&axis) {
				Some(action.clone())
			} else if let Some(action) = self.axis_bindings.get(&axis) {
				Some(action.clone())
			} else {
				None
			};

			if let Some(action) = action {
				for layer in ui_input_layers {
					let layer_ref = layer.borrow();
					if let Some(input_handler) = &layer_ref.input_handler {
						input_handler.on_axis_changed(&action, adjusted_value, &self);
					}
				}
			}
			return;
		}

		if let Some(action) = self.axis_bindings.get(&axis) {
			if let Some(actor) = &self.controlled_actor {
				actor.borrow_mut().on_axis_changed(action, adjusted_value, &self);
			}
		}
	}

	fn button_down(&self, button: u8) {
		let ui_input_layers = self.get_ui_input_layers();
		if ui_input_layers.len() > 0 {
			// Direct input at active UI handlers
			let action = if let Some(action) = self.ui_button_bindings.get(&button) {
				Some(action.clone())
			} else if let Some(action) = self.button_bindings.get(&button) {
				Some(action.clone())
			} else {
				None
			};

			if let Some(action) = action {
				for layer in ui_input_layers {
					let layer_ref = layer.borrow();
					if let Some(input_handler) = &layer_ref.input_handler {
						input_handler.on_button_down(&action, &self);
					}
				}
			}
			return;
		}

		if let Some(action) = self.button_bindings.get(&button) {
			if let Some(actor) = &self.controlled_actor {
				actor.borrow_mut().on_button_down(action, &self);
			}
		}
	}

	fn button_up(&self, button: u8) {
		let ui_input_layers = self.get_ui_input_layers();
		if ui_input_layers.len() > 0 {
			// Direct input at active UI handlers
			let action = if let Some(action) = self.ui_button_bindings.get(&button) {
				Some(action.clone())
			} else if let Some(action) = self.button_bindings.get(&button) {
				Some(action.clone())
			} else {
				None
			};

			if let Some(action) = action {
				for layer in ui_input_layers {
					let layer_ref = layer.borrow();
					if let Some(input_handler) = &layer_ref.input_handler {
						input_handler.on_button_up(&action, &self);
					}
				}
			}
			return;
		}

		if let Some(action) = self.button_bindings.get(&button) {
			if let Some(actor) = &self.controlled_actor {
				actor.borrow_mut().on_button_up(action, &self);
			}
		}
	}

	fn hat_changed(&self, hat: u8, state: HatState) {
		let left = match state {
			HatState::Left => true,
			HatState::LeftUp => true,
			HatState::LeftDown => true,
			_ => false
		};
		let right = match state {
			HatState::Right => true,
			HatState::RightUp => true,
			HatState::RightDown => true,
			_ => false
		};
		let up = match state {
			HatState::Up => true,
			HatState::LeftUp => true,
			HatState::RightUp => true,
			_ => false
		};
		let down = match state {
			HatState::Down => true,
			HatState::LeftDown => true,
			HatState::RightDown => true,
			_ => false
		};

		let ui_input_layers = self.get_ui_input_layers();
		if ui_input_layers.len() > 0 {
			// Direct input at active UI handlers
			let action = if let Some(action) = self.ui_hat_bindings.get(&hat) {
				Some(action.clone())
			} else if let Some(action) = self.hat_bindings.get(&hat) {
				Some(action.clone())
			} else {
				None
			};

			if let Some(action) = action {
				for layer in ui_input_layers {
					let layer_ref = layer.borrow();
					if let Some(input_handler) = &layer_ref.input_handler {
						if left {
							input_handler.on_button_down(&action.left, &self);
						} else {
							input_handler.on_button_up(&action.left, &self);
						}
						if right {
							input_handler.on_button_down(&action.right, &self);
						} else {
							input_handler.on_button_up(&action.right, &self);
						}
						if up {
							input_handler.on_button_down(&action.up, &self);
						} else {
							input_handler.on_button_up(&action.up, &self);
						}
						if down {
							input_handler.on_button_down(&action.down, &self);
						} else {
							input_handler.on_button_up(&action.down, &self);
						}
					}
				}
			}
			return;
		}

		if let Some(action) = self.hat_bindings.get(&hat) {
			if let Some(actor) = &self.controlled_actor {
				let mut actor_ref = actor.borrow_mut();
				if left {
					actor_ref.on_button_down(&action.left, &self);
				} else {
					actor_ref.on_button_up(&action.left, &self);
				}
				if right {
					actor_ref.on_button_down(&action.right, &self);
				} else {
					actor_ref.on_button_up(&action.right, &self);
				}
				if up {
					actor_ref.on_button_down(&action.up, &self);
				} else {
					actor_ref.on_button_up(&action.up, &self);
				}
				if down {
					actor_ref.on_button_down(&action.down, &self);
				} else {
					actor_ref.on_button_up(&action.down, &self);
				}
			}
		}
	}

	fn convert_mouse_pos(&self, x: isize, y: isize, screen_width: usize, screen_height: usize,
		dest_size: &RenderSize) -> (isize, isize) {
		let x_offset = ((screen_width - dest_size.width) / 2) as isize;
		let y_offset = ((screen_height - dest_size.height) / 2) as isize;
		let dest_x = x - x_offset;
		let dest_y = y - y_offset;
		let final_x = (dest_x * self.render_size.width as isize) / dest_size.width as isize;
		let final_y = (dest_y * self.render_size.height as isize) / dest_size.height as isize;
		(final_x, final_y)
	}

	fn mouse_button_down(&self, dest_x: isize, dest_y: isize, button: MouseButton,
		screen_width: usize, screen_height: usize, dest_size: &RenderSize) {
		let (x, y) = self.convert_mouse_pos(dest_x, dest_y, screen_width, screen_height, dest_size);
		let ui_input_layers = self.get_ui_input_layers();
		if ui_input_layers.len() > 0 {
			// Direct input at active UI handlers
			for layer in ui_input_layers {
				let layer_ref = layer.borrow();
				if (x < layer_ref.bounds.x) || (x >= (layer_ref.bounds.x + layer_ref.bounds.width)) ||
					(y < layer_ref.bounds.y) || (y >= (layer_ref.bounds.y + layer_ref.bounds.height)) {
					continue;
				}
				if let Some(input_handler) = &layer_ref.input_handler {
					input_handler.on_mouse_button_down(x - layer_ref.bounds.x, y - layer_ref.bounds.y, button, &self);
				}
			}
			return;
		}
	}

	fn mouse_button_up(&self, dest_x: isize, dest_y: isize, button: MouseButton,
		screen_width: usize, screen_height: usize, dest_size: &RenderSize) {
		let (x, y) = self.convert_mouse_pos(dest_x, dest_y, screen_width, screen_height, dest_size);
		let ui_input_layers = self.get_ui_input_layers();
		if ui_input_layers.len() > 0 {
			// Direct input at active UI handlers
			for layer in ui_input_layers {
				let layer_ref = layer.borrow();
				if (x < layer_ref.bounds.x) || (x >= (layer_ref.bounds.x + layer_ref.bounds.width)) ||
					(y < layer_ref.bounds.y) || (y >= (layer_ref.bounds.y + layer_ref.bounds.height)) {
					continue;
				}
				if let Some(input_handler) = &layer_ref.input_handler {
					input_handler.on_mouse_button_up(x - layer_ref.bounds.x, y - layer_ref.bounds.y, button, &self);
				}
			}
			return;
		}
	}

	fn mouse_move(&self, dest_x: isize, dest_y: isize, screen_width: usize, screen_height: usize, dest_size: &RenderSize) {
		let (x, y) = self.convert_mouse_pos(dest_x, dest_y, screen_width, screen_height, dest_size);
		let ui_input_layers = self.get_ui_input_layers();
		if ui_input_layers.len() > 0 {
			// Direct input at active UI handlers
			for layer in ui_input_layers {
				let layer_ref = layer.borrow();
				if (x < layer_ref.bounds.x) || (x >= (layer_ref.bounds.x + layer_ref.bounds.width)) ||
					(y < layer_ref.bounds.y) || (y >= (layer_ref.bounds.y + layer_ref.bounds.height)) {
					continue;
				}
				if let Some(input_handler) = &layer_ref.input_handler {
					input_handler.on_mouse_move(x - layer_ref.bounds.x, y - layer_ref.bounds.y, &self);
				}
			}
			return;
		}
	}

	fn get_pending_events(&mut self) -> Vec<PendingEvent> {
		let mut pending_events = self.pending_events.borrow_mut();
		let result = pending_events.clone();
		pending_events.clear();
		result
	}
}

fn init(title: &str, target: ResolutionTarget, game: &Box<Game>) -> (GameState, RenderState) {
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
	let texture = canvas.create_texture_streaming(PixelFormatEnum::RGB888,
		render_size.width as u32, render_size.height as u32).unwrap();

	// Create buffer to hold rendered pixels at internal resolution
	let mut render_buf: Vec<Vec<u32>> = Vec::new();
	for _ in 0 .. render_size.height {
		let mut line = Vec::new();
		line.resize(render_size.width, 0);
		render_buf.push(line);
	}

	let joystick_subsys = sdl.joystick().unwrap();
	let mut joystick = None;
	if let Ok(joystick_count) = joystick_subsys.num_joysticks() {
		for i in 0..joystick_count {
			if let Ok(js) = joystick_subsys.open(i) {
				joystick = Some(js);
				break;
			}
		}
	}

	let events = sdl.event_pump().unwrap();

	let game = GameState {
		assets: AssetNamespace::new(),
		map: None,
		ui_layouts: Vec::new(),
		actors: Vec::new(),
		persistent_actors: Vec::new(),
		controlled_actor: None,
		camera: None,
		render_size,
		scroll_x: 0, scroll_y: 0,
		fade_alpha: match game.fade_in_on_start() {
			true => 16,
			false => 0
		},
		target_fade_alpha: 0,
		frame: 0,
		key_bindings: HashMap::new(),
		axis_bindings: HashMap::new(),
		button_bindings: HashMap::new(),
		hat_bindings: HashMap::new(),
		ui_key_bindings: HashMap::new(),
		ui_axis_bindings: HashMap::new(),
		ui_button_bindings: HashMap::new(),
		ui_hat_bindings: HashMap::new(),
		actor_loaders: HashMap::new(),
		pending_events: RefCell::new(Vec::new())
	};
	let render_state = RenderState {
		canvas, events, _joystick: joystick,
		screen_width, screen_height,
		resolution_target: target,
		dest_size,
		render_buf, texture,
	};
	(game, render_state)
}

fn next_frame(game: &mut Box<Game>, game_state: &mut GameState, render_state: &mut RenderState) {
	#[cfg(target_os = "emscripten")] {
		let (cur_width, cur_height) = emscripten::get_canvas_size();
		if (cur_width != render_state.screen_width) || (cur_height != render_state.screen_height) {
			render_state.canvas.window_mut().set_size(cur_width as u32, cur_height as u32).unwrap();
		}
	}

	for event in render_state.events.poll_iter() {
		match event {
			Event::Quit {..} => process::exit(0),

			Event::KeyDown {keycode: Some(keycode), ..} =>
				game_state.key_down(keycode),
			Event::KeyUp {keycode: Some(keycode), ..} =>
				game_state.key_up(keycode),

			Event::JoyAxisMotion {axis_idx, value, ..} =>
				game_state.axis_changed(axis_idx, value),
			Event::JoyButtonDown {button_idx, ..} =>
				game_state.button_down(button_idx),
			Event::JoyButtonUp {button_idx, ..} =>
				game_state.button_up(button_idx),
			Event::JoyHatMotion {hat_idx, state, ..} =>
				game_state.hat_changed(hat_idx, state),

			Event::MouseButtonDown {x, y, mouse_btn, ..} =>
				game_state.mouse_button_down(x as isize, y as isize, mouse_btn, render_state.screen_width,
					render_state.screen_height, &render_state.dest_size),
			Event::MouseButtonUp {x, y, mouse_btn, ..} =>
				game_state.mouse_button_up(x as isize, y as isize, mouse_btn, render_state.screen_width,
					render_state.screen_height, &render_state.dest_size),
			Event::MouseMotion {x, y, ..} =>
				game_state.mouse_move(x as isize, y as isize, render_state.screen_width,
					render_state.screen_height, &render_state.dest_size),

			Event::Window {win_event: WindowEvent::SizeChanged(width, height), ..} => {
				render_state.screen_width = width as usize;
				render_state.screen_height = height as usize;
				let (render_size, dest_size) = render_state.resolution_target.compute_render_sizes(
					render_state.screen_width, render_state.screen_height);

				game_state.render_size = render_size;
				render_state.dest_size = dest_size;

				render_state.texture = render_state.canvas.create_texture_streaming(PixelFormatEnum::RGB888,
					game_state.render_size.width as u32, game_state.render_size.height as u32).unwrap();

				render_state.render_buf = Vec::new();
				for _ in 0 .. game_state.render_size.height {
					let mut line = Vec::new();
					line.resize(game_state.render_size.width, 0);
					render_state.render_buf.push(line);
				}
			},

			_ => {}
		}
	}

	// Handle events that were created by actor processing
	let event_list = game_state.get_pending_events();
	for event in event_list {
		match event {
			PendingEvent::MapChange(map_change) => {
				game_state.load_map_now(&map_change.map);
				for actor in &game_state.persistent_actors {
					game_state.actors.push(actor.clone());
				}
				for actor_ref in &game_state.actors {
					let mut actor = actor_ref.borrow_mut();
					actor.init(&game_state);
				}
				if let Some(camera) = &mut game_state.camera {
					camera.force_snap = true;
				}
			},
			PendingEvent::AddActor(add_actor) => {
				game_state.actors.push(add_actor.actor);
			},
			PendingEvent::AddUILayout(add_ui_layout) => {
				game_state.ui_layouts.push(add_ui_layout.layout);
			},
			PendingEvent::RemoveUILayout(remove_ui_layout) => {
				let mut new_ui_layouts = Vec::new();
				for layout in &game_state.ui_layouts {
					if !Rc::ptr_eq(layout, &remove_ui_layout.layout) {
						new_ui_layouts.push(layout.clone());
					}
				}
				game_state.ui_layouts = new_ui_layouts;
			},
			PendingEvent::FadeIn => {
				game_state.target_fade_alpha = 0;
			},
			PendingEvent::FadeOut => {
				game_state.target_fade_alpha = 16;
			}
		};
	}

	game.tick();

	// Advance sprite animations
	for actor in &game_state.actors {
		let mut actor_ref = actor.borrow_mut();
		let actor_info = actor_ref.actor_info_mut();
		for sprite in &mut actor_info.sprites {
			sprite.animation_frame += 1;
		}
	}

	// Tick actors, and keep a list of non-destroyed actors to replace the
	// actor list with afterwards
	let mut new_actor_list: Vec<ActorRef> = Vec::new();
	for actor in &game_state.actors {
		let mut actor_ref = actor.borrow_mut();
		if actor_ref.is_destroyed() {
			continue;
		}
		new_actor_list.push(actor.clone());
		actor_ref.tick(game_state);
	}

	// Replace actor list with destroyed actors removed
	game_state.actors = new_actor_list;

	// Update camera state
	if let Some(camera) = &mut game_state.camera {
		camera.tick(&game_state.render_size, &mut game_state.scroll_x, &mut game_state.scroll_y);
	}

	// Process fade animation
	if game_state.fade_alpha < game_state.target_fade_alpha {
		game_state.fade_alpha += 1;
	} else if game_state.fade_alpha > game_state.target_fade_alpha {
		game_state.fade_alpha -= 1;
	}

	// Render game at internal resolution
	render::render_frame(&game_state.render_size, &mut render_state.render_buf, &game_state);

	// Copy rendered frame into SDL texture
	let render_buf = &render_state.render_buf;
	render_state.texture.with_lock(None, |buffer: &mut [u8], pitch: usize| {
		for y in 0..game_state.render_size.height {
			let dest_line = &mut buffer[y * pitch .. (y * pitch) + (game_state.render_size.width * 4)];
			let src_line = &render_buf[y];
			LittleEndian::write_u32_into(src_line, dest_line);
		}
	}).unwrap();

	// Present frame scaled to fit screen
	render_state.canvas.clear();
	render_state.canvas.copy(&render_state.texture, None, Rect::new(
		((render_state.screen_width - render_state.dest_size.width) / 2) as i32,
		((render_state.screen_height - render_state.dest_size.height) / 2) as i32,
		render_state.dest_size.width as u32, render_state.dest_size.height as u32)).unwrap();
	render_state.canvas.present();

	game_state.frame += 1;
}

// Emscripten is event loop based, so state must be stored as a global variable or memory
// corruption will occur
#[cfg(target_os = "emscripten")]
static mut GAME: Option<Box<Game>> = None;
#[cfg(target_os = "emscripten")]
static mut GAME_STATE: Option<GameState> = None;
#[cfg(target_os = "emscripten")]
static mut RENDER_STATE: Option<RenderState> = None;

#[allow(unused_mut)] // Emscripten-only warning
pub fn run(mut game: Box<Game>) {
	let title = game.title();
	let target = game.target_resolution();

	// Initialize SDL and game state
	#[cfg(target_os = "emscripten")]
	let (game_state, render_state) = init(&title, target, &game);
	#[cfg(not(target_os = "emscripten"))]
	let (mut game_state, mut render_state) = init(&title, target, &game);

	// For Emscripten target, store state in a global variable to avoid use of freed memory
	#[cfg(target_os = "emscripten")]
	unsafe {
		GAME = Some(game);
		GAME_STATE = Some(game_state);
		RENDER_STATE = Some(render_state);
	}

	// Let game initialize
	#[cfg(target_os = "emscripten")]
	unsafe {
		if let Some(game) = &mut GAME {
			if let Some(game_state) = &mut GAME_STATE {
				game.init(game_state);
			}
		}
	}
	#[cfg(not(target_os = "emscripten"))]
	game.init(&mut game_state);

	// Start main loop
	#[cfg(target_os = "emscripten")]
	emscripten::set_main_loop_callback(|| {
		unsafe {
			if let Some(game) = &mut GAME {
				if let Some(game_state) = &mut GAME_STATE {
					if let Some(render_state) = &mut RENDER_STATE {
						next_frame(game, game_state, render_state);
					}
				}
			}
		}
	});

	#[cfg(not(target_os = "emscripten"))]
	loop { next_frame(&mut game, &mut game_state, &mut render_state); }
}
