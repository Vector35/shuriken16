#include <QUuid>
#include <set>
#include <map>
#include <memory>
#include "maplayer.h"
#include "project.h"

using namespace std;


bool MapLayer::SmartTileContext::Empty(int x, int y)
{
	if ((x < -2) || (x > 2) || (y < -2) || (y > 2))
		return false;
	return !tile[x + 2][y + 2];
}


bool MapLayer::SmartTileContext::Filled(int x, int y)
{
	if ((x < -2) || (x > 2) || (y < -2) || (y > 2))
		return false;
	return tile[x + 2][y + 2];
}


MapLayer::MapLayer(size_t width, size_t height, size_t tileWidth, size_t tileHeight, size_t tileDepth,
	bool effectLayer)
{
	m_id = QUuid::createUuid().toString().toStdString();

	m_width = width;
	m_height = height;
	m_tileWidth = tileWidth;
	m_tileHeight = tileHeight;
	m_tileDepth = tileDepth;
	m_tiles.resize(m_width * m_height);

	m_effectLayer = effectLayer;
	m_blendMode = BlendMode_Normal;
	m_alpha = 0;
	m_parallaxFactorX = m_parallaxFactorY = 0x100;
	m_autoScrollX = m_autoScrollY = 0;
}


MapLayer::MapLayer(const MapLayer& other)
{
	m_id = QUuid::createUuid().toString().toStdString();
	m_name = other.m_name;
	m_width = other.m_width;
	m_height = other.m_height;
	m_tileWidth = other.m_tileWidth;
	m_tileHeight = other.m_tileHeight;
	m_tileDepth = other.m_tileDepth;
	m_tiles = other.m_tiles;
	m_effectLayer = other.m_effectLayer;
	m_blendMode = other.m_blendMode;
	m_alpha = other.m_alpha;
	m_parallaxFactorX = other.m_parallaxFactorX;
	m_parallaxFactorY = other.m_parallaxFactorY;
	m_autoScrollX = other.m_autoScrollX;
	m_autoScrollY = other.m_autoScrollY;
}


void MapLayer::SetSize(size_t width, size_t height)
{
	vector<TileReference> newTiles;
	newTiles.resize(width * height);

	size_t copyWidth = width;
	size_t copyHeight = height;
	if (m_width < copyWidth)
		copyWidth = m_width;
	if (m_height < copyHeight)
		copyHeight = m_height;

	for (size_t y = 0; y < copyHeight; y++)
		for (size_t x = 0; x < copyWidth; x++)
			newTiles[(y * width) + x] = m_tiles[(y * m_width) + x];

	m_tiles = newTiles;
	m_width = width;
	m_height = height;
}


TileReference MapLayer::GetTileAt(size_t x, size_t y)
{
	if (x >= m_width)
		return TileReference();
	if (y >= m_height)
		return TileReference();
	return m_tiles[(y * m_width) + x];
}


void MapLayer::SetTileAt(size_t x, size_t y, const TileReference& tile)
{
	if (x >= m_width)
		return;
	if (y >= m_height)
		return;
	m_tiles[(y * m_width) + x] = tile;
}


void MapLayer::UpdateSmartTile(size_t x, size_t y)
{
	if (x >= m_width)
		return;
	if (y >= m_height)
		return;

	TileReference ref = GetTileAt(x, y);
	if (!ref.tileSet)
		return;
	if (!ref.tileSet->IsSmartTileSet())
		return;

	switch (ref.tileSet->GetSmartTileSetType())
	{
	case SimplifiedSingleWidthSmartTileSet:
		UpdateSimplifiedSingleWidthSmartTileSet(x, y, ref.tileSet);
		break;
	case SimplifiedDoubleWidthSmartTileSet:
		UpdateSimplifiedDoubleWidthSmartTileSet(x, y, ref.tileSet);
		break;
	default:
		break;
	}
}


