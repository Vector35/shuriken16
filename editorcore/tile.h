#pragma once

#include <memory>
#include <inttypes.h>
#include "palette.h"
#include "json/json.h"

class Project;

class Tile
{
	uint16_t m_width, m_height, m_depth, m_frames;
	uint8_t* m_data;
	size_t m_size, m_frameSize;
	std::shared_ptr<Palette> m_palette;
	uint8_t m_paletteOffset;

public:
	Tile(uint16_t width, uint16_t height, uint16_t depth = 4, uint16_t frames = 1);
	Tile(const Tile& other);
	~Tile();

	uint16_t GetWidth() const { return m_width; }
	uint16_t GetHeight() const { return m_height; }
	uint16_t GetDepth() const { return m_depth; }
	uint16_t GetFrameCount() const { return m_frames; }
	uint8_t* GetData() { return m_data; }
	const uint8_t* GetData() const { return m_data; }
	uint8_t* GetData(uint16_t frame);
	const uint8_t* GetData(uint16_t frame) const;
	size_t GetSize() { return m_size; }
	size_t GetPerFrameSize() { return m_frameSize; }

	void SetFrameCount(uint16_t frames);
	void CopyFrame(uint16_t from, uint16_t to);
	void SwapFrames(uint16_t from, uint16_t to);
	void DuplicateFrame(uint16_t frame);
	void RemoveFrame(uint16_t frame);
	std::shared_ptr<Tile> GetTileForSingleFrame(uint16_t frame);
	void InsertFrameFromTile(uint16_t frame, const std::shared_ptr<Tile>& tile);

	std::shared_ptr<Palette> GetPalette() const { return m_palette; }
	uint8_t GetPaletteOffset() const { return m_paletteOffset; }
	void SetPalette(const std::shared_ptr<Palette> palette, uint8_t offset);

	Json::Value Serialize();
	static std::shared_ptr<Tile> Deserialize(std::shared_ptr<Project> project, const Json::Value& data,
		size_t width, size_t height, size_t depth, size_t frames);
};
