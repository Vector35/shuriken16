#include <QUuid>
#include <set>
#include <map>
#include <memory>
#include "map.h"
#include "project.h"

using namespace std;


Map::Map()
{
	m_id = QUuid::createUuid().toString().toStdString();
	m_backgroundColor = 0;
}


Map::Map(size_t width, size_t height, size_t tileWidth, size_t tileHeight, size_t tileDepth)
{
	m_id = QUuid::createUuid().toString().toStdString();
	m_mainLayer = make_shared<MapLayer>(width, height, tileWidth, tileHeight, tileDepth);
	m_mainLayer->SetName("Main");
	m_layers.push_back(m_mainLayer);
	m_backgroundColor = 0;
}


Map::Map(const Map& other)
{
	m_id = QUuid::createUuid().toString().toStdString();
	m_name = other.m_name;
	m_backgroundColor = other.m_backgroundColor;
	for (auto& i : other.m_layers)
	{
		shared_ptr<MapLayer> layer = make_shared<MapLayer>(*i);
		m_layers.push_back(layer);
		if (i == other.m_mainLayer)
			m_mainLayer = layer;
	}
	if (!m_mainLayer)
		m_mainLayer = make_shared<MapLayer>(*other.m_mainLayer);
}


void Map::SetMainLayerSize(size_t width, size_t height)
{
	m_mainLayer->SetSize(width, height);

	size_t pixelWidth = width * m_mainLayer->GetTileWidth();
	size_t pixelHeight = height * m_mainLayer->GetTileHeight();

	// Resize non-effect layers to have the same size as the main layer
	for (auto& i : m_layers)
	{
		if (i->IsEffectLayer())
			continue;

		size_t layerWidth = (pixelWidth + (i->GetTileWidth() - 1)) / i->GetTileWidth();
		size_t layerHeight = (pixelHeight + (i->GetTileHeight() - 1)) / i->GetTileHeight();
		i->SetSize(layerWidth, layerHeight);
	}
}


void Map::InsertLayer(size_t i, shared_ptr<MapLayer> layer)
{
	if (i < m_layers.size())
		m_layers.insert(m_layers.begin() + i, layer);
	else
		m_layers.push_back(layer);
}


void Map::DeleteLayer(size_t i)
{
	if (i < m_layers.size())
		m_layers.erase(m_layers.begin() + i);
}


void Map::SwapLayers(size_t i, size_t j)
{
	if (i >= m_layers.size())
		return;
	if (j >= m_layers.size())
		return;
	shared_ptr<MapLayer> layer = m_layers[i];
	m_layers[i] = m_layers[j];
	m_layers[j] = layer;
}


bool Map::UsesTileSet(std::shared_ptr<TileSet> tileSet)
{
	for (auto& i : m_layers)
	{
		if (i->UsesTileSet(tileSet))
			return true;
	}
	return false;
}


bool Map::UsesEffectLayer(shared_ptr<MapLayer> layer)
{
	for (auto& i : m_layers)
	{
		if (i == layer)
			return true;
	}
	return false;
}


Json::Value Map::Serialize()
{
	Json::Value map(Json::objectValue);
	map["name"] = m_name;
	map["id"] = m_id;
	map["background_color"] = m_backgroundColor;

	Json::Value layers(Json::arrayValue);
	int mainLayer = -1;
	int i = 0;
	for (auto& j : m_layers)
	{
		if (j->IsEffectLayer())
		{
			Json::Value layer(Json::objectValue);
			layer["effect"] = j->GetId();
			layers.append(layer);
		}
		else
		{
			Json::Value layer(Json::objectValue);
			layer["normal"] = j->Serialize();
			layers.append(layer);
		}
		if (j == m_mainLayer)
			mainLayer = i;
		i++;
	}
	map["layers"] = layers;
	map["main_layer"] = mainLayer;

	return map;
}


shared_ptr<Map> Map::Deserialize(shared_ptr<Project> project, const Json::Value& data)
{
	shared_ptr<Map> result = make_shared<Map>();
	result->m_name = data["name"].asString();
	result->m_id = data["id"].asString();
	result->m_backgroundColor = (uint16_t)data["background_color"].asUInt();

	int i = 0;
	int mainLayer = data["main_layer"].asInt();
	for (auto& j : data["layers"])
	{
		shared_ptr<MapLayer> layer;
		if (j.isMember("effect"))
			layer = project->GetEffectLayerById(j["effect"].asString());
		else
			layer = MapLayer::Deserialize(project, j["normal"]);
		if (!layer)
			return shared_ptr<Map>();
		result->m_layers.push_back(layer);
		if (i == mainLayer)
			result->m_mainLayer = layer;
		i++;
	}

	if (!result->m_mainLayer)
		return shared_ptr<Map>();

	return result;
}
