use std::rc::Rc;
use std::cell::{RefCell, Ref, RefMut};
use sprite::{Sprite, SpriteAnimation};

pub struct SpriteWithOffset {
	pub sprite: Rc<Sprite>,
	pub animation: Rc<SpriteAnimation>,
	pub animation_frame: usize,
	pub x_offset: isize,
	pub y_offset: isize
}

pub struct ActorInfo {
	pub x: isize,
	pub y: isize,
	pub subpixel_x: u8,
	pub subpixel_y: u8,
	pub velocity_x: isize,
	pub velocity_y: isize,
	pub sprites: Vec<SpriteWithOffset>
}

#[derive(Clone)]
pub struct ActorRef {
	actor: Rc<RefCell<Box<Actor>>>
}

pub trait Actor {
	fn actor_info(&self) -> &ActorInfo;
	fn actor_info_mut(&mut self) -> &mut ActorInfo;

	fn update(&mut self) {}

	fn apply_move(&mut self) {
		let actor_info = self.actor_info_mut();
		let mut full_x = (actor_info.x << 8) + actor_info.subpixel_x as isize;
		let mut full_y = (actor_info.y << 8) + actor_info.subpixel_y as isize;
		full_x += actor_info.velocity_x;
		full_y += actor_info.velocity_y;
		actor_info.x = full_x >> 8;
		actor_info.y = full_y >> 8;
		actor_info.subpixel_x = (full_x & 0xff) as u8;
		actor_info.subpixel_y = (full_y & 0xff) as u8;
	}

	fn tick(&mut self) {
		self.update();
		self.apply_move();
	}

	fn add_sprite(&mut self, sprite: Rc<Sprite>, x_offset: isize, y_offset: isize) {
		let actor_info = self.actor_info_mut();
		let animation = sprite.get_default_animation();
		actor_info.sprites.push(SpriteWithOffset {
			sprite,
			animation,
			animation_frame: 0,
			x_offset, y_offset
		});
	}

	fn start_animation(&mut self, name: &str) {
		let actor_info = self.actor_info_mut();
		for sprite in &mut actor_info.sprites {
			if let Some(animation) = sprite.sprite.get_animation_by_name(name) {
				if !Rc::ptr_eq(&animation, &sprite.animation) {
					sprite.animation = animation;
					sprite.animation_frame = 0;
				}
			}
		}
	}

	fn on_button_down(&mut self, _name: &str) {}
	fn on_button_up(&mut self, _name: &str) {}
}

impl ActorInfo {
	pub fn new(x: isize, y: isize) -> ActorInfo {
		ActorInfo {
			x, y,
			subpixel_x: 0,
			subpixel_y: 0,
			velocity_x: 0,
			velocity_y: 0,
			sprites: Vec::new()
		}
	}
}

impl ActorRef {
	pub fn new(actor: Box<Actor>) -> ActorRef {
		ActorRef {
			actor: Rc::new(RefCell::new(actor))
		}
	}

	pub fn borrow(&self) -> Ref<Box<Actor>> {
		self.actor.borrow()
	}

	pub fn borrow_mut(&self) -> RefMut<Box<Actor>> {
		self.actor.borrow_mut()
	}
}
