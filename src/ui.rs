extern crate sdl2;

use self::sdl2::mouse::MouseButton;
use self::sdl2::keyboard::{Keycode, Mod};
use std::rc::{Rc, Weak};
use std::cell::RefCell;
use std::any::Any;
use game::GameState;
use map::{MapLayer, TileRef, BlendMode};
use tile::TileSet;
use actor::BoundingRect;
use sprite::{Sprite, SpriteAnimation};

pub struct UISprite {
	pub animation: Rc<SpriteAnimation>,
	pub x: isize,
	pub y: isize,
	pub blend_mode: BlendMode,
	pub alpha: u8
}

pub struct UILayerContents {
	pub layer: MapLayer,
	pub font_tile_set: Option<Rc<TileSet>>,
	pub font_base: u8,
	pub sprites: Vec<UISprite>
}

pub struct UILayer {
	pub contents: UILayerContents,
	pub renderer: Option<Box<UILayerRenderer>>,
	pub bounds: BoundingRect,
	pub input_handler: Option<Box<UIInputHandler>>
}

pub enum HorizontalAnchor {
	Left,
	Center,
	Right
}

pub enum VerticalAnchor {
	Top,
	Center,
	Bottom
}

pub struct SingleLayerLayout {
	pub layer: UILayerRef,
	pub width: Option<isize>,
	pub height: Option<isize>
}

pub struct EmptyLayout {
	pub width: Option<isize>,
	pub height: Option<isize>
}

pub struct FixedSizeLayout {
	pub inner: UILayoutRef,
	pub width: isize,
	pub height: isize
}

pub struct FixedPositionLayout {
	pub inner: UILayoutRef,
	pub x: isize,
	pub y: isize
}

pub struct AnchorLayout {
	pub inner: UILayoutRef,
	pub horizontal_anchor: HorizontalAnchor,
	pub vertical_anchor: VerticalAnchor
}

pub struct BorderLayout {
	pub inner: UILayoutRef,
	pub left: isize,
	pub right: isize,
	pub top: isize,
	pub bottom: isize
}

pub enum BackgroundLayoutAlignment {
	AlignToBackground,
	AlignToForeground
}

pub struct BackgroundLayout {
	pub background: UILayoutRef,
	pub foreground: UILayoutRef,
	pub alignment: BackgroundLayoutAlignment
}

pub struct HorizontalBoxLayout {
	pub layouts: Vec<UILayoutRef>
}

pub struct VerticalBoxLayout {
	pub layouts: Vec<UILayoutRef>
}

pub trait UIVisibilityHandler {
	fn is_visible(&self) -> bool;
}

pub struct VisibilityLayout {
	pub inner: UILayoutRef,
	pub handler: Box<UIVisibilityHandler>
}

pub fn to_layer_ref(layer: UILayer) -> UILayerRef {
	Rc::new(RefCell::new(layer))
}

pub trait UILayoutAsAny {
	fn as_any(&self) -> &Any;
}

pub trait UILayout: UILayoutAsAny {
	fn width(&self) -> Option<isize>;
	fn height(&self) -> Option<isize>;
	fn tile_width(&self) -> isize;
	fn tile_height(&self) -> isize;
	fn update(&self, bounds: &BoundingRect) -> BoundingRect;
	fn layers(&self) -> Vec<UILayerRef>;
}

impl<T: UILayout + 'static> UILayoutAsAny for T {
	fn as_any(&self) -> &Any {
		self
	}
}

pub fn to_layout_ref(layout: Box<UILayout>) -> UILayoutRef {
	Rc::new(RefCell::new(layout))
}

pub type UILayerRef = Rc<RefCell<UILayer>>;
pub type UILayoutRef = Rc<RefCell<Box<UILayout>>>;
pub type UILayoutWeakRef = Weak<RefCell<Box<UILayout>>>;

pub trait UILayerRendererAsAny {
	fn as_any(&self) -> &Any;
}

pub trait UILayerRenderer: UILayerRendererAsAny {
	fn update(&mut self, layer: &mut UILayerContents, game_state: &GameState);
}

impl<T: UILayerRenderer + 'static> UILayerRendererAsAny for T {
	fn as_any(&self) -> &Any {
		self
	}
}

pub trait UIInputHandler {
	fn on_button_down(&self, _name: &str, _game_state: &GameState) {}
	fn on_button_up(&self, _name: &str, _game_state: &GameState) {}
	fn on_axis_changed(&self, _name: &str, _value: f32, _game_state: &GameState) {}