bool MapLayer::IsCompatibleForSmartTiles(size_t x, size_t y, const shared_ptr<TileSet>& tileSet)
{
	if (x >= m_width)
		return true;
	if (y >= m_height)
		return true;
	TileReference ref = GetTileAt(x, y);
	if (!ref.tileSet)
		return false;
	return tileSet->IsCompatibleForSmartTiles(ref.tileSet);
}


MapLayer::SmartTileContext MapLayer::GetContextForSmartTile(size_t x, size_t y, const std::shared_ptr<TileSet>& tileSet)
{
	SmartTileContext result;
	for (int j = -2; j <= 2; j++)
		for (int i = -2; i <= 2; i++)
			result.tile[i + 2][j + 2] = IsCompatibleForSmartTiles(x + i, y + j, tileSet);
	return result;
}


void MapLayer::UpdateSimplifiedSingleWidthSmartTileSet(size_t x, size_t y, const shared_ptr<TileSet>& tileSet)
{
	SmartTileContext c = GetContextForSmartTile(x, y, tileSet);

#define TILE_INDEX(x, y) (((y) * 5) + (x))
	if (c.Filled(-1, 0) && c.Filled(1, 0) && c.Filled(0, -1) && c.Empty(0, 1))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(1, 2)));
	else if (c.Filled(-1, 0) && c.Filled(1, 0) && c.Empty(0, -1) && c.Filled(0, 1))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(1, 0)));
	else if (c.Filled(-1, 0) && c.Empty(1, 0) && c.Filled(0, -1) && c.Filled(0, 1))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(2, 1)));
	else if (c.Empty(-1, 0) && c.Filled(1, 0) && c.Filled(0, -1) && c.Filled(0, 1))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(0, 1)));
	else if (c.Filled(-1, 0) && c.Empty(1, 0) && c.Filled(0, -1) && c.Empty(0, 1))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(2, 2)));
	else if (c.Filled(-1, 0) && c.Empty(1, 0) && c.Empty(0, -1) && c.Filled(0, 1))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(2, 0)));
	else if (c.Empty(-1, 0) && c.Filled(1, 0) && c.Filled(0, -1) && c.Empty(0, 1))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(0, 2)));
	else if (c.Empty(-1, 0) && c.Filled(1, 0) && c.Empty(0, -1) && c.Filled(0, 1))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(0, 0)));
	else if (c.Empty(-1, -1))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(3, 0)));
	else if (c.Empty(1, -1))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(4, 0)));
	else if (c.Empty(-1, 1))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(3, 1)));
	else if (c.Empty(1, 1))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(4, 1)));
	else
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(1, 1)));
#undef TILE_INDEX
}


