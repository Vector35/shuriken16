use std::rc::Rc;
use game::GameState;
use map::{MapLayer, TileRef};
use tile::TileSet;

pub struct TextLayerContents {
	pub layer: MapLayer,
	pub font_tile_set: Rc<TileSet>,
	pub font_base: u8,
}

pub struct TextLayer {
	pub contents: TextLayerContents,
	pub renderer: Option<Box<TextLayerRenderer>>
}

pub trait UILayer {
	fn update(&mut self, game_state: &GameState);
	fn get_map_layer(&self) -> &MapLayer;
}

pub trait TextLayerRenderer {
	fn update(&mut self, layer: &mut TextLayerContents, game_state: &GameState);
}

impl TextLayerContents {
	pub fn width(&self) -> usize {
		self.layer.width
	}

	pub fn height(&self) -> usize {
		self.layer.height
	}

	pub fn clear(&mut self) {
		for y in 0..self.height() {
			for x in 0..self.width() {
				self.layer.set_tile(x, y, None);
			}
		}
	}

	pub fn write(&mut self, x: i32, y: i32, text: &str) {
		if (y < 0) || (y >= self.height() as i32) {
			return;
		}

		let mut cur_x = x;
		for ch in text.chars() {
			let ord = ch as u32;
			if ord < self.font_base as u32 {
				continue;
			}
			let font_index = ord - self.font_base as u32;
			if font_index as usize >= self.font_tile_set.tiles.len() {
				continue;
			}

			if (cur_x >= 0) && (cur_x < self.width() as i32) {
				self.layer.set_tile(cur_x as usize, y as usize, Some(TileRef {
					tile_set: Rc::clone(&self.font_tile_set),
					tile_index: font_index as usize
				}));
			}
			cur_x += 1;
		}
	}
}

impl TextLayer {
	pub fn new(font_tile_set: Rc<TileSet>, font_base: u8) -> TextLayer {
		TextLayer {
			contents: TextLayerContents {
				layer: MapLayer::new("Text", 0, 0, font_tile_set.width, font_tile_set.height, font_tile_set.depth),
				font_tile_set: Rc::clone(&font_tile_set),
				font_base
			},
			renderer: None
		}
	}

	pub fn width(&self) -> usize {
		self.contents.width()
	}

	pub fn height(&self) -> usize {
		self.contents.height()
	}

	pub fn clear(&mut self) {
		self.contents.clear();
	}

	pub fn write(&mut self, x: i32, y: i32, text: &str) {
		self.contents.write(x, y, text);
	}
}

impl UILayer for TextLayer {
	fn update(&mut self, game_state: &GameState) {
		let desired_width = game_state.render_size.width / self.contents.font_tile_set.width;
		let desired_height = game_state.render_size.height / self.contents.font_tile_set.height;
		if (desired_width != self.contents.layer.width) || (desired_height != self.contents.layer.height) {
			self.contents.layer.resize(desired_width, desired_height);
		}

		match &mut self.renderer {
			Some(renderer) => {
				renderer.update(&mut self.contents, game_state);
			},
			None => ()
		};
	}

	fn get_map_layer(&self) -> &MapLayer {
		&self.contents.layer
	}
}
