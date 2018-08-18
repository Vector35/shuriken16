#include "mapfloatinglayer.h"

using namespace std;


MapFloatingLayer::MapFloatingLayer(shared_ptr<MapLayer> layer, int x, int y, int width, int height)
{
	m_mapLayer = layer;
	m_x = x;
	m_y = y;
	m_width = width;
	m_height = height;
	m_tiles = new MapFloatingLayerTile[width * height];
	for (int i = 0; i < (width * height); i++)
		m_tiles[i].valid = false;
}


MapFloatingLayer::MapFloatingLayer(const MapFloatingLayer& other)
{
	m_mapLayer = other.m_mapLayer;
	m_x = other.m_x;
	m_y = other.m_y;
	m_width = other.m_width;
	m_height = other.m_height;
	m_tiles = new MapFloatingLayerTile[m_width * m_height];
	for (int i = 0; i < (m_width * m_height); i++)
		m_tiles[i] = other.m_tiles[i];
}


MapFloatingLayer::~MapFloatingLayer()
{
	delete[] m_tiles;
}


MapFloatingLayerTile MapFloatingLayer::GetTile(int x, int y)
{
	if ((x < 0) || (y < 0) || (x >= m_width) || (y >= m_height))
	{
		MapFloatingLayerTile result;
		result.valid = false;
		return result;
	}
	return m_tiles[(y * m_width) + x];
}


void MapFloatingLayer::ClearTile(int x, int y)
{
	if ((x < 0) || (y < 0) || (x >= m_width) || (y >= m_height))
		return;
	m_tiles[(y * m_width) + x].valid = false;
}


void MapFloatingLayer::SetTile(int x, int y, shared_ptr<TileSet> tileSet, uint16_t index)
{
	if ((x < 0) || (y < 0) || (x >= m_width) || (y >= m_height))
		return;
	m_tiles[(y * m_width) + x].valid = true;
	m_tiles[(y * m_width) + x].tileSet = tileSet;
	m_tiles[(y * m_width) + x].index = index;
}


void MapFloatingLayer::SetTile(int x, int y, const MapFloatingLayerTile& tile)
{
	if ((x < 0) || (y < 0) || (x >= m_width) || (y >= m_height))
		return;
	m_tiles[(y * m_width) + x] = tile;
}


bool MapFloatingLayer::UsesTileSet(shared_ptr<TileSet> tileSet)
{
	for (int i = 0; i < (m_width * m_height); i++)
	{
		if (m_tiles[i].valid && (m_tiles[i].tileSet == tileSet))
			return true;
	}
	return false;
}
