extern crate serde_json;

use std::io;
use std::io::Read;
use std::fs::File;
use std::path::{Path, PathBuf};
use std::collections::HashMap;
use std::rc::Rc;
use palette::Palette;
use tile::TileSet;
use sprite::Sprite;
use map::MapLayer;
use map::Map;

pub static RUNTIME_ASSET: &str = "runtime";

#[derive(Serialize, Deserialize)]
struct Manifest {
	palettes: Vec<String>,
	tilesets: Vec<String>,
	effect_layers: Vec<String>,
	maps: Vec<String>,
	sprites: Vec<String>
}

#[derive(Clone)]
pub struct AssetNamespace {
	palettes_by_id: HashMap<String, Rc<Palette>>,
	palettes_by_name: HashMap<String, Rc<Palette>>,
	tile_sets_by_id: HashMap<String, Rc<TileSet>>,
	tile_sets_by_name: HashMap<String, Rc<TileSet>>,
	effect_layers_by_id: HashMap<String, Rc<MapLayer>>,
	effect_layers_by_name: HashMap<String, Rc<MapLayer>>,
	maps_by_id: HashMap<String, Rc<Map>>,
	maps_by_name: HashMap<String, Rc<Map>>,
	sprites_by_id: HashMap<String, Rc<Sprite>>,
	sprites_by_name: HashMap<String, Rc<Sprite>>
}

impl AssetNamespace {
	pub fn new() -> AssetNamespace {
		AssetNamespace {
			palettes_by_id: HashMap::new(),
			palettes_by_name: HashMap::new(),
			tile_sets_by_id: HashMap::new(),
			tile_sets_by_name: HashMap::new(),
			effect_layers_by_id: HashMap::new(),
			effect_layers_by_name: HashMap::new(),
			maps_by_id: HashMap::new(),
			maps_by_name: HashMap::new(),
			sprites_by_id: HashMap::new(),
			sprites_by_name: HashMap::new()
		}
	}

	pub fn import(&mut self, path: &Path) -> Result<Vec<String>, io::Error> {
		let manifest: Manifest = serde_json::from_str(&load_asset_string(path, "manifest.json")?)?;
		let mut registered_assets: Vec<String> = Vec::new();

		for name in manifest.palettes {
			let palette = Palette::import(&load_asset_string(path, &name)?)?;
			registered_assets.push(palette.id.clone());
			self.palettes_by_id.insert(palette.id.clone(), Rc::clone(&palette));
			self.palettes_by_name.insert(palette.name.clone(), Rc::clone(&palette));
		}

		for name in manifest.tilesets {
			let tile_set = TileSet::import(&self, &load_asset_string(path, &name)?)?;
			registered_assets.push(tile_set.id.clone());
			self.tile_sets_by_id.insert(tile_set.id.clone(), Rc::clone(&tile_set));
			self.tile_sets_by_name.insert(tile_set.name.clone(), Rc::clone(&tile_set));
		}

		for name in manifest.effect_layers {
			let layer = MapLayer::import_effect_layer(&self, &load_asset_string(path, &name)?)?;
			registered_assets.push(layer.id.clone());
			self.effect_layers_by_id.insert(layer.id.clone(), Rc::clone(&layer));
			self.effect_layers_by_name.insert(layer.name.clone(), Rc::clone(&layer));
		}

		for name in manifest.maps {
			let map = Map::import(&self, &load_asset_string(path, &name)?)?;
			registered_assets.push(map.id.clone());
			self.maps_by_id.insert(map.id.clone(), Rc::clone(&map));
			self.maps_by_name.insert(map.name.clone(), Rc::clone(&map));
		}

		for name in manifest.sprites {
			let sprite = Sprite::import(&self, &load_asset_string(path, &name)?)?;
			registered_assets.push(sprite.id.clone());
			self.sprites_by_id.insert(sprite.id.clone(), Rc::clone(&sprite));
			self.sprites_by_name.insert(sprite.name.clone(), Rc::clone(&sprite));
		}

		return Ok(registered_assets);
	}

	pub fn get_palette_by_id(&self, id: &str) -> Option<Rc<Palette>> {
		if let Some(palette) = self.palettes_by_id.get(id) {
			Some(Rc::clone(palette))
		} else {
			None
		}
	}

	pub fn get_palette_by_name(&self, name: &str) -> Option<Rc<Palette>> {
		if let Some(palette) = self.palettes_by_name.get(name) {
			Some(Rc::clone(palette))
		} else {
			None
		}
	}

	pub fn get_tile_set_by_id(&self, id: &str) -> Option<Rc<TileSet>> {
		if let Some(tile_set) = self.tile_sets_by_id.get(id) {
			Some(Rc::clone(tile_set))
		} else {
			None
		}
	}

	pub fn get_tile_set_by_name(&self, name: &str) -> Option<Rc<TileSet>> {
		if let Some(tile_set) = self.tile_sets_by_name.get(name) {
			Some(Rc::clone(tile_set))
		} else {
			None
		}
	}

	pub fn get_effect_layer_by_id(&self, id: &str) -> Option<Rc<MapLayer>> {
		if let Some(layer) = self.effect_layers_by_id.get(id) {
			Some(Rc::clone(layer))
		} else {
			None
		}
	}

	pub fn get_effect_layer_by_name(&self, name: &str) -> Option<Rc<MapLayer>> {
		if let Some(layer) = self.effect_layers_by_name.get(name) {
			Some(Rc::clone(layer))
		} else {
			None
		}
	}

	pub fn get_map_by_id(&self, id: &str) -> Option<Rc<Map>> {
		if let Some(map) = self.maps_by_id.get(id) {
			Some(Rc::clone(map))
		} else {
			None
		}
	}

	pub fn get_map_by_name(&self, name: &str) -> Option<Rc<Map>> {
		if let Some(map) = self.maps_by_name.get(name) {
			Some(Rc::clone(map))
		} else {
			None
		}
	}

	pub fn get_sprite_by_id(&self, id: &str) -> Option<Rc<Sprite>> {
		if let Some(sprite) = self.sprites_by_id.get(id) {
			Some(Rc::clone(sprite))
		} else {
			None
		}
	}

	pub fn get_sprite_by_name(&self, name: &str) -> Option<Rc<Sprite>> {
		if let Some(sprite) = self.sprites_by_name.get(name) {
			Some(Rc::clone(sprite))
		} else {
			None
		}
	}
}

fn load_asset_string(path: &Path, name: &str) -> Result<String, io::Error> {
	let asset_path: PathBuf = [path, Path::new(name)].iter().collect();
	let mut result = String::new();
	File::open(asset_path)?.read_to_string(&mut result)?;
	Ok(result)
}
