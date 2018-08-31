use std::rc::Rc;
use std::cell::RefCell;
use std::any::Any;
use sprite::{Sprite, SpriteAnimation};
use game::GameState;
use map::BlendMode;

pub struct SpriteWithOffset {
	pub sprite: Rc<Sprite>,
	pub animation: Rc<SpriteAnimation>,
	pub animation_frame: usize,
	pub x_offset: isize,
	pub y_offset: isize,
	pub blend_mode: BlendMode,
	pub alpha: u8
}

#[derive(Debug)]
pub struct BoundingRect {
	pub x: isize,
	pub y: isize,
	pub width: isize,
	pub height: isize
}

pub struct ActorInfo {
	pub x: isize,
	pub y: isize,
	pub subpixel_x: u8,
	pub subpixel_y: u8,
	pub velocity_x: isize,
	pub velocity_y: isize,
	pub collision_bounds: Option<BoundingRect>,
	pub sprites: Vec<SpriteWithOffset>
}

pub type ActorRef = Rc<RefCell<Box<Actor>>>;

pub trait AsAny {
	fn as_any(&self) -> &Any;
}

impl BoundingRect {
	pub fn is_colliding(&self, other: &BoundingRect) -> bool {
		(other.x < (self.x + self.width)) && (self.x < (other.x + other.width)) &&
			(other.y < (self.y + self.height)) && (self.y < (other.y + other.height))
	}
}

pub trait Actor: AsAny {
	fn actor_info(&self) -> &ActorInfo;
	fn actor_info_mut(&mut self) -> &mut ActorInfo;

	fn update(&mut self, _game_state: &GameState) {}

	fn move_with_collision(&mut self, game_state: &GameState) -> bool {
		let actor_info = self.actor_info_mut();
		let mut full_x = (actor_info.x << 8) + actor_info.subpixel_x as isize;
		let mut full_y = (actor_info.y << 8) + actor_info.subpixel_y as isize;
		full_x += actor_info.velocity_x;
		full_y += actor_info.velocity_y;

		let mut new_x = full_x >> 8;
		let mut new_y = full_y >> 8;

		let collision_x_offset;
		let collision_y_offset;
		let collision_width;
		let collision_height;
		if let Some(collision_bounds) = &actor_info.collision_bounds {
			collision_x_offset = collision_bounds.x;
			collision_y_offset = collision_bounds.y;
			collision_width = collision_bounds.width;
			collision_height = collision_bounds.height;
		} else {
			collision_x_offset = 0;
			collision_y_offset = 0;
			collision_width = 1;
			collision_height = 1;
		}

		let mut bounds = BoundingRect {
			x: actor_info.x + collision_x_offset,
			y: actor_info.y + collision_y_offset,
			width: collision_width,
			height: collision_height
		};

		let mut collided_with_world = false;
		if let Some(map) = &game_state.map {
			if let Some(revised_x) = map.sweep_collision_x(&bounds, new_x + collision_x_offset) {
				new_x = revised_x - collision_x_offset;
				full_x = new_x << 8;
				actor_info.velocity_x = 0;
				collided_with_world = true;
			}

			bounds.x = new_x + collision_x_offset;

			if let Some(revised_y) = map.sweep_collision_y(&bounds, new_y + collision_y_offset) {
				new_y = revised_y - collision_y_offset;
				full_y = new_y << 8;
				actor_info.velocity_y = 0;
				collided_with_world = true;
			}
		}

		actor_info.x = full_x >> 8;
		actor_info.y = full_y >> 8;
		actor_info.subpixel_x = (full_x & 0xff) as u8;
		actor_info.subpixel_y = (full_y & 0xff) as u8;

		collided_with_world
	}

	fn check_for_actor_collision(&mut self, game_state: &GameState) -> Vec<ActorRef> {
		let actor_info = self.actor_info();
		let mut collided_actors: Vec<ActorRef> = Vec::new();

		if let Some(collision_bounds) = &actor_info.collision_bounds {
			let collision_x_offset = collision_bounds.x;
			let collision_y_offset = collision_bounds.y;
			let collision_width = collision_bounds.width;
			let collision_height = collision_bounds.height;

			let mut bounds = BoundingRect {
				x: actor_info.x + collision_x_offset,
				y: actor_info.y + collision_y_offset,
				width: collision_width,
				height: collision_height
			};

			for actor_ref in &game_state.actors {
				if let Ok(other_actor) = actor_ref.try_borrow() {
					let other_actor_info = other_actor.actor_info();
					if let Some(other_bounds) = &other_actor_info.collision_bounds {
						let collision_x_offset = other_bounds.x;
						let collision_y_offset = other_bounds.y;
						let collision_width = other_bounds.width;
						let collision_height = other_bounds.height;

						let mut other_bounds = BoundingRect {
							x: other_actor_info.x + collision_x_offset,
							y: other_actor_info.y + collision_y_offset,
							width: collision_width,
							height: collision_height
						};

						if bounds.is_colliding(&other_bounds) {
							collided_actors.push(actor_ref.clone());
						}
					}
				}
			}
		}

		collided_actors
	}

	fn apply_move(&mut self, game_state: &GameState) {
		if self.move_with_collision(game_state) {
			self.on_collide_with_world(game_state);
		}

		for actor in self.check_for_actor_collision(game_state) {
			self.on_collide_with_actor(&actor, game_state);
		}
	}

	fn tick(&mut self, game_state: &GameState) {
		self.update(game_state);
		self.apply_move(game_state);
	}

	fn add_sprite(&mut self, sprite: Rc<Sprite>, x_offset: isize, y_offset: isize) {
		let actor_info = self.actor_info_mut();
		let animation = sprite.get_default_animation();
		actor_info.sprites.push(SpriteWithOffset {
			sprite,
			animation,
			animation_frame: 0,
			x_offset, y_offset,
			blend_mode: BlendMode::Normal,
			alpha: 0
		});
	}

	fn add_sprite_with_blending(&mut self, sprite: Rc<Sprite>, x_offset: isize, y_offset: isize,
		blend_mode: BlendMode, alpha: u8) {
		let actor_info = self.actor_info_mut();
		let animation = sprite.get_default_animation();
		actor_info.sprites.push(SpriteWithOffset {
			sprite,
			animation,
			animation_frame: 0,
			x_offset, y_offset,
			blend_mode, alpha
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

	fn set_collision_bounds(&mut self, bounds: BoundingRect) {
		let actor_info = self.actor_info_mut();
		actor_info.collision_bounds = Some(bounds);
	}

	fn on_button_down(&mut self, _name: &str) {}
	fn on_button_up(&mut self, _name: &str) {}
	fn on_axis_changed(&mut self, _name: &str, _value: f32) {}

	fn on_collide_with_world(&mut self, _game_state: &GameState) {}
	fn on_collide_with_actor(&mut self, _actor: &ActorRef, _game_state: &GameState) {}
}

impl ActorInfo {
	pub fn new(x: isize, y: isize) -> ActorInfo {
		ActorInfo {
			x, y,
			subpixel_x: 0,
			subpixel_y: 0,
			velocity_x: 0,
			velocity_y: 0,
			collision_bounds: None,
			sprites: Vec::new()
		}
	}
}

impl<T: Actor + 'static> AsAny for T {
	fn as_any(&self) -> &Any {
		self
	}
}