	fn on_mouse_button_down(&self, _x: isize, _y: isize, _button: MouseButton, _game_state: &GameState) {}
	fn on_mouse_button_up(&self, _x: isize, _y: isize, _button: MouseButton, _game_state: &GameState) {}
	fn on_mouse_move(&self, _x: isize, _y: isize, _game_state: &GameState) {}
	fn on_mouse_wheel(&self, _x: isize, _y: isize, _game_state: &GameState) {}
	fn on_double_click(&self, _x: isize, _y: isize, _button: MouseButton, _game_state: &GameState) {}

	fn raw_keyboard_input(&self) -> bool { false }
	fn on_key_down(&self, _key: Keycode, _key_mod: Mod, _game_state: &GameState) {}
	fn on_key_up(&self, _key: Keycode, _key_mod: Mod, _game_state: &GameState) {}
	fn on_text_input(&self, _text: &str, _game_state: &GameState) {}
}

impl UILayerContents {
	pub fn width(&self) -> isize {
		self.layer.width as isize
	}

	pub fn height(&self) -> isize {
		self.layer.height as isize
	}

	pub fn clear(&mut self) {
		for y in 0..self.height() {
			for x in 0..self.width() {
				self.layer.set_tile(x as usize, y as usize, None);
			}
		}
		self.sprites.clear();
	}

	pub fn write(&mut self, x: isize, y: isize, text: &str) {
		let font_tile_set = match &self.font_tile_set {
			Some(tile_set) => tile_set.clone(),
			None => return
		};

		if (y < 0) || (y >= self.height()) {
			return;
		}

		let mut cur_x = x;
		let mut offset_x = 0;
		for ch in text.chars() {
			if ch == '\t' {
				let tab_width = 4 - (offset_x % 4);
				cur_x += tab_width;
				offset_x += tab_width;
				continue;
			}

			let ord = ch as u32;
			if ord < self.font_base as u32 {
				continue;
			}
			let font_index = ord - self.font_base as u32;
			if font_index as usize >= font_tile_set.tiles.len() {
				continue;
			}

			if (cur_x >= 0) && (cur_x < self.width()) {
				self.layer.set_tile(cur_x as usize, y as usize, Some(TileRef {
					tile_set: Rc::clone(&font_tile_set),
					tile_index: font_index as usize
				}));
			}
			cur_x += 1;
			offset_x += 1;
		}
	}

	pub fn set_tile(&mut self, x: isize, y: isize, tile: TileRef) {
		if (x >= 0) && (x < self.width()) && (y >= 0) && (y < self.height()) {
			self.layer.set_tile(x as usize, y as usize, Some(tile));
		}
	}

	pub fn add_sprite(&mut self, x: isize, y: isize, sprite: Rc<Sprite>) {
		self.sprites.push(UISprite {
			x, y,
			animation: sprite.get_default_animation(),
			blend_mode: BlendMode::Normal,
			alpha: 0
		});
	}

	pub fn add_sprite_with_blending(&mut self, x: isize, y: isize, sprite: Rc<Sprite>,
		blend_mode: BlendMode, alpha: u8) {
		self.sprites.push(UISprite {
			x, y,
			animation: sprite.get_default_animation(),
			blend_mode, alpha
		});
	}

	pub fn add_sprite_animation(&mut self, x: isize, y: isize, animation: Rc<SpriteAnimation>) {
		self.sprites.push(UISprite {
			x, y,
			animation,
			blend_mode: BlendMode::Normal,
			alpha: 0
		});
	}

	pub fn add_sprite_animation_with_blending(&mut self, x: isize, y: isize, animation: Rc<SpriteAnimation>,
		blend_mode: BlendMode, alpha: u8) {
		self.sprites.push(UISprite {
			x, y, animation, blend_mode, alpha
		});
	}
}

impl UILayer {
	pub fn new(tile_width: usize, tile_height: usize, tile_depth: usize) -> UILayer {
		UILayer {
			contents: UILayerContents {
				layer: MapLayer::new("Text", 0, 0, tile_width, tile_height, tile_depth),
				font_tile_set: None,
				font_base: 0,
				sprites: Vec::new()
			},
			renderer: None,
			bounds: BoundingRect { x: 0, y: 0, width: 0, height: 0 },
			input_handler: None
		}
	}

	pub fn set_font(&mut self, font_tile_set: Rc<TileSet>, font_base: u8) {
		self.contents.font_tile_set = Some(font_tile_set);
		self.contents.font_base = font_base;
	}