void MapLayer::UpdateSimplifiedDoubleWidthSmartTileSet(size_t x, size_t y, const shared_ptr<TileSet>& tileSet)
{
	SmartTileContext c = GetContextForSmartTile(x, y, tileSet);

#define TILE_INDEX(x, y) (((y) * 9) + (x))
	// Outer upper left corner
	if (c.Filled(1, 0) && c.Filled(1, 1) && c.Filled(1, 2) && c.Filled(2, 1) && c.Empty(2, 2))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(0, 0)));
	else if (c.Filled(0, 1) && c.Filled(0, 2) && c.Filled(1, 0) && c.Filled(1, 1) && c.Empty(1, 2))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(1, 0)));
	else if (c.Filled(1, 0) && c.Filled(1, 1) && c.Filled(2, 0) && c.Empty(2, 1))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(0, 1)));
	else if (c.Filled(0, 1) && c.Filled(1, 0) && c.Empty(1, 1))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(1, 1)));

	// Top
	else if (c.Filled(0, 1) && c.Filled(-2, 0) && c.Filled(-1, 0) && c.Filled(1, 0) && c.Filled(2, 0) && c.Empty(0, 2))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(2, 0)));
	else if (c.Filled(-2, 0) && c.Filled(-1, 0) && c.Filled(1, 0) && c.Filled(2, 0) && c.Empty(0, 1))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(2, 1)));

	// Outer upper right corner
	else if (c.Filled(0, 1) && c.Filled(0, 2) && c.Filled(-1, 0) && c.Filled(-1, 1) && c.Empty(-1, 2))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(3, 0)));
	else if (c.Filled(-1, 0) && c.Filled(-1, 1) && c.Filled(-1, 2) && c.Filled(-2, 1) && c.Empty(-2, 2))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(4, 0)));
	else if (c.Filled(0, 1) && c.Filled(-1, 0) && c.Empty(-1, 1))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(3, 1)));
	else if (c.Filled(-1, 0) && c.Filled(-1, 1) && c.Filled(-2, 0) && c.Empty(-2, 1))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(4, 1)));

	// Left
	else if (c.Filled(1, 0) && c.Filled(0, -2) && c.Filled(0, -1) && c.Filled(0, 1) && c.Filled(0, 2) && c.Empty(2, 0))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(0, 2)));
	else if (c.Filled(0, -2) && c.Filled(0, -1) && c.Filled(0, 1) && c.Filled(0, 2) && c.Empty(1, 0))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(1, 2)));

	// Right
	else if (c.Filled(0, -2) && c.Filled(0, -1) && c.Filled(0, 1) && c.Filled(0, 2) && c.Empty(-1, 0))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(3, 2)));
	else if (c.Filled(-1, 0) && c.Filled(0, -2) && c.Filled(0, -1) && c.Filled(0, 1) && c.Filled(0, 2) && c.Empty(-2, 0))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(4, 2)));

	// Outer bottom left corner
	else if (c.Filled(1, 0) && c.Filled(1, -1) && c.Filled(2, 0) && c.Empty(2, -1))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(0, 3)));
	else if (c.Filled(0, -1) && c.Filled(1, 0) && c.Empty(1, -1))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(1, 3)));
	else if (c.Filled(1, 0) && c.Filled(1, -1) && c.Filled(1, -2) && c.Filled(2, -1) && c.Empty(2, -2))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(0, 4)));
	else if (c.Filled(0, -1) && c.Filled(0, -2) && c.Filled(1, 0) && c.Filled(1, -1) && c.Empty(1, -2))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(1, 4)));

	// Bottom
	else if (c.Filled(-2, 0) && c.Filled(-1, 0) && c.Filled(1, 0) && c.Filled(2, 0) && c.Empty(0, -1))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(2, 3)));
	else if (c.Filled(0, -1) && c.Filled(-2, 0) && c.Filled(-1, 0) && c.Filled(1, 0) && c.Filled(2, 0) && c.Empty(0, -2))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(2, 4)));

	// Outer bottom right corner
	else if (c.Filled(0, -1) && c.Filled(-1, 0) && c.Empty(-1, -1))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(3, 3)));
	else if (c.Filled(-1, 0) && c.Filled(-1, -1) && c.Filled(-2, 0) && c.Empty(-2, -1))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(4, 3)));
	else if (c.Filled(0, -1) && c.Filled(0, -2) && c.Filled(-1, 0) && c.Filled(-1, -1) && c.Empty(-1, -2))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(3, 4)));
	else if (c.Filled(-1, 0) && c.Filled(-1, -1) && c.Filled(-1, -2) && c.Filled(-2, -1) && c.Empty(-2, -2))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(4, 4)));

	// Inner upper left corner
	else if (c.Filled(1, 0) && c.Filled(1, 1) && c.Filled(0, 1) && c.Empty(2, 0) && c.Empty(0, 2))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(5, 0)));
	else if (c.Filled(-1, 0) && c.Filled(-1, 1) && c.Filled(0, 1) && c.Empty(1, 0) && c.Empty(0, 2))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(6, 0)));
	else if (c.Filled(0, -1) && c.Filled(1, -1) && c.Filled(1, 0) && c.Empty(2, 0) && c.Empty(0, 1))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(5, 1)));
	else if (c.Filled(-1, -1) && c.Filled(-1, 0) && c.Filled(0, -1) && c.Empty(1, 0) && c.Empty(0, 1))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(6, 1)));

	// Inner upper right corner
	else if (c.Filled(1, 0) && c.Filled(1, 1) && c.Filled(0, 1) && c.Empty(-1, 0) && c.Empty(0, 2))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(7, 0)));
	else if (c.Filled(-1, 0) && c.Filled(-1, 1) && c.Filled(0, 1) && c.Empty(-2, 0) && c.Empty(0, 2))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(8, 0)));
	else if (c.Filled(1, -1) && c.Filled(1, 0) && c.Filled(0, -1) && c.Empty(-1, 0) && c.Empty(0, 1))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(7, 1)));
	else if (c.Filled(0, -1) && c.Filled(-1, -1) && c.Filled(-1, 0) && c.Empty(-2, 0) && c.Empty(0, 1))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(8, 1)));

	// Inner bottom left corner
	else if (c.Filled(0, 1) && c.Filled(1, 1) && c.Filled(1, 0) && c.Empty(2, 0) && c.Empty(0, -1))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(5, 2)));
	else if (c.Filled(-1, 1) && c.Filled(-1, 0) && c.Filled(0, 1) && c.Empty(1, 0) && c.Empty(0, -1))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(6, 2)));
	else if (c.Filled(1, 0) && c.Filled(1, -1) && c.Filled(0, -1) && c.Empty(2, 0) && c.Empty(0, -2))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(5, 3)));
	else if (c.Filled(-1, 0) && c.Filled(-1, -1) && c.Filled(0, -1) && c.Empty(1, 0) && c.Empty(0, -2))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(6, 3)));

	// Inner bottom right corner
	else if (c.Filled(1, 1) && c.Filled(1, 0) && c.Filled(0, 1) && c.Empty(-1, 0) && c.Empty(0, -1))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(7, 2)));
	else if (c.Filled(0, 1) && c.Filled(-1, 1) && c.Filled(-1, 0) && c.Empty(-2, 0) && c.Empty(0, -1))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(8, 2)));
	else if (c.Filled(1, 0) && c.Filled(1, -1) && c.Filled(0, -1) && c.Empty(-1, 0) && c.Empty(0, -2))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(7, 3)));
	else if (c.Filled(-1, 0) && c.Filled(-1, -1) && c.Filled(0, -1) && c.Empty(-2, 0) && c.Empty(0, -2))
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(8, 3)));

	else
		SetTileAt(x, y, TileReference(tileSet, TILE_INDEX(2, 2)));
