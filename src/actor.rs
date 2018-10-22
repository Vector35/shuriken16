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

#[derive(Debug, Clone)]
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
	pub collision_channel: u32,
	pub blocking_collision: bool,
	pub sprites: Vec<SpriteWithOffset>,
	pub destroyed: bool,
	pub health: i32,
}

pub type ActorRef = Rc<RefCell<Box<Actor>>>;

pub trait ActorAsAny {
	fn as_any(&self) -> &Any;
	fn as_any_mut(&mut self) -> &mut Any;
}

impl BoundingRect {
	pub fn is_colliding(&self, other: &BoundingRect) -> bool {
		(other.x < (self.x + self.width)) && (self.x < (other.x + other.width)) &&
			(other.y < (self.y + self.height)) && (self.y < (other.y + other.height))
	}

	pub fn sweep_collision_x(&self, rect: &BoundingRect, final_x: isize) -> Option<isize> {
		if (self.y >= (rect.y + rect.height)) || (rect.y >= (self.y + self.height)) {
			// Not colliding on y axis
			return None;
		}

		if (self.x < (rect.x + rect.width)) && (rect.x < (self.x + self.width)) {
			// Already colliding at start
			return Some(rect.x);
		}

		if ((rect.x + rect.width) <= self.x) && (final_x > rect.x) {
			if (self.x - rect.width) < final_x {
				// Found earlier collision moving to the right
				return Some(self.x - rect.width);
			}
		} else if (rect.x >= (self.x + self.width)) && (final_x < rect.x) {
			if (self.x + self.width) > final_x {
				// Found earlier collision moving to the left
				return Some(self.x + self.width);
			}
		}

		None
	}

	pub fn sweep_collision_y(&self, rect: &BoundingRect, final_y: isize) -> Option<isize> {
		if (self.x >= (rect.x + rect.width)) || (rect.x >= (self.x + self.width)) {
			// Not colliding on x axis
			return None;
		}

		if (self.y < (rect.y + rect.height)) && (rect.y < (self.y + self.height)) {
			// Already colliding at start
			return Some(rect.y);
		}

		if ((rect.y + rect.height) <= self.y) && (final_y > rect.y) {
			if (self.y - rect.height) < final_y {
				// Found earlier collision moving down
				return Some(self.y - rect.height);
			}
		} else if (rect.y >= (self.y + self.height)) && (final_y < rect.y) {
			if (self.y + self.height) > final_y {
				// Found earlier collision moving up
				return Some(self.y + self.height);
			}
		}

		None
	}
}

pub enum MovementCollision {
	None,
	CollidedWithWorld,
	CollidedWithActor(ActorRef)
}

pub trait Actor: ActorAsAny {
	fn actor_info(&self) -> &ActorInfo;
	fn actor_info_mut(&mut self) -> &mut ActorInfo;

	fn init(&mut self, _game_state: &GameState) {}
	fn update(&mut self, _game_state: &GameState) {}

	fn destroy(&mut self) {
		self.actor_info_mut().destroyed = true;
	}

	fn is_destroyed(&self) -> bool {
		self.actor_info().destroyed
	}

