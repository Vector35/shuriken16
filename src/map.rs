extern crate serde_json;

use std::io;
use std::rc::Rc;
use tile::TileSet;
use asset;
use asset::AssetNamespace;

#[derive(Serialize, Deserialize)]
struct RawMapLayer {
	pub name: String,
	pub id: String,
	pub width: usize,
	pub height: usize,
	pub tile_width: usize,
	pub tile_height: usize,
	pub tile_depth: usize,
	pub tile_sets: Vec<String>,
	pub tiles: Vec<Vec<usize>>,
	pub effect: bool,
	pub blend: u32,
	pub alpha: u8,
	pub parallax_x: i16,
	pub parallax_y: i16,
	pub auto_scroll_x: i16,
	pub auto_scroll_y: i16
}

#[derive(Serialize, Deserialize)]
struct RawMapLayerRef {
	pub normal: Option<RawMapLayer>,
	pub effect: Option<String>
}

#[derive(Serialize, Deserialize)]
struct RawMap {
	pub name: String,
	pub id: String,
	pub background_color: u16,
	pub layers: Vec<RawMapLayerRef>,
	pub main_layer: isize
}

#[derive(Clone, Debug)]
pub enum BlendMode {
	Normal,
	Add,
	Subtract,
	Multiply
}

#[derive(Clone)]
pub struct TileRef {
	pub tile_set: Rc<TileSet>,
	pub tile_index: usize
}

#[derive(Clone)]
pub struct MapLayer {
	pub name: String,
	pub id: String,
	pub width: usize,
	pub height: usize,
	pub tile_width: usize,
	pub tile_height: usize,
	pub tile_depth: usize,
	pub tiles: Vec<Option<TileRef>>,
	pub effect: bool,
	pub blend_mode: BlendMode,
	pub alpha: u8,
	pub parallax_x: i16,
	pub parallax_y: i16,
	pub auto_scroll_x: i16,
	pub auto_scroll_y: i16
}

#[derive(Clone)]
pub struct Map {
	pub name: String,
	pub id: String,
	pub background_color: u16,
	pub layers: Vec<Rc<MapLayer>>,
	pub main_layer: Option<usize>
}

impl MapLayer {
	pub fn new(name: &str, width: usize, height: usize, tile_width: usize, tile_height: usize, tile_depth: usize) -> MapLayer {
		let mut layer = MapLayer {
			name: name.to_string(),
			id: asset::RUNTIME_ASSET.to_string(),
			width, height, tile_width, tile_height, tile_depth,
			tiles: Vec::new(),
			effect: false,
			blend_mode: BlendMode::Normal,
			alpha: 0,
			parallax_x: 0x100,
			parallax_y: 0x100,
			auto_scroll_x: 0,
			auto_scroll_y: 0
		};
		layer.tiles.resize(width * height, None);

		layer
	}

	fn import_from_raw_layer(assets: &AssetNamespace, raw_map_layer: RawMapLayer, effect: bool) -> Result<Rc<MapLayer>, io::Error> {
		let mut map_layer = MapLayer {
			name: raw_map_layer.name,
			id: raw_map_layer.id,
			width: raw_map_layer.width,
			height: raw_map_layer.height,
			tile_width: raw_map_layer.tile_width,
			tile_height: raw_map_layer.tile_height,
			tile_depth: raw_map_layer.tile_depth,
			tiles: Vec::new(),
			effect: raw_map_layer.effect,
			blend_mode: match raw_map_layer.blend {
				0 => BlendMode::Normal,
				1 => BlendMode::Add,
				2 => BlendMode::Subtract,
				3 => BlendMode::Multiply,
				_ => return Err(io::Error::new(io::ErrorKind::InvalidData, "Invalid blend mode"))
			},
			alpha: raw_map_layer.alpha,
			parallax_x: raw_map_layer.parallax_x,
			parallax_y: raw_map_layer.parallax_y,
			auto_scroll_x: raw_map_layer.auto_scroll_x,
			auto_scroll_y: raw_map_layer.auto_scroll_y
		};

		// Check effect layer flag for asset import type
		if effect && (!map_layer.effect) {
			return Err(io::Error::new(io::ErrorKind::InvalidData, "Non-effect layer in effect layer asset"));
		}
		if (!effect) && map_layer.effect {
			return Err(io::Error::new(io::ErrorKind::InvalidData, "Effect layer used as normal map layer asset"));
		}

		// Non-effect layers must not have parallax or auto scrolling
		if (!map_layer.effect) && ((map_layer.parallax_x != 0x100) || (map_layer.parallax_y != 0x100) ||
			(map_layer.auto_scroll_x != 0) || (map_layer.auto_scroll_y != 0)) {
			return Err(io::Error::new(io::ErrorKind::InvalidData, "Non-effect layers cannot have scrolling effects"));
		}

		// Check tile count for given width and height
		if raw_map_layer.tiles.len() != (map_layer.width * map_layer.height) {
			return Err(io::Error::new(io::ErrorKind::InvalidData, "Tile count does not match width and height"));
		}

		// Resolve tile sets
		let mut tile_sets = Vec::new();
		for tile_set_id in raw_map_layer.tile_sets {
			let tile_set = match assets.get_tile_set_by_id(&tile_set_id) {
				Some(found_tile_set) => found_tile_set,
				None => return Err(io::Error::new(io::ErrorKind::InvalidData,
					format!("Tile set {} not found", tile_set_id)))
			};

			if (map_layer.tile_width != tile_set.width) ||
				(map_layer.tile_height != tile_set.height) ||
				(map_layer.tile_depth != tile_set.depth) {
				return Err(io::Error::new(io::ErrorKind::InvalidData,
					format!("Tile set {} does not match tile size for this map layer", tile_set_id)));
			}

			tile_sets.push(Rc::clone(&tile_set));
		}

		// Resolve individual tiles
		for raw_tile in raw_map_layer.tiles {
			let tile = match raw_tile.len() {
				0 => None,
				2 => {
					let tile_set_index = raw_tile[0];
					let tile_index = raw_tile[1];
					if tile_set_index >= tile_sets.len() {
						return Err(io::Error::new(io::ErrorKind::InvalidData, "Invalid tile set reference"));
					}
					Some(TileRef { tile_set: Rc::clone(&tile_sets[tile_set_index]), tile_index })
				},
				_ => return Err(io::Error::new(io::ErrorKind::InvalidData, "Invalid tile format"))
			};
			map_layer.tiles.push(tile)
		}

		Ok(Rc::new(map_layer))
	}

