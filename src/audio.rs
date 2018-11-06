extern crate lewton;
extern crate sdl2;

use self::lewton::inside_ogg::OggStreamReader;
use self::sdl2::audio::AudioCallback;
use std::cell::RefCell;
use std::sync::{Arc, Mutex};
use std::io::Cursor;
use std::collections::VecDeque;

pub const AUDIO_TYPE_MUSIC: usize = 0;
pub const AUDIO_TYPE_GAME: usize = 1;
pub const AUDIO_TYPE_UI: usize = 2;

pub trait AudioSource: Send {
	fn next_sample(&mut self) -> (i16, i16);
	fn done(&self) -> bool;
}

pub enum SoundFade {
	NoFade,
	FadeIn(u32),
	FadeOut(u32)
}

pub struct Sound {
	pub source: Box<AudioSource>,
	pub volume: u8,
	pub pan: u8,
	pub fade: SoundFade,
	pub fade_sample: u32,
	pub destroyed: bool,
	pub audio_type: usize
}

pub type SoundRef = Arc<Mutex<RefCell<Sound>>>;

pub struct AudioMixer {
	pub sounds: Vec<SoundRef>,
	pub type_volumes: Vec<u8>
}

pub type AudioMixerRef = Arc<Mutex<RefCell<AudioMixer>>>;

pub struct AudioMixerCallback {
	pub mixer: AudioMixerRef
}

pub struct OggAudioSource {
	stream: OggStreamReader<Cursor<Vec<u8>>>,
	data: Vec<u8>,
	pending_samples: VecDeque<(i16, i16)>
}

impl AudioCallback for AudioMixerCallback {
	type Channel = i16;

	fn callback(&mut self, out: &mut [i16]) {
		let mixer_lock = self.mixer.lock().unwrap();
		let mut mixer = mixer_lock.borrow_mut();
		for i in 0..out.len() / 2 {
			let mut left: i16 = 0;
			let mut right: i16 = 0;
			for sound_ref in &mixer.sounds {
				let sound_lock = sound_ref.lock().unwrap();
				let mut sound = sound_lock.borrow_mut();
				if sound.destroyed {
					continue;
				}
				if sound.source.done() {
					sound.destroyed = true;
					continue;
				}
				if let SoundFade::FadeIn(samples) = sound.fade {
					sound.fade_sample += 1;
					if sound.fade_sample >= samples {
						if sound.volume == 255 {
							sound.fade = SoundFade::NoFade;
						} else {
							sound.volume += 1;
						}
						sound.fade_sample = 0;
					}
				} else if let SoundFade::FadeOut(samples) = sound.fade {
					sound.fade_sample += 1;
					if sound.fade_sample >= samples {
						if sound.volume == 0 {
							sound.destroyed = true;
							sound.fade = SoundFade::NoFade;
						} else {
							sound.volume -= 1;
						}
						sound.fade_sample = 0;
					}
				}
				let (mut cur_left, mut cur_right) = sound.source.next_sample();
				let type_volume = mixer.type_volumes[sound.audio_type];
				let volume = (sound.volume as i32 * type_volume as i32) / 255;
				let left_volume = (volume * i32::max(sound.pan as i32, 128)) / 128;
				let right_volume = (volume * (255 - i32::min(sound.pan as i32, 128))) / 127;
				cur_left = ((cur_left as i32 * left_volume) / 255) as i16;
				cur_right = ((cur_right as i32 * right_volume) / 255) as i16;
				left = left.saturating_add(cur_left);
				right = right.saturating_add(cur_right);
			}
			out[i * 2] = left;
			out[i * 2 + 1] = right;
		}
		let mut new_sounds = Vec::new();
		for sound_ref in &mixer.sounds {
			let sound_lock = sound_ref.lock().unwrap();
			let mut sound = sound_lock.borrow_mut();
			if !sound.destroyed {
				new_sounds.push(sound_ref.clone());
			}
		}
		mixer.sounds = new_sounds;
	}
}

impl Sound {
	pub fn fade_out(&mut self, fade_time: f32) {
		self.fade = SoundFade::FadeOut(((44100.0 * fade_time) / 255.0) as u32);
		self.fade_sample = 0;
	}

	pub fn destroy(&mut self) {
		self.destroyed = true;
	}
}

impl AudioMixer {
	pub fn new() -> AudioMixerRef {
		let mixer = AudioMixer {
			sounds: Vec::new(),
			type_volumes: vec![255, 255, 255]
		};
		Arc::new(Mutex::new(RefCell::new(mixer)))
	}

	pub fn set_type_volume(&mut self, audio_type: usize, volume: u8) {
		self.type_volumes[audio_type] = volume;
	}

	pub fn play(&mut self, source: Box<AudioSource>, audio_type: usize) -> SoundRef {
		let sound = Sound {
			source,
			volume: 255,
			pan: 255,
			fade: SoundFade::NoFade,
			fade_sample: 0,
			destroyed: false,
			audio_type
		};
		let sound_ref = Arc::new(Mutex::new(RefCell::new(sound)));
		self.sounds.push(sound_ref.clone());
		sound_ref
	}

	pub fn play_fade_in(&mut self, source: Box<AudioSource>, fade_time: f32, audio_type: usize) -> SoundRef {
		let sound = Sound {
			source,
			volume: 0,
			pan: 255,
			fade: SoundFade::FadeIn(((44100.0 * fade_time) / 255.0) as u32),
			fade_sample: 0,
			destroyed: false,
			audio_type
		};
		let sound_ref = Arc::new(Mutex::new(RefCell::new(sound)));
		self.sounds.push(sound_ref.clone());
		sound_ref
	}
}

impl AudioMixerCallback {
	pub fn new(mixer: &AudioMixerRef) -> AudioMixerCallback {
		AudioMixerCallback {
			mixer: mixer.clone()
		}
	}
}

impl AudioSource for OggAudioSource {
	fn next_sample(&mut self) -> (i16, i16) {
		while self.pending_samples.is_empty() {
			let mut reached_end = false;
			match self.stream.read_dec_packet_itl().unwrap() {
				Some(samples) => {
					for i in 0..samples.len() / 2 {
						self.pending_samples.push_back((samples[i * 2], samples[i * 2 + 1]));
					}
				},
				None => {
					reached_end = true;
				}
			};
			if reached_end {
				self.stream = OggStreamReader::new(Cursor::new(self.data.clone())).unwrap();
			}
		}

		if self.pending_samples.is_empty() {
			return (0, 0);
		}

		return self.pending_samples.pop_front().unwrap();
	}

	fn done(&self) -> bool {
		false
	}
}

impl OggAudioSource {
	pub fn new(data: Vec<u8>) -> Box<AudioSource> {
		let cursor = Cursor::new(data.clone());
		let stream = OggStreamReader::new(cursor).unwrap();
		Box::new(OggAudioSource {
			stream,
			data,
			pending_samples: VecDeque::new()
		})
	}
}
