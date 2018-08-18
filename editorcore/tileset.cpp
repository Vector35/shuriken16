#include <QUuid>
#include "tileset.h"
#include "project.h"

using namespace std;


TileSet::TileSet(size_t width, size_t height, size_t depth, SmartTileSetType smartTileSetType)
{
	m_id = QUuid::createUuid().toString().toStdString();
	m_width = width;
	m_height = height;
	m_depth = depth;
	m_smartTileSetType = smartTileSetType;

	m_displayCols = GetDisplayColumnsForSmartTileSet(smartTileSetType);

	if (m_displayCols == 0)
	{
		if (m_width >= 128)
			m_displayCols = 1;
		else
			m_displayCols = 128 / m_width;
	}

	if (smartTileSetType != NormalTileSet)
		SetTileCount(GetTileCountForSmartTileSet(smartTileSetType));
}


TileSet::TileSet(const TileSet& other)
{
	m_id = QUuid::createUuid().toString().toStdString();
	m_name = other.m_name;
	m_width = other.m_width;
	m_height = other.m_height;
	m_depth = other.m_depth;
	m_smartTileSetType = other.m_smartTileSetType;
	m_displayCols = other.m_displayCols;

	for (auto& i : other.m_tiles)
	{
		if (i)
			m_tiles.push_back(make_shared<Tile>(*i));
		else
			m_tiles.emplace_back();
	}

	if (other.m_animation)
		m_animation = make_shared<Animation>(*other.m_animation);
}


size_t TileSet::GetFrameCount() const
{
	if (m_animation)
		return m_animation->GetFrameCount();
	return 1;
}


shared_ptr<Tile> TileSet::GetTile(size_t i)
{
	if (i >= m_tiles.size())
		return shared_ptr<Tile>();
	return m_tiles[i];
}


void TileSet::SetTile(size_t i, shared_ptr<Tile> tile)
{
	if (i >= m_tiles.size())
		return;
	m_tiles[i] = tile;
}


void TileSet::SetTileCount(size_t count)
{
	size_t oldCount = m_tiles.size();
	m_tiles.resize(count);
	for (size_t i = oldCount; i < count; i++)
		SetTile(i, CreateTile());
}


shared_ptr<Tile> TileSet::CreateTile()
{
	return make_shared<Tile>(m_width, m_height, m_depth);
}


void TileSet::SetAnimation(shared_ptr<Animation> anim)
{
	m_animation = anim;
	for (auto& i : m_tiles)
		i->SetFrameCount((uint16_t)GetFrameCount());
}


uint16_t TileSet::GetFrameForTime(uint32_t ticks)
{
	if (m_animation)
	{
		ticks %= (uint32_t)m_animation->GetTotalLength();
		return m_animation->GetFrameForTime((uint16_t)ticks);
	}
	return 0;
}


void TileSet::CopyFrame(uint16_t from, uint16_t to)
{
	for (auto& i : m_tiles)
		i->CopyFrame(from, to);
}


void TileSet::SwapFrames(uint16_t from, uint16_t to)
{
	if (!m_animation)
		return;
	for (auto& i : m_tiles)
		i->SwapFrames(from, to);
	uint16_t tempFrameLen = m_animation->GetFrameLength(to);
	m_animation->SetFrameLength(to, m_animation->GetFrameLength(from));
	m_animation->SetFrameLength(from, tempFrameLen);
}


void TileSet::DuplicateFrame(uint16_t frame)
{
	if (!m_animation)
		return;
	for (auto& i : m_tiles)
		i->DuplicateFrame(frame);
	uint16_t len = m_animation->GetFrameLength(frame);
	m_animation->InsertFrame(frame, len);
}


void TileSet::RemoveFrame(uint16_t frame)
{
	if (!m_animation)
		return;
	if ((frame >= m_animation->GetFrameCount()) || (m_animation->GetFrameCount() == 1))
		return;
	for (auto& i : m_tiles)
		i->RemoveFrame(frame);
	m_animation->RemoveFrame(frame);
}


