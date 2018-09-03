extern crate serde_json;
extern crate hex;

use std::io;
use std::rc::Rc;
use std::collections::HashMap;
use asset;
use asset::AssetNamespace;
use tile::{PaletteWithOffset, Animation};

#[derive(Serialize, Deserialize)]
struct RawSpriteTile {
	pub palette: Option<String>,
	pub offset: Option<usize>,
	pub data: String
}

#[derive(Serialize, Deserialize)]
struct RawSpriteAnimation {
	name: String,
	tile: RawSpriteTile,
	anim: Vec<usize>,
	looping: bool
}

#[derive(Serialize, Deserialize)]
struct RawSprite {
	name: String,
	id: String,
	width: usize,
	height: usize,
	depth: usize,
	anim: Vec<RawSpriteAnimation>
}

pub struct SpriteAnimation {
	pub name: String,
	pub width: usize,
	pub height: usize,
	pub depth: usize,
	pub single_frame_size: usize,
	pub palette: Option<PaletteWithOffset>,
	pub data: Vec<u8>,
	pub animation: Animation,
	pub frames: usize,
	pub looping: bool
}

pub struct Sprite {
	pub name: String,
	pub id: String,
	pub width: usize,
	pub height: usize,
	pub depth: usize,
	pub single_frame_size: usize,
	pub animations: Vec<Rc<SpriteAnimation>>,
	pub animations_by_name: HashMap<String, Rc<SpriteAnimation>>
}

impl SpriteAnimation {
	pub fn data_for_frame(&self, frame: usize) -> &[u8] {
		&self.data[(frame * self.single_frame_size) .. ((frame + 1) * self.single_frame_size)]
	}

	pub fn frame_for_time(&self, t: usize) -> usize {
		if self.looping {
			self.animation.frame_for_time[t % self.animation.total_length]
		} else if t >= self.animation.total_length {
			*self.animation.frame_for_time.last().unwrap()
		} else {
			self.animation.frame_for_time[t]
		}
	}

	pub fn data_for_time(&self, t: usize) -> &[u8] {
		self.data_for_frame(self.frame_for_time(t))
	}
}

impl Sprite {
	pub fn new(name: &str, width: usize, height: usize, depth: usize) -> Sprite {
		Sprite {
			name: name.to_string(),
			id: asset::RUNTIME_ASSET.to_string(),
			width, height, depth,
			single_frame_size: {
				match depth {
					4 => ((width + 1) / 2) * height,
					8 => width * height,
					16 => width * height * 2,
					_ => panic!("Invalid sprite depth {}", depth)
				}
			},
			animations: Vec::new(),
			animations_by_name: HashMap::new()
		}
	}

	pub fn import(assets: &AssetNamespace, data: &str) -> Result<Rc<Sprite>, io::Error> {
		let raw_sprite: RawSprite = serde_json::from_str(data)?;
		let mut sprite = Sprite {
			name: raw_sprite.name,
			id: raw_sprite.id,
			width: raw_sprite.width,
			height: raw_sprite.height,
			depth: raw_sprite.depth,
			single_frame_size: {
				match raw_sprite.depth {
					4 => ((raw_sprite.width + 1) / 2) * raw_sprite.height,
					8 => raw_sprite.width * raw_sprite.height,
					16 => raw_sprite.width * raw_sprite.height * 2,
					_ => return Err(io::Error::new(io::ErrorKind::InvalidData,
						format!("Invalid sprite depth {}", raw_sprite.depth)))
				}
			},
			animations: Vec::new(),
			animations_by_name: HashMap::new()
		};

		for raw_sprite_anim in raw_sprite.anim {
			// Check animation length for sanity
			let mut total_length = 0;
			for frame_length in &raw_sprite_anim.anim {
				total_length += frame_length;
			}

			if total_length == 0 {
				return Err(io::Error::new(io::ErrorKind::InvalidData, "Animation with zero length"));
			}
			if total_length >= 0x10000 {
				return Err(io::Error::new(io::ErrorKind::InvalidData, "Animation too long"));
			}

			let animation = Animation::new(raw_sprite_anim.anim);
			let frames = animation.frame_lengths.len();

			// If palette is valid, look up palette in asset namespace
			let palette = match raw_sprite_anim.tile.palette {
				Some(palette_id) =>
					match assets.get_palette_by_id(&palette_id) {
						Some(found_palette) => Some(PaletteWithOffset {
								palette: found_palette,
								offset: match raw_sprite_anim.tile.offset {
									Some(o) => o,
									None => 0
								}
							}),
						None => return Err(io::Error::new(io::ErrorKind::InvalidData,
							format!("Palette {} not found", palette_id)))
					},
				None => None
			};

			// Decode tile data
			let data = match hex::decode(raw_sprite_anim.tile.data) {
				Ok(decoded_data) => decoded_data,
				Err(_) => return Err(io::Error::new(io::ErrorKind::InvalidData, "Sprite data is invalid"))
			};
			if data.len() != (frames * sprite.single_frame_size) {
				return Err(io::Error::new(io::ErrorKind::InvalidData, "Sprite data size is incorrect for its animation"));
			}

			let mut sprite_anim = SpriteAnimation {
				name: raw_sprite_anim.name,
				width: sprite.width,
				height: sprite.height,
				depth: sprite.depth,
				single_frame_size: sprite.single_frame_size,
				palette,
				data,
				animation,
				frames,
				looping: raw_sprite_anim.looping
			};

			let sprite_anim_rc = Rc::new(sprite_anim);
			sprite.animations.push(Rc::clone(&sprite_anim_rc));
			sprite.animations_by_name.insert(sprite_anim_rc.name.clone(), sprite_anim_rc);
		}

		Ok(Rc::new(sprite))
	}

	pub fn push(&mut self, animation: SpriteAnimation) {
		assert_eq!(self.width, animation.width);
		assert_eq!(self.height, animation.height);
		assert_eq!(self.depth, animation.depth);
		let animation_rc = Rc::new(animation);
		self.animations.push(Rc::clone(&animation_rc));
		self.animations_by_name.insert(animation_rc.name.clone(), animation_rc);
	}

	pub fn get_default_animation(&self) -> Rc<SpriteAnimation> {
		Rc::clone(&self.animations[0])
	}

	pub fn get_animation_by_name(&self, name: &str) -> Option<Rc<SpriteAnimation>> {
		match self.animations_by_name.get(name) {
			Some(animation) => Some(Rc::clone(animation)),
			None => None
		}
	}
}
