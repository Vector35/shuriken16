extern crate serde_json;

use std::io;
use std::rc::Rc;

#[derive(Serialize, Deserialize)]
pub struct RawPalette {
	pub name: String,
	pub id: String,
	pub entries: Vec<u16>
}

pub struct Palette {
	pub name: String,
	pub id: String,
	pub entries: Vec<u32>
}

impl Palette {
	pub fn import(data: &String) -> Result<Rc<Palette>, io::Error> {
		let raw_palette: RawPalette = serde_json::from_str(data)?;
		let mut palette = Palette {
			name: raw_palette.name,
			id: raw_palette.id,
			entries: Vec::new()
		};
		for color in raw_palette.entries {
			palette.entries.push(Palette::convert_color(color));
		}
		return Ok(Rc::new(palette));
	}

	pub fn convert_color(color: u16) -> u32 {
		((color as u32 & 0x1f) << 3) | ((color as u32 & 0x3e0) << 6) | ((color as u32 & 0x7c00) << 9)
	}
}