vector<shared_ptr<Tile>> TileSet::GetTilesForSingleFrame(uint16_t frame)
{
	vector<shared_ptr<Tile>> result;
	for (auto& i : m_tiles)
		result.push_back(i->GetTileForSingleFrame(frame));
	return result;
}


void TileSet::InsertFrameFromTiles(uint16_t frame, const vector<shared_ptr<Tile>>& tiles, uint16_t length)
{
	if (m_tiles.size() != tiles.size())
		return;
	for (size_t i = 0; i < tiles.size(); i++)
		m_tiles[i]->InsertFrameFromTile(frame, tiles[i]);
	m_animation->InsertFrame(frame, length);
}


bool TileSet::UsesPalette(shared_ptr<Palette> palette)
{
	for (auto& i : m_tiles)
	{
		if (i->GetPalette() == palette)
			return true;
	}
	return false;
}


Json::Value TileSet::Serialize()
{
	Json::Value tileSet(Json::objectValue);
	tileSet["name"] = m_name;
	tileSet["id"] = m_id;
	tileSet["width"] = (uint64_t)m_width;
	tileSet["height"] = (uint64_t)m_height;
	tileSet["depth"] = (uint64_t)m_depth;
	tileSet["display_cols"] = (uint64_t)m_displayCols;

	Json::Value tiles(Json::arrayValue);
	for (auto& i : m_tiles)
	{
		if (i)
			tiles.append(i->Serialize());
		else
			tiles.append(CreateTile()->Serialize());
	}
	tileSet["tiles"] = tiles;

	if (m_animation)
		tileSet["anim"] = m_animation->Serialize();

	switch (m_smartTileSetType)
	{
	case SimplifiedDoubleWidthSmartTileSet:
		tileSet["smart"] = "simplified_double_width";
	default:
		break;
	}

	return tileSet;
}


shared_ptr<TileSet> TileSet::Deserialize(shared_ptr<Project> project, const Json::Value& data)
{
	size_t width = (size_t)data["width"].asUInt64();
	size_t height = (size_t)data["height"].asUInt64();
	size_t depth = (size_t)data["depth"].asUInt64();

	SmartTileSetType smartTileSetType = NormalTileSet;
	if (data.isMember("smart"))
	{
		if (data["smart"].asString() == "simplified_double_width")
			smartTileSetType = SimplifiedDoubleWidthSmartTileSet;
	}

	shared_ptr<TileSet> result = make_shared<TileSet>(width, height, depth, smartTileSetType);
	result->m_name = data["name"].asString();
	result->m_id = data["id"].asString();

	if (data.isMember("display_cols"))
		result->m_displayCols = data["display_cols"].asUInt64();

	size_t frames = 1;
	if (data.isMember("anim"))
	{
		result->m_animation = Animation::Deserialize(data["anim"]);
		if (!result->m_animation)
			return shared_ptr<TileSet>();
		frames = result->m_animation->GetFrameCount();
	}

	result->m_tiles.clear();
	for (auto& i : data["tiles"])
		result->m_tiles.push_back(Tile::Deserialize(project, i, width, height, depth, frames));

	return result;
}


size_t TileSet::GetTileCountForSmartTileSet(SmartTileSetType type)
{
	switch (type)
	{
	case SimplifiedDoubleWidthSmartTileSet:
		return 41;
	default:
		return 0;
	}
}


size_t TileSet::GetDisplayColumnsForSmartTileSet(SmartTileSetType type)
{
	switch (type)
	{
	case SimplifiedDoubleWidthSmartTileSet:
		return 9;
	default:
		return 0;
	}
}


size_t TileSet::GetDefaultTileForSmartTileSet(SmartTileSetType type)
{
	switch (type)
	{
	case SimplifiedDoubleWidthSmartTileSet:
		return (2 * 9) + 2;
	default:
		return 0;
	}
}
