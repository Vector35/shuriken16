extern crate sdl2;

use self::sdl2::mouse::MouseButton;
use std::rc::Rc;
use ui;
use ui::{UILayout, UILayer, UILayerRef, UILayoutRef, UILayerContents, UILayerRenderer, UIInputHandler};
use game::GameState;
use tile::TileSet;
use map::TileRef;
use actor::BoundingRect;

pub trait UIDynamicText {
	fn text(&self) -> String;
}

pub struct UILabel {
	pub text: String,
	pub dynamic_text: Option<Box<UIDynamicText>>
}

pub struct UILabelLayout {
	pub label: UILayerRef,
	pub tile_width: isize,
	pub tile_height: isize
}

pub struct UIWrappingLabel {
	pub lines: Vec<String>,
	pub width: usize,
	pub dynamic_text: Option<Box<UIDynamicText>>
}

pub struct UIWrappingLabelLayout {
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
		if let Some(dynamic_text) = &self.dynamic_text {
			let text = dynamic_text.text();
			layer.write(0, 0, &text);
		} else {
			layer.write(0, 0, &self.text);
		}
	}
}

impl UILabel {
	pub fn new_ui_layer(font_tile_set: &Rc<TileSet>, font_base: u8, text: &str) -> UILayerRef {
		let mut label = UILayer::new(font_tile_set.width, font_tile_set.height, font_tile_set.depth);
		let renderer = UILabel {
			text: text.to_string(),
			dynamic_text: None
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

	pub fn new_dynamic_ui_layer(font_tile_set: &Rc<TileSet>, font_base: u8, dynamic_text: Box<UIDynamicText>) -> UILayerRef {
		let mut label = UILayer::new(font_tile_set.width, font_tile_set.height, font_tile_set.depth);
		let renderer = UILabel {
			text: "".to_string(),
			dynamic_text: Some(dynamic_text)
		};
		label.renderer = Some(Box::new(renderer));
		label.set_font(font_tile_set.clone(), font_base);
		ui::to_layer_ref(label)
	}

	pub fn new_dynamic_ui_layout(font_tile_set: &Rc<TileSet>, font_base: u8, dynamic_text: Box<UIDynamicText>) -> UILabelLayout {
		UILabelLayout {
			label: UILabel::new_dynamic_ui_layer(font_tile_set, font_base, dynamic_text),
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
				if let Some(dynamic_text) = &label.dynamic_text {
					return dynamic_text.text()
				} else {
					return label.text.clone()
				}
			}
		}
		"".to_string()
	}
}

impl UILayerRenderer for UIWrappingLabel {
	fn update(&mut self, layer: &mut UILayerContents, _game_state: &GameState) {
		layer.clear();
		if let Some(dynamic_text) = &self.dynamic_text {
			let text = dynamic_text.text();
			let lines = UIWrappingLabel::wrap_text(&text, self.width);
			for (i, line) in lines.iter().enumerate() {
				layer.write(0, i as isize, &line);
			}
		} else {
			for (i, line) in self.lines.iter().enumerate() {
				layer.write(0, i as isize, &line);
			}
		}
	}
}

impl UIWrappingLabel {
	pub fn wrap_text(text: &str, width: usize) -> Vec<String> {
		let mut lines = Vec::new();
		let mut cur_line = "".to_string();
		let mut pending_word = "".to_string();
		let mut x = 0;
		for ch in text.chars() {
			if ch == '\n' {
				cur_line.push_str(&pending_word);
				lines.push(cur_line);
				cur_line = "".to_string();
				pending_word = "".to_string();
				x = 0;
			} else if (ch == ' ') || (ch == '\t') {
				cur_line.push_str(&pending_word);
				cur_line.push(ch);
				pending_word = "".to_string();
				if ch == '\t' {
					x += 4 - (x % 4);
				} else {
					x += 1;
				}
			} else if (cur_line.len() > 0) && (x >= width) {
				pending_word.push(ch);
				lines.push(cur_line);
				cur_line = "".to_string();
				x = pending_word.len();
			} else {
				pending_word.push(ch);
				x += 1;
			}
		}
		cur_line.push_str(&pending_word);
		if cur_line.len() > 0 {
			lines.push(cur_line);
		}
		lines
	}