	fn import(assets: &AssetNamespace, data: &str, effect: bool) -> Result<Rc<MapLayer>, io::Error> {
		let raw_map_layer: RawMapLayer = serde_json::from_str(data)?;
		MapLayer::import_from_raw_layer(assets, raw_map_layer, effect)
	}

	pub fn import_normal_layer(assets: &AssetNamespace, data: &str) -> Result<Rc<MapLayer>, io::Error> {
		MapLayer::import(assets, data, false)
	}

	pub fn import_effect_layer(assets: &AssetNamespace, data: &str) -> Result<Rc<MapLayer>, io::Error> {
		MapLayer::import(assets, data, true)
	}

	pub fn get_tile(&self, x: usize, y: usize) -> &Option<TileRef> {
		&self.tiles[(y * self.width) + x]
	}

	pub fn set_tile(&mut self, x: usize, y: usize, tile: Option<TileRef>) {
		self.tiles[(y * self.width) + x] = tile;
	}

	pub fn resize(&mut self, width: usize, height: usize) {
		let mut new_tiles = Vec::new();
		new_tiles.resize(width * height, None);

		let mut copy_width = width;
		let mut copy_height = height;
		if copy_width > self.width {
			copy_width = self.width;
		}
		if copy_height > self.height {
			copy_height = self.height;
		}
		for y in 0..copy_height {
			for x in 0..copy_width {
				new_tiles[(y * width) + x] = self.tiles[(y * self.width) + x].clone();
			}
		}

		self.width = width;
		self.height = height;
		self.tiles = new_tiles;
	}
}

impl Map {
	pub fn new(name: &str) -> Map {
		Map {
			name: name.to_string(),
			id: asset::RUNTIME_ASSET.to_string(),
			background_color: 0,
			layers: Vec::new(),
			main_layer: None
		}
	}

	pub fn import(assets: &AssetNamespace, data: &str) -> Result<Rc<Map>, io::Error> {
		let raw_map: RawMap = serde_json::from_str(data)?;
		let mut map = Map {
			name: raw_map.name,
			id: raw_map.id,
			background_color: raw_map.background_color,
			layers: Vec::new(),
			main_layer: match raw_map.main_layer {
				-1 => None,
				n if n < 0 => return Err(io::Error::new(io::ErrorKind::InvalidData, "Invalid main layer")),
				_ => Some(raw_map.main_layer as usize)
			}
		};

		if let Some(main_layer) = map.main_layer {
			if main_layer >= raw_map.layers.len() {
				return Err(io::Error::new(io::ErrorKind::InvalidData, "Invalid main layer"))
			}
		}

		for raw_layer in raw_map.layers {
			let layer = match raw_layer.normal {
				Some(normal_raw_layer) => {
					match raw_layer.effect {
						Some(_) => return Err(io::Error::new(io::ErrorKind::InvalidData, "Layer has multiple definitions")),
						_ => ()
					};
					MapLayer::import_from_raw_layer(assets, normal_raw_layer, false)?
				}
				None => {
					match raw_layer.effect {
						Some(effect_layer_id) => {
							match assets.get_effect_layer_by_id(&effect_layer_id) {
								Some(effect_layer) => effect_layer,
								None => return Err(io::Error::new(io::ErrorKind::InvalidData,
									format!("Effect layer {} not found", effect_layer_id)))
							}
						},
						None => return Err(io::Error::new(io::ErrorKind::InvalidData, "Layer has no definition"))
					}
				}
			};
			map.layers.push(layer)
		}

		Ok(Rc::new(map))
	}
}
