#pragma once

#include <string>
#include "animation.h"
#include "tile.h"

class SpriteAnimation
{
	std::string m_name;
	std::shared_ptr<Tile> m_tile;
	std::shared_ptr<Animation> m_animation;
	bool m_loop;

public:
	SpriteAnimation(const std::string& name, size_t width, size_t height, size_t depth);
	SpriteAnimation(const SpriteAnimation& other);

	const std::string& GetName() const { return m_name; }
	void SetName(const std::string& name) { m_name = name; }

	bool IsLooping() const { return m_loop; }
	void SetLooping(bool loop) { m_loop = loop; }

	size_t GetFrameCount() const;

	std::shared_ptr<Tile> GetTile() { return m_tile; }

	std::shared_ptr<Animation> GetAnimation() const { return m_animation; }
	void SetAnimation(std::shared_ptr<Animation> anim);
	uint16_t GetFrameForTime(uint32_t ticks);
	void CopyFrame(uint16_t from, uint16_t to);
	void SwapFrames(uint16_t from, uint16_t to);
	void DuplicateFrame(uint16_t frame);
	void RemoveFrame(uint16_t frame);
	std::shared_ptr<Tile> GetTileForSingleFrame(uint16_t frame);
	void InsertFrameFromTile(uint16_t frame, const std::shared_ptr<Tile>& tile, uint16_t length);

	bool UsesPalette(std::shared_ptr<Palette> palette);

	Json::Value Serialize();
	static std::shared_ptr<SpriteAnimation> Deserialize(std::shared_ptr<Project> project, const Json::Value& data,
		size_t width, size_t height, size_t depth);
};