	pub fn new_ui_layer(font_tile_set: &Rc<TileSet>, font_base: u8, text: &str, width: usize) -> UILayerRef {
		let mut label = UILayer::new(font_tile_set.width, font_tile_set.height, font_tile_set.depth);
		let renderer = UIWrappingLabel {
			lines: UIWrappingLabel::wrap_text(text, width),
			width,
			dynamic_text: None
		};
		label.renderer = Some(Box::new(renderer));
		label.set_font(font_tile_set.clone(), font_base);
		ui::to_layer_ref(label)
	}

	pub fn new_ui_layout(font_tile_set: &Rc<TileSet>, font_base: u8, text: &str, width: usize) -> UIWrappingLabelLayout {
		UIWrappingLabelLayout {
			label: UIWrappingLabel::new_ui_layer(font_tile_set, font_base, text, width),
			tile_width: font_tile_set.width as isize,
			tile_height: font_tile_set.height as isize
		}
	}

	pub fn new_dynamic_ui_layer(font_tile_set: &Rc<TileSet>, font_base: u8,
		dynamic_text: Box<UIDynamicText>, width: usize) -> UILayerRef {
		let mut label = UILayer::new(font_tile_set.width, font_tile_set.height, font_tile_set.depth);
		let renderer = UIWrappingLabel {
			lines: Vec::new(),
			width,
			dynamic_text: Some(dynamic_text)
		};
		label.renderer = Some(Box::new(renderer));
		label.set_font(font_tile_set.clone(), font_base);
		ui::to_layer_ref(label)
	}

	pub fn new_dynamic_ui_layout(font_tile_set: &Rc<TileSet>, font_base: u8,
		dynamic_text: Box<UIDynamicText>, width: usize) -> UIWrappingLabelLayout {
		UIWrappingLabelLayout {
			label: UIWrappingLabel::new_dynamic_ui_layer(font_tile_set, font_base, dynamic_text, width),
			tile_width: font_tile_set.width as isize,
			tile_height: font_tile_set.height as isize
		}
	}
}

impl UILayout for UIWrappingLabelLayout {
	fn width(&self) -> Option<isize> {
		let mut width = 0;
		for line in self.lines() {
			let mut x = 0;
			for ch in line.chars() {
				if ch == '\t' {
					x += 4 - (x % 4);
				} else {
					x += 1;
				}
			}
			if x > width {
				width = x;
			}
		}
		Some(self.tile_width * width as isize)
	}

	fn height(&self) -> Option<isize> {
		Some(self.tile_height * self.lines().len() as isize)
	}

	fn tile_width(&self) -> isize { self.tile_width }
	fn tile_height(&self) -> isize { self.tile_height }

	fn update(&self, bounds: &BoundingRect) -> BoundingRect {
		let width = match self.width() {
			Some(width) => width,
			None => 0
		};
		let height = match self.height() {
			Some(height) => height,
			None => 0
		};
		let rect = BoundingRect {
			x: bounds.x,
			y: bounds.y,
			width,
			height
		};
		self.label.borrow_mut().set_target_rect(&rect);
		rect
	}

	fn layers(&self) -> Vec<UILayerRef> { [self.label.clone()].to_vec() }
}

impl UIWrappingLabelLayout {
	pub fn lines(&self) -> Vec<String> {
		let label = self.label.borrow();
		if let Some(renderer) = &label.renderer {
			if let Some(label) = renderer.as_any().downcast_ref::<UIWrappingLabel>() {
				if let Some(dynamic_text) = &label.dynamic_text {
					let text = dynamic_text.text();
					return UIWrappingLabel::wrap_text(&text, label.width);
				} else {
					return label.lines.clone()
				}
			}
		}
		Vec::new()
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