	fn move_with_collision(&mut self, game_state: &GameState) -> MovementCollision {
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
		let mut collision_result = MovementCollision::None;
		if let Some(collision_bounds) = &actor_info.collision_bounds {
			collision_x_offset = collision_bounds.x;
			collision_y_offset = collision_bounds.y;
			collision_width = collision_bounds.width;
			collision_height = collision_bounds.height;

			let mut bounds = BoundingRect {
				x: actor_info.x + collision_x_offset,
				y: actor_info.y + collision_y_offset,
				width: collision_width,
				height: collision_height
			};

			if let Some(map) = &game_state.map {
				if let Some(revised_x) = map.sweep_collision_x(&bounds, new_x + collision_x_offset, actor_info.collision_channel) {
					new_x = revised_x - collision_x_offset;
					full_x = new_x << 8;
					actor_info.velocity_x = 0;
					collision_result = MovementCollision::CollidedWithWorld;
				}

				for actor_ref in &game_state.actors {
					if let Ok(other_actor) = actor_ref.try_borrow() {
						let other_actor_info = other_actor.actor_info();
						if !other_actor_info.blocking_collision {
							continue;
						}
						if let Some(other_bounds) = &other_actor_info.collision_bounds {
							let other_collision_x_offset = other_bounds.x;
							let other_collision_y_offset = other_bounds.y;
							let other_collision_width = other_bounds.width;
							let other_collision_height = other_bounds.height;

							let mut other_bounds = BoundingRect {
								x: other_actor_info.x + other_collision_x_offset,
								y: other_actor_info.y + other_collision_y_offset,
								width: other_collision_width,
								height: other_collision_height
							};

							if let Some(revised_x) = other_bounds.sweep_collision_x(&bounds, new_x + collision_x_offset) {
								new_x = revised_x - collision_x_offset;
								full_x = new_x << 8;
								actor_info.velocity_x = 0;
								collision_result = MovementCollision::CollidedWithActor(actor_ref.clone());
							}
						}
					}
				}

				bounds.x = new_x + collision_x_offset;

				if let Some(revised_y) = map.sweep_collision_y(&bounds, new_y + collision_y_offset, actor_info.collision_channel) {
					new_y = revised_y - collision_y_offset;
					full_y = new_y << 8;
					actor_info.velocity_y = 0;
					collision_result = MovementCollision::CollidedWithWorld;
				}

				for actor_ref in &game_state.actors {
					if let Ok(other_actor) = actor_ref.try_borrow() {
						let other_actor_info = other_actor.actor_info();
						if !other_actor_info.blocking_collision {
							continue;
						}
						if let Some(other_bounds) = &other_actor_info.collision_bounds {
							let other_collision_x_offset = other_bounds.x;
							let other_collision_y_offset = other_bounds.y;
							let other_collision_width = other_bounds.width;
							let other_collision_height = other_bounds.height;

							let mut other_bounds = BoundingRect {
								x: other_actor_info.x + other_collision_x_offset,
								y: other_actor_info.y + other_collision_y_offset,
								width: other_collision_width,
								height: other_collision_height
							};

							if let Some(revised_y) = other_bounds.sweep_collision_y(&bounds, new_y + collision_y_offset) {
								new_y = revised_y - collision_y_offset;
								full_y = new_y << 8;
								actor_info.velocity_y = 0;
								collision_result = MovementCollision::CollidedWithActor(actor_ref.clone());
							}
						}
					}
				}
			}
		}

		actor_info.x = full_x >> 8;
		actor_info.y = full_y >> 8;
		actor_info.subpixel_x = (full_x & 0xff) as u8;
		actor_info.subpixel_y = (full_y & 0xff) as u8;

		collision_result
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

	fn before_move(&mut self, _game_state: &GameState) {}
	fn after_move(&mut self, _game_state: &GameState) {}

	fn apply_move(&mut self, game_state: &GameState) {
		self.before_move(game_state);

		match self.move_with_collision(game_state) {
			MovementCollision::CollidedWithWorld => self.on_collide_with_world(game_state),
			MovementCollision::CollidedWithActor(actor) => self.on_collide_with_actor(&actor, game_state),
			_ => ()
		}

		for actor in self.check_for_actor_collision(game_state) {
			self.on_collide_with_actor(&actor, game_state);
		}

		self.after_move(game_state);
	}

	fn tick(&mut self, game_state: &GameState) {
		self.update(game_state);
		self.apply_move(game_state);
	}

	fn add_sprite(&mut self, sprite: Rc<Sprite>, x_offset: isize, y_offset: isize) -> usize {
		self.actor_info_mut().add_sprite(sprite, x_offset, y_offset)
	}

	fn add_sprite_with_blending(&mut self, sprite: Rc<Sprite>, x_offset: isize, y_offset: isize,
		blend_mode: BlendMode, alpha: u8) -> usize {
		self.actor_info_mut().add_sprite_with_blending(sprite, x_offset, y_offset, blend_mode, alpha)
	}

	fn set_sprite_alpha(&mut self, sprite_index: usize, alpha: u8) {
		self.actor_info_mut().set_sprite_alpha(sprite_index, alpha);
	}

	fn get_sprite_alpha(&mut self, sprite_index: usize) -> u8 {
		self.actor_info_mut().get_sprite_alpha(sprite_index)
	}

	fn adjust_sprite_alpha(&mut self, sprite_index: usize, change: i8) {
		self.actor_info_mut().adjust_sprite_alpha(sprite_index, change);
	}

	fn start_animation(&mut self, name: &str) {
		self.actor_info_mut().start_animation(name);
	}

	fn set_collision_bounds(&mut self, bounds: BoundingRect) {
		self.actor_info_mut().set_collision_bounds(bounds);
	}

	fn clear_collision_bounds(&mut self) {
		self.actor_info_mut().clear_collision_bounds();
	}

	fn get_camera_focus_offset(&self) -> (isize, isize) { (0, 0) }

	fn adjust_health(&mut self, amount: i32, game_state: &GameState) {
		if self.actor_info().health <= 0 {
			return;
		}
		let new_health = self.actor_info().health.saturating_add(amount);
		self.actor_info_mut().health = new_health;
		if new_health <= 0 {
			self.on_death(game_state);
		}
	}

	fn damage(&mut self, _damage_type: &str, _amount: i32, _game_state: &GameState) {}
	fn knockback(&mut self, _x: isize, _y: isize, _game_state: &GameState) {}

	fn on_death(&mut self, _game_state: &GameState) {}

	fn on_button_down(&mut self, _name: &str, _game_state: &GameState) {}
	fn on_button_up(&mut self, _name: &str, _game_state: &GameState) {}
	fn on_axis_changed(&mut self, _name: &str, _value: f32, _game_state: &GameState) {}

	fn on_collide_with_world(&mut self, _game_state: &GameState) {}
	fn on_collide_with_actor(&mut self, _actor: &ActorRef, _game_state: &GameState) {}

	fn on_persistent_actor_removed(&mut self, _game_state: &GameState) {}
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
			collision_channel: 0,
			blocking_collision: false,
			sprites: Vec::new(),
			destroyed: false,
			health: 100
		}
	}

	pub fn add_sprite(&mut self, sprite: Rc<Sprite>, x_offset: isize, y_offset: isize) -> usize {
		let animation = sprite.get_default_animation();
		let index = self.sprites.len();
		self.sprites.push(SpriteWithOffset {
			sprite,
			animation,
			animation_frame: 0,
			x_offset, y_offset,
			blend_mode: BlendMode::Normal,
			alpha: 0
		});
		index
	}

	pub fn add_sprite_with_blending(&mut self, sprite: Rc<Sprite>, x_offset: isize, y_offset: isize,
		blend_mode: BlendMode, alpha: u8) -> usize {
		let animation = sprite.get_default_animation();
		let index = self.sprites.len();
		self.sprites.push(SpriteWithOffset {
			sprite,
			animation,
			animation_frame: 0,
			x_offset, y_offset,
			blend_mode, alpha
		});
		index
	}

	pub fn set_sprite_alpha(&mut self, sprite_index: usize, alpha: u8) {
		if sprite_index < self.sprites.len() {
			self.sprites[sprite_index].alpha = alpha;
		}
	}

	pub fn get_sprite_alpha(&mut self, sprite_index: usize) -> u8 {
		if sprite_index < self.sprites.len() {
			self.sprites[sprite_index].alpha
		} else {
			0
		}
	}

	pub fn adjust_sprite_alpha(&mut self, sprite_index: usize, change: i8) {
		let mut alpha = self.get_sprite_alpha(sprite_index);
		if change > 0 {
			alpha = alpha.saturating_add(change as u8);
			if alpha > 16 {
				alpha = 16;
			}
		} else if change < 0 {
			alpha = alpha.saturating_sub((-change) as u8);
		}
		self.set_sprite_alpha(sprite_index, alpha);
	}

	pub fn start_animation(&mut self, name: &str) {
		for sprite in &mut self.sprites {
			if let Some(animation) = sprite.sprite.get_animation_by_name(name) {
				if !Rc::ptr_eq(&animation, &sprite.animation) {
					sprite.animation = animation;
					sprite.animation_frame = 0;
				}
			}
		}
	}

	pub fn set_collision_bounds(&mut self, bounds: BoundingRect) {
		self.collision_bounds = Some(bounds);
	}

	pub fn clear_collision_bounds(&mut self) {
		self.collision_bounds = None;
	}
}

impl<T: Actor + 'static> ActorAsAny for T {
	fn as_any(&self) -> &Any {
		self
	}

	fn as_any_mut(&mut self) -> &mut Any {
		self
	}
}
