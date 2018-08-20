#[macro_use]
extern crate serde_derive;

pub mod render;
pub mod palette;
pub mod tile;
pub mod sprite;
pub mod map;
pub mod asset;
pub mod game;
pub mod ui;
pub mod actor;

#[cfg(target_os = "emscripten")]
pub mod emscripten;
