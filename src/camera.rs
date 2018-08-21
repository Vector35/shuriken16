use actor::{ActorRef, BoundingRect};
use render::RenderSize;

pub struct Camera {
	pub map_bounds: BoundingRect,
	pub follow_margin_ratio_x: f32,
	pub follow_margin_ratio_y: f32,
	pub follow_actor: Option<ActorRef>
}

impl Camera {
	pub fn new(map_bounds: BoundingRect, follow_margin_ratio_x: f32, follow_margin_ratio_y: f32,
		follow_actor: Option<ActorRef>) -> Camera {
		Camera { map_bounds, follow_margin_ratio_x, follow_margin_ratio_y, follow_actor }
	}

	fn snap_focus_to_point(&self, x: isize, y: isize, render_size: &RenderSize,
		scroll_x: &mut isize, scroll_y: &mut isize) {
		let follow_margin_x = (render_size.width as f32 * self.follow_margin_ratio_x) as isize;
		let follow_margin_y = (render_size.height as f32 * self.follow_margin_ratio_y) as isize;
		let follow_left = follow_margin_x;
		let follow_right = render_size.width as isize - follow_margin_x;
		let follow_top = follow_margin_y;
		let follow_bottom = render_size.height as isize - follow_margin_y;

		if (x - *scroll_x) < follow_left {
			let new_x = x - follow_left;
			if new_x >= self.map_bounds.x {
				*scroll_x = new_x;
			} else {
				*scroll_x = self.map_bounds.x;
			}
		} else if (x - *scroll_x) > follow_right {
			let new_x = x - follow_right;
			if new_x <= (self.map_bounds.x + self.map_bounds.width - render_size.width as isize) {
				*scroll_x = new_x;
			} else {
				*scroll_x = self.map_bounds.x + self.map_bounds.width - render_size.width as isize;
			}
		}

		if (y - *scroll_y) < follow_top {
			let new_y = y - follow_top;
			if new_y >= self.map_bounds.y {
				*scroll_y = new_y;
			} else {
				*scroll_y = self.map_bounds.y;
			}
		} else if (y - *scroll_y) > follow_bottom {
			let new_y = y - follow_bottom;
			if new_y <= (self.map_bounds.y + self.map_bounds.height - render_size.height as isize) {
				*scroll_y = new_y;
			} else {
				*scroll_y = self.map_bounds.y + self.map_bounds.height - render_size.height as isize;
			}
		}
	}

	pub fn start_following(&mut self, actor: &ActorRef) {
		self.follow_actor = Some(actor.clone());
	}

	pub fn stop_following(&mut self) {
		self.follow_actor = None;
	}

	pub fn tick(&self, render_size: &RenderSize, scroll_x: &mut isize, scroll_y: &mut isize) {
		if let Some(actor) = &self.follow_actor {
			let actor = actor.borrow();
			let actor_info = actor.actor_info();
			let mut x = actor_info.x;
			let mut y = actor_info.y;
			if let Some(bounds) = &actor_info.collision_bounds {
				x += bounds.width / 2;
				y += bounds.height / 2;
			}
			self.snap_focus_to_point(x, y, render_size, scroll_x, scroll_y);
		}
	}
}
