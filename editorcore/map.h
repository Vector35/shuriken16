#pragma once

#include <string>
#include <vector>
#include <memory>
#include "tileset.h"
#include "maplayer.h"
#include "json/json.h"

class Project;
class Actor;

class Map
{
	std::string m_name;
	std::vector<std::shared_ptr<MapLayer>> m_layers;
	std::shared_ptr<MapLayer> m_mainLayer;
	uint16_t m_backgroundColor;
	std::vector<std::shared_ptr<Actor>> m_actors;
	std::string m_id;

public:
	Map();
	Map(size_t width, size_t height, size_t tileWidth, size_t tileHeight, size_t tileDepth);
	Map(const Map& other);

	const std::string& GetName() const { return m_name; }
	void SetName(const std::string& name) { m_name = name; }

	void SetMainLayerSize(size_t width, size_t height);

	const std::vector<std::shared_ptr<MapLayer>>& GetLayers() const { return m_layers; }
	std::shared_ptr<MapLayer> GetMainLayer() const { return m_mainLayer; }
	void SetMainLayer(std::shared_ptr<MapLayer> layer) { m_mainLayer = layer; }

	void InsertLayer(size_t i, std::shared_ptr<MapLayer> layer);
	void DeleteLayer(size_t i);
	void SwapLayers(size_t i, size_t j);

	uint16_t GetBackgroundColor() const { return m_backgroundColor; }
	void SetBackgroundColor(uint16_t color) { m_backgroundColor = color; }

	const std::vector<std::shared_ptr<Actor>>& GetActors() const { return m_actors; }
	void AddActor(std::shared_ptr<Actor> actor);
	void InsertActor(size_t i, std::shared_ptr<Actor> actor);
	size_t RemoveActor(std::shared_ptr<Actor> actor);

	bool UsesTileSet(std::shared_ptr<TileSet> tileSet);
	bool UsesEffectLayer(std::shared_ptr<MapLayer> layer);

	const std::string& GetId() const { return m_id; }
	Json::Value Serialize();
	static std::shared_ptr<Map> Deserialize(std::shared_ptr<Project> project, const Json::Value& data);
};
