extern crate serde_json;
extern crate hex;

use std::io;
use std::rc::Rc;
use palette::Palette;
use asset;
use asset::AssetNamespace;

#[derive(Serialize, Deserialize)]
struct RawTile {
	pub palette: Option<String>,
	pub offset: Option<usize>,
	pub data: String
}

#[derive(Serialize, Deserialize)]
struct RawTileSet {
	pub name: String,
	pub id: String,
	pub width: usize,
	pub height: usize,
	pub depth: usize,
	pub tiles: Vec<RawTile>,
	pub anim: Option<Vec<usize>>
}

#[derive(Debug)]
pub struct PaletteWithOffset {
	pub palette: Rc<Palette>,
	pub offset: usize
}

#[derive(Debug)]
pub struct Tile {
	pub palette: Option<PaletteWithOffset>,
	pub data: Vec<u8>
}

#[derive(Debug)]
pub struct Animation {
	pub total_length: usize,
	pub frame_lengths: Vec<usize>,
	pub frame_for_time: Vec<usize>
}

#[derive(Debug)]
pub struct TileSet {
	pub name: String,
	pub id: String,
	pub width: usize,
	pub height: usize,
	pub depth: usize,
	pub frames: usize,
	pub single_frame_size: usize,
	pub tiles: Vec<Tile>,
	pub animation: Option<Animation>
}

impl Animation {
	pub fn new(frame_lengths: Vec<usize>) -> Animation {
		let mut total_length = 0;
		for frame_length in &frame_lengths {
			total_length += frame_length;
		}

		let mut frame_for_time = Vec::new();
		for (frame, frame_length) in frame_lengths.iter().enumerate() {
			for _ in 0..*frame_length {
				frame_for_time.push(frame)
			}
		}

		Animation { total_length, frame_lengths, frame_for_time }
	}
}

impl TileSet {
	pub fn new(name: &str, width: usize, height: usize, depth: usize, animation: Option<Animation>) -> TileSet {
		TileSet {
			name: name.to_string(),
			id: asset::RUNTIME_ASSET.to_string(),
			width, height, depth,
			frames: match &animation {
				Some(anim) => anim.frame_lengths.len(),
				None => 1
			},
			single_frame_size: {
				match depth {
					4 => ((width + 1) / 2) * height,
					8 => width * height,
					16 => width * height * 2,
					_ => panic!("Invalid tile depth {}", depth)
				}
			},
			tiles: Vec::new(),
			animation
		}
	}

	pub fn import(assets: &AssetNamespace, data: &str) -> Result<Rc<TileSet>, io::Error> {
		// Parse raw tile set data and populate basic information
		let raw_tile_set: RawTileSet = serde_json::from_str(data)?;
		let mut tile_set = TileSet {
			name: raw_tile_set.name,
			id: raw_tile_set.id,
			width: raw_tile_set.width,
			height: raw_tile_set.height,
			depth: raw_tile_set.depth,
			frames: 1,
			single_frame_size: {
				match raw_tile_set.depth {
					4 => ((raw_tile_set.width + 1) / 2) * raw_tile_set.height,
					8 => raw_tile_set.width * raw_tile_set.height,
					16 => raw_tile_set.width * raw_tile_set.height * 2,
					_ => return Err(io::Error::new(io::ErrorKind::InvalidData,
						format!("Invalid tile depth {}", raw_tile_set.depth)))
				}
			},
			tiles: Vec::new(),
			animation: None
		};

		// Process animation data
		if let Some(raw_animation) = raw_tile_set.anim {
			// Check animation length for sanity
			let mut total_length = 0;
			for frame_length in &raw_animation {
				total_length += frame_length;
			}

			if total_length == 0 {
				return Err(io::Error::new(io::ErrorKind::InvalidData, "Animation with zero length"));
			}
			if total_length >= 0x10000 {
				return Err(io::Error::new(io::ErrorKind::InvalidData, "Animation too long"));
			}

			let animation = Animation::new(raw_animation);
			tile_set.frames = animation.frame_lengths.len();
			tile_set.animation = Some(animation);
		}

		// Process tile data
		for raw_tile in raw_tile_set.tiles {
			// If palette is valid, look up palette in asset namespace
			let palette = match raw_tile.palette {
				Some(palette_id) =>
					match assets.get_palette_by_id(&palette_id) {
						Some(found_palette) => Some(PaletteWithOffset {
								palette: found_palette,
								offset: match raw_tile.offset {
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
			let data = match hex::decode(raw_tile.data) {
				Ok(decoded_data) => decoded_data,
				Err(_) => return Err(io::Error::new(io::ErrorKind::InvalidData, "Tile data is invalid"))
			};
			if data.len() != (tile_set.frames * tile_set.single_frame_size) {
				return Err(io::Error::new(io::ErrorKind::InvalidData, "Tile data size is incorrect for its tile set"));
			}

			tile_set.tiles.push(Tile { palette, data });
		}

		Ok(Rc::new(tile_set))
	}

	pub fn push(&mut self, tile: Tile) {
		assert!(tile.data.len() != (self.frames * self.single_frame_size), "Tile data size is incorrect for its tile set");
		self.tiles.push(tile);
	}

	pub fn data_for_frame(&self, tile: usize, frame: usize) -> &[u8] {
		&self.tiles[tile].data[(frame * self.single_frame_size) .. ((frame + 1) * self.single_frame_size)]
	}

	pub fn frame_for_time(&self, t: usize) -> usize {
		match &self.animation {
			Some(animation) => animation.frame_for_time[t % animation.total_length],
			None => 0
		}
	}

	pub fn data_for_time(&self, tile: usize, t: usize) -> &[u8] {
		self.data_for_frame(tile, self.frame_for_time(t))
	}
}
