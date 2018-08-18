#pragma once

#include "tileset.h"
#include "maplayer.h"

struct MapFloatingLayerTile
{
	bool valid;
	std::shared_ptr<TileSet> tileSet;
	uint16_t index;
};

class MapFloatingLayer
{
	std::shared_ptr<MapLayer> m_mapLayer;
	int m_x, m_y, m_width, m_height;
	MapFloatingLayerTile* m_tiles;

public:
	MapFloatingLayer(std::shared_ptr<MapLayer> layer, int x, int y, int width, int height);
	MapFloatingLayer(const MapFloatingLayer& other);
	~MapFloatingLayer();

	std::shared_ptr<MapLayer> GetMapLayer() const { return m_mapLayer; }

	int GetX() const { return m_x; }
	int GetY() const { return m_y; }
	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }
	void Move(int x, int y) { m_x = x; m_y = y; }

	MapFloatingLayerTile GetTile(int x, int y);
	void ClearTile(int x, int y);
	void SetTile(int x, int y, std::shared_ptr<TileSet> tileSet, uint16_t index);
	void SetTile(int x, int y, const MapFloatingLayerTile& tile);

	bool UsesTileSet(std::shared_ptr<TileSet> tileSet);
};