	pub fn width(&self) -> isize {
		self.contents.width()
	}

	pub fn height(&self) -> isize {
		self.contents.height()
	}

	pub fn clear(&mut self) {
		self.contents.clear();
	}

	pub fn write(&mut self, x: isize, y: isize, text: &str) {
		self.contents.write(x, y, text);
	}

	pub fn set_tile(&mut self, x: isize, y: isize, tile: TileRef) {
		self.contents.set_tile(x, y, tile);
	}

	pub fn update(&mut self, game_state: &GameState) {
		if let Some(renderer) = &mut self.renderer {
			renderer.update(&mut self.contents, game_state);
		}
	}

	pub fn get_map_layer(&self) -> &MapLayer {
		&self.contents.layer
	}

	pub fn set_target_rect(&mut self, rect: &BoundingRect) {
		let desired_width = rect.width as usize / self.contents.layer.tile_width;
		let desired_height = rect.height as usize / self.contents.layer.tile_height;
		if (desired_width != self.contents.layer.width) || (desired_height != self.contents.layer.height) {
			self.contents.layer.resize(desired_width, desired_height);
		}
		self.bounds = BoundingRect {
			x: rect.x + (rect.width - (desired_width * self.contents.layer.tile_width) as isize) / 2,
			y: rect.y + (rect.height - (desired_height * self.contents.layer.tile_height) as isize) / 2,
			width: (desired_width * self.contents.layer.tile_width) as isize,
			height: (desired_height * self.contents.layer.tile_height) as isize
		}
	}

	pub fn get_window_rect(&self) -> BoundingRect { self.bounds.clone() }
}

impl UILayout for SingleLayerLayout {
	fn width(&self) -> Option<isize> { self.width }
	fn height(&self) -> Option<isize> { self.height }
	fn tile_width(&self) -> isize { self.layer.borrow().contents.layer.tile_width as isize }
	fn tile_height(&self) -> isize { self.layer.borrow().contents.layer.tile_height as isize }

	fn update(&self, bounds: &BoundingRect) -> BoundingRect {
		let rect = BoundingRect {
			x: bounds.x,
			y: bounds.y,
			width: match self.width() {
				Some(width) => width,
				None => bounds.width
			},
			height: match self.height() {
				Some(height) => height,
				None => bounds.height
			}
		};
		self.layer.borrow_mut().set_target_rect(&rect);
		rect
	}

	fn layers(&self) -> Vec<UILayerRef> { [self.layer.clone()].to_vec() }
}

impl SingleLayerLayout {
	pub fn with_fixed_size(layer: UILayerRef, width: isize, height: isize) -> SingleLayerLayout {
		SingleLayerLayout {
			layer,
			width: Some(width),
			height: Some(height)
		}
	}

	pub fn with_fixed_width(layer: UILayerRef, width: isize) -> SingleLayerLayout {
		SingleLayerLayout {
			layer,
			width: Some(width),
			height: None
		}
	}

	pub fn with_fixed_height(layer: UILayerRef, height: isize) -> SingleLayerLayout {
		SingleLayerLayout {
			layer,
			width: None,
			height: Some(height)
		}
	}

	pub fn with_fill(layer: UILayerRef) -> SingleLayerLayout {
		SingleLayerLayout {
			layer,
			width: None,
			height: None
		}
	}
}

impl UILayout for EmptyLayout {
	fn width(&self) -> Option<isize> { self.width }
	fn height(&self) -> Option<isize> { self.height }
	fn tile_width(&self) -> isize { 1 }
	fn tile_height(&self) -> isize { 1 }

	fn update(&self, bounds: &BoundingRect) -> BoundingRect {
		let rect = BoundingRect {
			x: bounds.x,
			y: bounds.y,
			width: match self.width() {
				Some(width) => width,
				None => bounds.width
			},
			height: match self.height() {
				Some(height) => height,
				None => bounds.height
			}
		};
		rect
	}

	fn layers(&self) -> Vec<UILayerRef> { [].to_vec() }
}

impl EmptyLayout {
	pub fn with_fixed_size(width: isize, height: isize) -> EmptyLayout {
		EmptyLayout {
			width: Some(width),
			height: Some(height)
		}
	}

	pub fn with_fixed_width(width: isize) -> EmptyLayout {
		EmptyLayout {
			width: Some(width),
			height: None
		}
	}

