#pragma once

#include <string>
#include <vector>
#include <memory>
#include "tileset.h"
#include "json/json.h"

struct TileReference
{
	std::shared_ptr<TileSet> tileSet;
	uint16_t index;

	TileReference(): index(0) {}
	TileReference(std::shared_ptr<TileSet> s, uint16_t i): tileSet(s), index(i) {}
};

class Project;

enum BlendMode
{
	BlendMode_Normal,
	BlendMode_Add,
	BlendMode_Subtract,
	BlendMode_Multiply
};

class MapLayer
{
	struct SmartTileContext
	{
		bool tile[5][5];
		bool Empty(int x, int y);
		bool Filled(int x, int y);
	};

	std::string m_name;
	std::string m_id;
	size_t m_width, m_height;
	size_t m_tileWidth, m_tileHeight, m_tileDepth;
	std::vector<TileReference> m_tiles;

	bool m_effectLayer;
	BlendMode m_blendMode;
	uint8_t m_alpha;
	int16_t m_parallaxFactorX, m_parallaxFactorY;
	int16_t m_autoScrollX, m_autoScrollY;

	bool IsCompatibleForSmartTiles(size_t x, size_t y, const std::shared_ptr<TileSet>& tileSet);
	SmartTileContext GetContextForSmartTile(size_t x, size_t y, const std::shared_ptr<TileSet>& tileSet);
	void UpdateSimplifiedSingleWidthSmartTileSet(size_t x, size_t y, const std::shared_ptr<TileSet>& tileSet);
	void UpdateSimplifiedDoubleWidthSmartTileSet(size_t x, size_t y, const std::shared_ptr<TileSet>& tileSet);

public:
	MapLayer(size_t width, size_t height, size_t tileWidth, size_t tileHeight, size_t tileDepth,
		bool effectLayer = false);
	MapLayer(const MapLayer& other);

	const std::string& GetName() const { return m_name; }
	void SetName(const std::string& name) { m_name = name; }

	size_t GetWidth() const { return m_width; }
	size_t GetHeight() const { return m_height; }
	void SetSize(size_t width, size_t height);

	size_t GetTileWidth() const { return m_tileWidth; }
	size_t GetTileHeight() const { return m_tileHeight; }
	size_t GetTileDepth() const { return m_tileDepth; }

	TileReference GetTileAt(size_t x, size_t y);
	void SetTileAt(size_t x, size_t y, const TileReference& tile);

	bool IsEffectLayer() const { return m_effectLayer; }
	void SetIsEffectLayer(bool effectLayer) { m_effectLayer = effectLayer; }
	BlendMode GetBlendMode() const { return m_blendMode; }
	void SetBlendMode(BlendMode blendMode) { m_blendMode = blendMode; }
	uint8_t GetAlpha() const { return m_alpha; }
	void SetAlpha(uint8_t alpha) { m_alpha = alpha; }
	int16_t GetParallaxFactorX() const { return m_parallaxFactorX; }
	int16_t GetParallaxFactorY() const { return m_parallaxFactorY; }
	void SetParallaxFactorX(int16_t x) { m_parallaxFactorX = x; }
	void SetParallaxFactorY(int16_t y) { m_parallaxFactorY = y; }
	int16_t GetAutoScrollX() const { return m_autoScrollX; }
	int16_t GetAutoScrollY() const { return m_autoScrollY; }
	void SetAutoScrollX(int16_t x) { m_autoScrollX = x; }
	void SetAutoScrollY(int16_t y) { m_autoScrollY = y; }

	void UpdateSmartTile(size_t x, size_t y);
	void UpdateRegionForSmartTiles(int x, int y, int w, int h);

	bool UsesTileSet(std::shared_ptr<TileSet> tileSet);

	const std::string& GetId() const { return m_id; }
	Json::Value Serialize();
	static std::shared_ptr<MapLayer> Deserialize(std::shared_ptr<Project> project, const Json::Value& data);
};
