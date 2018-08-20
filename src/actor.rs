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
	pub sprites: Vec<SpriteWithOffset>
}

#[derive(Clone)]
pub struct ActorRef {
	actor: Rc<RefCell<Box<Actor>>>
}

pub trait Actor {
	fn actor_info(&self) -> &ActorInfo;
	fn actor_info_mut(&mut self) -> &mut ActorInfo;

	fn tick(&mut self) {}

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
			match sprite.sprite.get_animation_by_name(name) {
				Some(animation) => {
					if !Rc::ptr_eq(&animation, &sprite.animation) {
						sprite.animation = animation;
						sprite.animation_frame = 0;
					}
				},
				None => ()
			};
		}
	}
}

impl ActorInfo {
	pub fn new(x: isize, y: isize) -> ActorInfo {
		ActorInfo {
			x, y,
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