#undef TILE_INDEX
}


void MapLayer::UpdateRegionForSmartTiles(int x, int y, int w, int h)
{
	if (x >= 2)
	{
		x -= 2;
		w += 4;
	}
	else
	{
		w += x + 2;
		x = 0;
	}
	if (y >= 2)
	{
		y -= 2;
		h += 4;
	}
	else
	{
		h += y + 2;
		y = 0;
	}

	if (x >= (int)m_width)
		return;
	if (y >= (int)m_height)
		return;
	if (w <= 0)
		return;
	if (h <= 0)
		return;

	if ((x + w) > (int)m_width)
		w = m_width - x;
	if ((y + h) > (int)m_height)
		h = m_height - y;

	if (w <= 0)
		return;
	if (h <= 0)
		return;

	for (int j = 0; j < h; j++)
		for (int i = 0; i < w; i++)
			UpdateSmartTile((size_t)(x + i), (size_t)(y + j));
}


bool MapLayer::UsesTileSet(std::shared_ptr<TileSet> tileSet)
{
	for (auto& i : m_tiles)
	{
		if (i.tileSet == tileSet)
			return true;
	}
	return false;
}


Json::Value MapLayer::Serialize()
{
	Json::Value map(Json::objectValue);
	map["name"] = m_name;
	map["id"] = m_id;
	map["width"] = (uint64_t)m_width;
	map["height"] = (uint64_t)m_height;
	map["tile_width"] = (uint64_t)m_tileWidth;
	map["tile_height"] = (uint64_t)m_tileHeight;
	map["tile_depth"] = (uint64_t)m_tileDepth;

	set<shared_ptr<TileSet>> usedTileSets;
	for (auto& i : m_tiles)
	{
		if (i.tileSet)
			usedTileSets.insert(i.tileSet);
	}
	vector<shared_ptr<TileSet>> sortedUsedTileSets;
	for (auto& i : usedTileSets)
		sortedUsedTileSets.push_back(i);
	sort(sortedUsedTileSets.begin(), sortedUsedTileSets.end(),
		[&](const shared_ptr<TileSet>& a, const shared_ptr<TileSet>& b) {
			return a->GetId() < b->GetId();
		});

	Json::Value tileSets(Json::arrayValue);
	std::map<shared_ptr<TileSet>, uint64_t> tileSetIds;
	uint64_t i = 0;
	for (auto& j : sortedUsedTileSets)
	{
		tileSets.append(j->GetId());
		tileSetIds[j] = i++;
	}
	map["tile_sets"] = tileSets;

	Json::Value tiles(Json::arrayValue);
	for (size_t y = 0; y < m_height; y++)
	{
		Json::Value row(Json::arrayValue);
		for (size_t x = 0; x < m_width; x++)
		{
			Json::Value tile(Json::arrayValue);
			TileReference ref = GetTileAt(x, y);
			if (ref.tileSet)
			{
				tile.append(tileSetIds[ref.tileSet]);
				tile.append(ref.index);
			}
			row.append(tile);
		}
		Json::FastWriter writer;
		string rowStr = writer.write(row);
		tiles.append(rowStr);
	}
	map["tiles"] = tiles;

	map["effect"] = m_effectLayer;
	map["blend"] = (uint32_t)m_blendMode;
	map["alpha"] = m_alpha;
	map["parallax_x"] = m_parallaxFactorX;
	map["parallax_y"] = m_parallaxFactorY;
	map["auto_scroll_x"] = m_autoScrollX;
	map["auto_scroll_y"] = m_autoScrollY;
	return map;
}


