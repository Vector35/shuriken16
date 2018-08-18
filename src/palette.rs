extern crate serde_json;

use std::io;
use std::rc::Rc;

#[derive(Serialize, Deserialize, Debug)]
pub struct Palette {
	pub name: String,
	pub id: String,
	pub entries: Vec<u16>
}

impl Palette {
	pub fn import(data: &String) -> Result<Rc<Palette>, io::Error> {
		return Ok(Rc::new(serde_json::from_str(data)?));
	}
}
