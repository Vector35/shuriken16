use std::f32::consts::PI;
use actor::{ActorRef, BoundingRect};
use render::RenderSize;

pub struct Camera {
	pub map_bounds: BoundingRect,
	pub follow_margin_ratio_x: f32,
	pub follow_margin_ratio_y: f32,
	pub follow_actor: Option<ActorRef>,
	pub maximum_snap_dist: usize,
	pub pan_time: usize,
	pub force_snap: bool,
	panning: bool,
	pan_frame: usize,
	pan_start_x: isize,
	pan_start_y: isize,
	pan_target_x: isize,
	pan_target_y: isize
}

impl Camera {
	pub fn new(map_bounds: BoundingRect, follow_margin_ratio_x: f32, follow_margin_ratio_y: f32,
		follow_actor: Option<ActorRef>) -> Camera {
		Camera {
			map_bounds, follow_margin_ratio_x, follow_margin_ratio_y, follow_actor,
			maximum_snap_dist: 2,
			pan_time: 30,
			force_snap: true,
			panning: false,
			pan_frame: 0,
			pan_start_x: 0,
			pan_start_y: 0,
			pan_target_x: 0,
			pan_target_y: 0
		}
	}

	fn move_focus_to_point(&mut self, x: isize, y: isize, render_size: &RenderSize,
		scroll_x: &mut isize, scroll_y: &mut isize) {
		let follow_margin_x = (render_size.width as f32 * self.follow_margin_ratio_x) as isize;
		let follow_margin_y = (render_size.height as f32 * self.follow_margin_ratio_y) as isize;
		let follow_left = follow_margin_x;
		let follow_right = render_size.width as isize - follow_margin_x;
		let follow_top = follow_margin_y;
		let follow_bottom = render_size.height as isize - follow_margin_y;

		let cur_x = *scroll_x;
		let cur_y = *scroll_y;
		let mut target_scroll_x = cur_x;
		let mut target_scroll_y = cur_y;

		if (x - *scroll_x) < follow_left {
			let new_x = x - follow_left;
			target_scroll_x = new_x;
		} else if (x - *scroll_x) > follow_right {
			let new_x = x - follow_right;
			target_scroll_x = new_x;
		}

		if (y - *scroll_y) < follow_top {
			let new_y = y - follow_top;
			target_scroll_y = new_y;
		} else if (y - *scroll_y) > follow_bottom {
			let new_y = y - follow_bottom;
			target_scroll_y = new_y;
		}

		if target_scroll_x < self.map_bounds.x {
			target_scroll_x = self.map_bounds.x;
		} else if target_scroll_x > (self.map_bounds.x + self.map_bounds.width - render_size.width as isize) {
			target_scroll_x = self.map_bounds.x + self.map_bounds.width - render_size.width as isize;
		}
		if target_scroll_y < self.map_bounds.y {
			target_scroll_y = self.map_bounds.y;
		} else if target_scroll_y > (self.map_bounds.y + self.map_bounds.height - render_size.height as isize) {
			target_scroll_y = self.map_bounds.y + self.map_bounds.height - render_size.height as isize;
		}

		if self.force_snap {
			self.force_snap = false;
			self.panning = false;
			*scroll_x = target_scroll_x;
			*scroll_y = target_scroll_y;
			return;
		}

		if !self.panning {
			let dist = (target_scroll_x - cur_x).abs() + (target_scroll_y - cur_y).abs();
			if dist as usize > self.maximum_snap_dist {
				self.panning = true;
				self.pan_frame = 0;
				self.pan_start_x = cur_x;
				self.pan_start_y = cur_y;
				self.pan_target_x = target_scroll_x;
				self.pan_target_y = target_scroll_y;
			}
		}

		if self.panning {
			let dist = (target_scroll_x - self.pan_target_x).abs() + (target_scroll_y - self.pan_target_y).abs();
			if dist as usize > self.maximum_snap_dist {
				// If moving fast while panning, try to keep camera at fastest movement speed, which is halfway
				// in the animation cycle
				let target_pan_frame = self.pan_time / 2;
				if self.pan_frame >= target_pan_frame {
					// Past target animation frame, move backwards and solve for the start position that would
					// leave the camera at its current location
					if self.pan_frame == target_pan_frame {
						self.pan_frame -= 1;
					} else {
						self.pan_frame -= 2;
					}

					let frac = ((self.pan_frame as f32 / self.pan_time as f32) * PI + PI).cos() * 0.5 + 0.5;
					self.pan_start_x = ((*scroll_x as f32 - (target_scroll_x as f32 * frac)) / (1.0 - frac) + 0.5) as isize;
					self.pan_start_y = ((*scroll_y as f32 - (target_scroll_y as f32 * frac)) / (1.0 - frac) + 0.5) as isize;
				}
			}

			self.pan_target_x = target_scroll_x;
			self.pan_target_y = target_scroll_y;

			self.pan_frame += 1;
			let frac = ((self.pan_frame as f32 / self.pan_time as f32) * PI + PI).cos() * 0.5 + 0.5;
			*scroll_x = ((target_scroll_x as f32 * frac) + (self.pan_start_x as f32 * (1.0 - frac)) + 0.5) as isize;
			*scroll_y = ((target_scroll_y as f32 * frac) + (self.pan_start_y as f32 * (1.0 - frac)) + 0.5) as isize;

			if self.pan_frame >= self.pan_time {
				self.panning = false;
			}
		} else {
			*scroll_x = target_scroll_x;
			*scroll_y = target_scroll_y;
		}
	}

	pub fn start_following(&mut self, actor: &ActorRef) {
		self.follow_actor = Some(actor.clone());
	}

	pub fn stop_following(&mut self) {
		self.follow_actor = None;
	}

	pub fn tick(&mut self, render_size: &RenderSize, scroll_x: &mut isize, scroll_y: &mut isize) {
		let actor_ref = match &self.follow_actor {
			Some(actor) => Some(actor.clone()),
			None => None
		};

		if let Some(actor) = actor_ref {
			let actor = actor.borrow();
			let actor_info = actor.actor_info();
			let focus_offset = actor.get_camera_focus_offset();
			let mut x = actor_info.x + focus_offset.0;
			let mut y = actor_info.y + focus_offset.1;
			self.move_focus_to_point(x, y, render_size, scroll_x, scroll_y);
		}
	}
}