shared_ptr<MapLayer> MapLayer::Deserialize(shared_ptr<Project> project, const Json::Value& data)
{
	size_t width = (size_t)data["width"].asUInt64();
	size_t height = (size_t)data["height"].asUInt64();
	size_t tileWidth = (size_t)data["tile_width"].asUInt64();
	size_t tileHeight = (size_t)data["tile_height"].asUInt64();
	size_t tileDepth = (size_t)data["tile_depth"].asUInt64();
	bool effect = data["effect"].asBool();

	shared_ptr<MapLayer> result = make_shared<MapLayer>(width, height,
		tileWidth, tileHeight, tileDepth, effect);
	result->m_name = data["name"].asString();
	result->m_id = data["id"].asString();
	result->m_blendMode = (BlendMode)data["blend"].asUInt();
	result->m_alpha = (uint8_t)data["alpha"].asUInt();
	result->m_parallaxFactorX = (int16_t)data["parallax_x"].asInt();
	result->m_parallaxFactorY = (int16_t)data["parallax_y"].asInt();
	result->m_autoScrollX = (int16_t)data["auto_scroll_x"].asInt();
	result->m_autoScrollY = (int16_t)data["auto_scroll_y"].asInt();

	vector<shared_ptr<TileSet>> tileSets;
	for (auto& i : data["tile_sets"])
		tileSets.push_back(project->GetTileSetById(i.asString()));

	size_t y = 0;
	for (auto& rowStr : data["tiles"])
	{
		if (y >= height)
			return shared_ptr<MapLayer>();

		Json::Reader reader;
		Json::Value row;
		if (!reader.parse(rowStr.asString(), row, false))
			return shared_ptr<MapLayer>();

		size_t x = 0;
		for (auto& col : row)
		{
			TileReference ref;
			if ((col.size() == 2) && (col[0].asUInt64() < tileSets.size()))
			{
				ref.tileSet = tileSets[(size_t)col[0].asUInt64()];
				ref.index = (uint16_t)col[1].asUInt();
			}
			result->SetTileAt(x, y, ref);
			x++;
		}

		y++;
	}

	return result;
}