	pub fn with_fixed_height(height: isize) -> EmptyLayout {
		EmptyLayout {
			width: None,
			height: Some(height)
		}
	}

	pub fn with_fill() -> EmptyLayout {
		EmptyLayout {
			width: None,
			height: None
		}
	}
}

impl UILayout for FixedSizeLayout {
	fn width(&self) -> Option<isize> { Some(self.width) }
	fn height(&self) -> Option<isize> { Some(self.height) }
	fn tile_width(&self) -> isize { self.inner.borrow().tile_width() }
	fn tile_height(&self) -> isize { self.inner.borrow().tile_height() }

	fn update(&self, bounds: &BoundingRect) -> BoundingRect {
		let rect = BoundingRect {
			x: bounds.x,
			y: bounds.y,
			width: self.width,
			height: self.height
		};
		self.inner.borrow().update(&rect);
		rect
	}

	fn layers(&self) -> Vec<UILayerRef> { self.inner.borrow().layers() }
}

impl FixedSizeLayout {
	pub fn new(inner: UILayoutRef, width: isize, height: isize) -> FixedSizeLayout {
		FixedSizeLayout {
			inner, width, height
		}
	}
}

impl UILayout for FixedPositionLayout {
	fn width(&self) -> Option<isize> { self.inner.borrow().width() }
	fn height(&self) -> Option<isize> { self.inner.borrow().height() }
	fn tile_width(&self) -> isize { self.inner.borrow().tile_width() }
	fn tile_height(&self) -> isize { self.inner.borrow().tile_height() }

	fn update(&self, bounds: &BoundingRect) -> BoundingRect {
		let rect = BoundingRect {
			x: self.x,
			y: self.y,
			width: match self.width() {
				Some(width) => width,
				None => bounds.width
			},
			height: match self.height() {
				Some(height) => height,
				None => bounds.height
			}
		};
		self.inner.borrow().update(&rect);
		rect
	}

	fn layers(&self) -> Vec<UILayerRef> { self.inner.borrow().layers() }
}

impl FixedPositionLayout {
	pub fn new(inner: UILayoutRef, x: isize, y: isize) -> FixedPositionLayout {
		FixedPositionLayout {
			inner, x, y
		}
	}
}

impl UILayout for AnchorLayout {
	fn width(&self) -> Option<isize> { self.inner.borrow().width() }
	fn height(&self) -> Option<isize> { self.inner.borrow().height() }
	fn tile_width(&self) -> isize { self.inner.borrow().tile_width() }
	fn tile_height(&self) -> isize { self.inner.borrow().tile_height() }

	fn update(&self, bounds: &BoundingRect) -> BoundingRect {
		let inner_width = self.width();
		let inner_height = self.height();

		let x;
		let width;
		if let Some(w) = inner_width {
			width = w;
			x = match self.horizontal_anchor {
				HorizontalAnchor::Left => bounds.x,
				HorizontalAnchor::Center => bounds.x + (bounds.width - width) / 2,
				HorizontalAnchor::Right => bounds.x + bounds.width - width
			};
		} else {
			width = bounds.width;
			x = 0;
		}

		let y;
		let height;
		if let Some(h) = inner_height {
			height = h;
			y = match self.vertical_anchor {
				VerticalAnchor::Top => bounds.y,
				VerticalAnchor::Center => bounds.y + (bounds.height - height) / 2,
				VerticalAnchor::Bottom => bounds.y + bounds.height - height
			};
		} else {
			height = bounds.height;
			y = 0;
		}

		let rect = BoundingRect { x, y, width, height };
		self.inner.borrow().update(&rect);
		rect
	}

	fn layers(&self) -> Vec<UILayerRef> { self.inner.borrow().layers() }
}

impl AnchorLayout {
	pub fn new(inner: UILayoutRef, horizontal_anchor: HorizontalAnchor, vertical_anchor: VerticalAnchor) -> AnchorLayout {
		AnchorLayout {
			inner, horizontal_anchor, vertical_anchor
		}
	}
}

impl UILayout for BorderLayout {
	fn width(&self) -> Option<isize> {
		if let Some(width) = self.inner.borrow().width() {
			Some(width + self.left + self.right)
		} else {
			None
		}
	}

	fn height(&self) -> Option<isize> {
		if let Some(height) = self.inner.borrow().height() {
			Some(height + self.top + self.bottom)
		} else {
			None
		}
	}

