#pragma once

#include <string>
#include <vector>
#include <memory>
#include <set>
#include <inttypes.h>
#include "tile.h"
#include "animation.h"
#include "json/json.h"

class Project;

enum SmartTileSetType
{
	NormalTileSet,
	SimplifiedSingleWidthSmartTileSet,
	SimplifiedDoubleWidthSmartTileSet
};

class TileSet
{
	std::string m_name;
	size_t m_width, m_height, m_depth;
	size_t m_displayCols;
	std::vector<std::shared_ptr<Tile>> m_tiles;
	std::shared_ptr<Animation> m_animation;
	std::string m_id;
	SmartTileSetType m_smartTileSetType;
	std::set<std::shared_ptr<TileSet>> m_associatedTileSets;
	std::set<std::string> m_initialAssociatedTileSetIds;

public:
	TileSet(size_t width, size_t height, size_t depth, SmartTileSetType smartTileSetType = NormalTileSet);
	TileSet(const TileSet& other);

	const std::string& GetName() const { return m_name; }
	void SetName(const std::string& name) { m_name = name; }

	bool IsSmartTileSet() const { return m_smartTileSetType != NormalTileSet; }
	SmartTileSetType GetSmartTileSetType() const { return m_smartTileSetType; }

	size_t GetWidth() const { return m_width; }
	size_t GetHeight() const { return m_height; }
	size_t GetDepth() const { return m_depth; }
	size_t GetFrameCount() const;

	const std::vector<std::shared_ptr<Tile>>& GetTiles() const { return m_tiles; }
	size_t GetTileCount() const { return m_tiles.size(); }
	std::shared_ptr<Tile> GetTile(size_t i);
	void SetTile(size_t i, std::shared_ptr<Tile> tile);
	void SetTileCount(size_t count);

	std::shared_ptr<Animation> GetAnimation() const { return m_animation; }
	void SetAnimation(std::shared_ptr<Animation> anim);
	uint16_t GetFrameForTime(uint32_t ticks);
	void CopyFrame(uint16_t from, uint16_t to);
	void SwapFrames(uint16_t from, uint16_t to);
	void DuplicateFrame(uint16_t frame);
	void RemoveFrame(uint16_t frame);
	std::vector<std::shared_ptr<Tile>> GetTilesForSingleFrame(uint16_t frame);
	void InsertFrameFromTiles(uint16_t frame, const std::vector<std::shared_ptr<Tile>>& tiles, uint16_t length);

	size_t GetDisplayColumns() const { return m_displayCols; }
	void SetDisplayColumns(size_t cols) { m_displayCols = cols; }

	std::shared_ptr<Tile> CreateTile();

	bool UsesPalette(std::shared_ptr<Palette> palette);

	const std::set<std::shared_ptr<TileSet>>& GetAssociatedTileSets() const { return m_associatedTileSets; }
	void AddAssociatedTileSet(const std::shared_ptr<TileSet>& tileSet);
	void RemoveAssociatedTileSet(const std::shared_ptr<TileSet>& tileSet);
	bool IsCompatibleForSmartTiles(const std::shared_ptr<TileSet>& tileSet);
	void ResolveInitialAssociatedTileSets(const std::shared_ptr<Project>& project);

	const std::string& GetId() const { return m_id; }
	Json::Value Serialize();
	static std::shared_ptr<TileSet> Deserialize(std::shared_ptr<Project> project, const Json::Value& data);

	static size_t GetTileCountForSmartTileSet(SmartTileSetType type);
	static size_t GetDisplayColumnsForSmartTileSet(SmartTileSetType type);
	static size_t GetDefaultTileForSmartTileSet(SmartTileSetType type);
};
