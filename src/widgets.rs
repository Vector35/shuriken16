use std::rc::Rc;
use ui;
use ui::{UILayout, UILayer, UILayerRef, UILayerContents, UILayerRenderer};
use game::GameState;
use tile::TileSet;
use actor::BoundingRect;

pub struct UILabel {
	pub text: String
}

pub struct UILabelLayout {
	pub label: UILayerRef,
	pub tile_width: isize,
	pub tile_height: isize
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