	fn tile_width(&self) -> isize { self.inner.borrow().tile_width() }
	fn tile_height(&self) -> isize { self.inner.borrow().tile_height() }

	fn update(&self, bounds: &BoundingRect) -> BoundingRect {
		let inner_rect = BoundingRect {
			x: bounds.x + self.left,
			y: bounds.y + self.top,
			width: bounds.width - (self.left + self.right),
			height: bounds.height - (self.top + self.bottom)
		};
		self.inner.borrow().update(&inner_rect);
		bounds.clone()
	}

	fn layers(&self) -> Vec<UILayerRef> { self.inner.borrow().layers() }
}

impl BorderLayout {
	pub fn new(inner: UILayoutRef, left: isize, right: isize, top: isize, bottom: isize) -> BorderLayout {
		BorderLayout {
			inner, left, right, top, bottom
		}
	}
}

impl UILayout for BackgroundLayout {
	fn width(&self) -> Option<isize> { self.foreground.borrow().width() }
	fn height(&self) -> Option<isize> { self.foreground.borrow().height() }

	fn tile_width(&self) -> isize {
		match self.alignment {
			BackgroundLayoutAlignment::AlignToBackground => self.background.borrow().tile_width(),
			BackgroundLayoutAlignment::AlignToForeground => self.foreground.borrow().tile_width()
		}
	}

	fn tile_height(&self) -> isize {
		match self.alignment {
			BackgroundLayoutAlignment::AlignToBackground => self.background.borrow().tile_height(),
			BackgroundLayoutAlignment::AlignToForeground => self.foreground.borrow().tile_height()
		}
	}

	fn update(&self, bounds: &BoundingRect) -> BoundingRect {
		let background = self.background.borrow();
		let foreground = self.foreground.borrow();
		match self.alignment {
			BackgroundLayoutAlignment::AlignToBackground => {
				background.update(bounds);
				let tile_width = background.tile_width();
				let tile_height = background.tile_height();
				let desired_width = bounds.width / tile_width;
				let desired_height = bounds.height / tile_height;
				let aligned_bounds = BoundingRect {
					x: bounds.x + (bounds.width - (desired_width * tile_width)) / 2,
					y: bounds.y + (bounds.height - (desired_height * tile_height)) / 2,
					width: (desired_width * tile_width),
					height: (desired_height * tile_height)
				};
				foreground.update(&aligned_bounds);
			},
			BackgroundLayoutAlignment::AlignToForeground => {
				foreground.update(bounds);
				let tile_width = foreground.tile_width();
				let tile_height = foreground.tile_height();
				let desired_width = bounds.width / tile_width;
				let desired_height = bounds.height / tile_height;
				let aligned_bounds = BoundingRect {
					x: bounds.x + (bounds.width - (desired_width * tile_width)) / 2,
					y: bounds.y + (bounds.height - (desired_height * tile_height)) / 2,
					width: (desired_width * tile_width),
					height: (desired_height * tile_height)
				};
				background.update(&aligned_bounds);
			}
		};
		bounds.clone()
	}

	fn layers(&self) -> Vec<UILayerRef> {
		let mut background_layers = self.background.borrow().layers();
		let mut foreground_layers = self.foreground.borrow().layers();
		background_layers.append(&mut foreground_layers);
		background_layers
	}
}

impl BackgroundLayout {
	pub fn new(background: UILayoutRef, foreground: UILayoutRef,
		alignment: BackgroundLayoutAlignment) -> BackgroundLayout {
		BackgroundLayout {
			background, foreground, alignment
		}
	}
}

impl UILayout for HorizontalBoxLayout {
	fn width(&self) -> Option<isize> {
		let mut width = 0;
		for layout in &self.layouts {
			match layout.borrow().width() {
				Some(layout_width) => width += layout_width,
				None => return None
			}
		}
		Some(width)
	}

	fn height(&self) -> Option<isize> {
		let mut height = 0;
		for layout in &self.layouts {
			match layout.borrow().height() {
				Some(layout_height) => {
					if layout_height > height {
						height = layout_height;
					}
				},
				None => return None
			}
		}
		Some(height)
	}

	fn tile_width(&self) -> isize {
		if self.layouts.len() > 0 {
			self.layouts[0].borrow().tile_width()
		} else {
			1
		}
	}

	fn tile_height(&self) -> isize {
		if self.layouts.len() > 0 {
			self.layouts[0].borrow().tile_height()
		} else {
			1
		}
	}

