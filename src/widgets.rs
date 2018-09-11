extern crate sdl2;

use self::sdl2::mouse::MouseButton;
use std::rc::Rc;
use ui;
use ui::{UILayout, UILayer, UILayerRef, UILayoutRef, UILayerContents, UILayerRenderer, UIInputHandler};
use game::GameState;
use tile::TileSet;
use map::TileRef;
use actor::BoundingRect;

pub struct UILabel {
	pub text: String
}

pub struct UILabelLayout {
	pub label: UILayerRef,
	pub tile_width: isize,
	pub tile_height: isize
}

pub trait UIButtonHandler {
	fn on_click(&self, game_state: &GameState);
}

pub struct UIButton {
	pub tile_set: Rc<TileSet>
}

pub struct UIButtonInputHandler {
	pub handler: Box<UIButtonHandler>
}

pub struct UIButtonLayout {
	pub background: UILayerRef,
	pub contents: UILayoutRef,
	pub x_margin: isize,
	pub y_margin: isize,
	pub tile_width: isize,
	pub tile_height: isize,
}

impl UILayerRenderer for UILabel {
	fn update(&mut self, layer: &mut UILayerContents, _game_state: &GameState) {
		layer.clear();
		layer.write(0, 0, &self.text);
	}
}

impl UILabel {
	pub fn new_ui_layer(font_tile_set: &Rc<TileSet>, font_base: u8, text: &str) -> UILayerRef {
		let mut label = UILayer::new(font_tile_set.width, font_tile_set.height, font_tile_set.depth);
		let renderer = UILabel {
			text: text.to_string()
		};
		label.renderer = Some(Box::new(renderer));
		label.set_font(font_tile_set.clone(), font_base);
		ui::to_layer_ref(label)
	}

	pub fn new_ui_layout(font_tile_set: &Rc<TileSet>, font_base: u8, text: &str) -> UILabelLayout {
		UILabelLayout {
			label: UILabel::new_ui_layer(font_tile_set, font_base, text),
			tile_width: font_tile_set.width as isize,
			tile_height: font_tile_set.height as isize
		}
	}
}

impl UILayout for UILabelLayout {
	fn width(&self) -> Option<isize> { Some(self.tile_width * self.text().len() as isize) }
	fn height(&self) -> Option<isize> { Some(self.tile_height) }
	fn tile_width(&self) -> isize { self.tile_width }
	fn tile_height(&self) -> isize { self.tile_height }

	fn update(&self, bounds: &BoundingRect) -> BoundingRect {
		let rect = BoundingRect {
			x: bounds.x,
			y: bounds.y,
			width: self.tile_width * self.text().len() as isize,
			height: self.tile_height
		};
		self.label.borrow_mut().set_target_rect(&rect);
		rect
	}

	fn layers(&self) -> Vec<UILayerRef> { [self.label.clone()].to_vec() }
}

impl UILabelLayout {
	pub fn text(&self) -> String {
		let label = self.label.borrow();
		if let Some(renderer) = &label.renderer {
			if let Some(label) = renderer.as_any().downcast_ref::<UILabel>() {
				return label.text.clone()
			}
		}
		"".to_string()
	}
}

impl UILayerRenderer for UIButton {
	fn update(&mut self, layer: &mut UILayerContents, _game_state: &GameState) {
		layer.clear();
		layer.set_tile(0, 0, TileRef::new(&self.tile_set, 0));
		for x in 1..(layer.width() - 1) {
			layer.set_tile(x, 0, TileRef::new(&self.tile_set, 1));
		}
		let x = layer.width() - 1;
		layer.set_tile(x, 0, TileRef::new(&self.tile_set, 2));
	}
}

impl UIButton {
	pub fn new_ui_layer(tile_set: &Rc<TileSet>, handler: Box<UIButtonHandler>) -> UILayerRef {
		let mut button = UILayer::new(tile_set.width, tile_set.height, tile_set.depth);
		let renderer = UIButton {
			tile_set: tile_set.clone()
		};
		button.renderer = Some(Box::new(renderer));
		button.input_handler = Some(Box::new(UIButtonInputHandler {
			handler
		}));
		ui::to_layer_ref(button)
	}

	pub fn new_ui_layout(tile_set: &Rc<TileSet>, x_margin: isize, y_margin: isize, contents: UILayoutRef,
		handler: Box<UIButtonHandler>) -> UIButtonLayout {
		UIButtonLayout {
			background: UIButton::new_ui_layer(tile_set, handler),
			contents,
			x_margin, y_margin,
			tile_width: tile_set.width as isize,
			tile_height: tile_set.height as isize
		}
	}
}

impl UILayout for UIButtonLayout {
	fn width(&self) -> Option<isize> {
		if let Some(width) = self.contents.borrow().width() {
			let mut adjusted_width = width + self.x_margin * 2;
			if (adjusted_width % self.tile_width) != 0 {
				adjusted_width += self.tile_width - (adjusted_width % self.tile_width);
			}
			Some(adjusted_width)
		} else {
			None
		}
	}
	fn height(&self) -> Option<isize> { Some(self.tile_height) }
	fn tile_width(&self) -> isize { self.tile_width }
	fn tile_height(&self) -> isize { self.tile_height }

	fn update(&self, bounds: &BoundingRect) -> BoundingRect {
		self.background.borrow_mut().set_target_rect(bounds);

		let rect;
		if let Some(width) = self.contents.borrow().width() {
			rect = BoundingRect {
				x: bounds.x + self.x_margin + ((bounds.width - self.x_margin * 2) - width) / 2,
				y: bounds.y + self.y_margin,
				width,
				height: self.tile_height - self.y_margin * 2
			};
		} else {
			rect = BoundingRect {
				x: bounds.x + self.x_margin,
				y: bounds.y + self.y_margin,
				width: bounds.width - self.x_margin * 2,
				height: self.tile_height - self.y_margin * 2
			};
		}
		self.contents.borrow().update(&rect);
		bounds.clone()
	}

	fn layers(&self) -> Vec<UILayerRef> {
		let mut contents_layers = self.contents.borrow().layers();
		let mut result = Vec::new();
		result.push(self.background.clone());
		result.append(&mut contents_layers);
		result
	}
}

impl UIInputHandler for UIButtonInputHandler {
	fn on_mouse_button_down(&self, _x: isize, _y: isize, button: MouseButton, game_state: &GameState) {
		if button == MouseButton::Left {
			self.handler.on_click(game_state);
		}
	}
}