	fn update(&self, bounds: &BoundingRect) -> BoundingRect {
		let mut fixed_width = 0;
		let mut fill_count = 0;
		for layout in &self.layouts {
			match layout.borrow().width() {
				Some(layout_width) => fixed_width += layout_width,
				None => fill_count += 1
			}
		}

		let mut fill_width = 0;
		if fill_count > 0 {
			fill_width = (bounds.width - fixed_width) / fill_count;
			if fill_width < 0 {
				fill_width = 0;
			}
		}

		let mut x = 0;
		for layout in &self.layouts {
			let width = match layout.borrow().width() {
				Some(layout_width) => layout_width,
				None => fill_width
			};
			layout.borrow().update(&BoundingRect {
				x: bounds.x + x,
				y: bounds.y,
				width: width,
				height: bounds.height
			});
			x += width;
		}
		bounds.clone()
	}

	fn layers(&self) -> Vec<UILayerRef> {
		let mut result = Vec::new();
		for layout in &self.layouts {
			result.append(&mut layout.borrow().layers());
		}
		result
	}
}

impl HorizontalBoxLayout {
	pub fn new(layouts: Vec<UILayoutRef>) -> HorizontalBoxLayout {
		HorizontalBoxLayout {
			layouts
		}
	}
}

impl UILayout for VerticalBoxLayout {
	fn width(&self) -> Option<isize> {
		let mut width = 0;
		for layout in &self.layouts {
			match layout.borrow().width() {
				Some(layout_width) => {
					if layout_width > width {
						width = layout_width;
					}
				},
				None => return None
			}
		}
		Some(width)
	}

	fn height(&self) -> Option<isize> {
		let mut height = 0;
		for layout in &self.layouts {
			match layout.borrow().height() {
				Some(layout_height) => height += layout_height,
				None => return None
			}
		}
		Some(height)
	}

	fn tile_width(&self) -> isize {
		if self.layouts.len() > 0 {
			self.layouts[0].borrow().tile_width()
		} else {
			1
		}
	}

	fn tile_height(&self) -> isize {
		if self.layouts.len() > 0 {
			self.layouts[0].borrow().tile_height()
		} else {
			1
		}
	}

	fn update(&self, bounds: &BoundingRect) -> BoundingRect {
		let mut fixed_height = 0;
		let mut fill_count = 0;
		for layout in &self.layouts {
			match layout.borrow().height() {
				Some(layout_height) => fixed_height += layout_height,
				None => fill_count += 1
			}
		}

		let mut fill_height = 0;
		if fill_count > 0 {
			fill_height = (bounds.height - fixed_height) / fill_count;
			if fill_height < 0 {
				fill_height = 0;
			}
		}

		let mut y = 0;
		for layout in &self.layouts {
			let height = match layout.borrow().height() {
				Some(layout_height) => layout_height,
				None => fill_height
			};
			layout.borrow().update(&BoundingRect {
				x: bounds.x,
				y: bounds.y + y,
				width: bounds.width,
				height: height
			});
			y += height;
		}
		bounds.clone()
	}

	fn layers(&self) -> Vec<UILayerRef> {
		let mut result = Vec::new();
		for layout in &self.layouts {
			result.append(&mut layout.borrow().layers());
		}
		result
	}
}

impl VerticalBoxLayout {
	pub fn new(layouts: Vec<UILayoutRef>) -> VerticalBoxLayout {
		VerticalBoxLayout {
			layouts
		}
	}
}

impl UILayout for VisibilityLayout {
	fn width(&self) -> Option<isize> {
		if self.handler.is_visible() {
			self.inner.borrow().width()
		} else {
			Some(0)
		}
	}

	fn height(&self) -> Option<isize> {
		if self.handler.is_visible() {
			self.inner.borrow().height()
		} else {
			Some(0)
		}
	}

	fn tile_width(&self) -> isize { self.inner.borrow().tile_width() }
	fn tile_height(&self) -> isize { self.inner.borrow().tile_height() }

	fn update(&self, bounds: &BoundingRect) -> BoundingRect {
		self.inner.borrow().update(bounds);
		bounds.clone()
	}

	fn layers(&self) -> Vec<UILayerRef> {
		if self.handler.is_visible() {
			self.inner.borrow().layers()
		} else {
			Vec::new()
		}
	}
}

impl VisibilityLayout {
	pub fn new(inner: UILayoutRef, handler: Box<UIVisibilityHandler>) -> VisibilityLayout {
		VisibilityLayout {
			inner, handler
		}
	}
}
