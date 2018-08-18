#pragma once

#include <QString>
#include <memory>
#include <map>
#include <string>
#include "palette.h"
#include "tileset.h"
#include "map.h"
#include "json/json.h"

class Project
{
	std::map<std::string, std::shared_ptr<Palette>> m_palettes;
	std::map<std::string, std::shared_ptr<TileSet>> m_tileSets;
	std::map<std::string, std::shared_ptr<MapLayer>> m_effectLayers;
	std::map<std::string, std::shared_ptr<Map>> m_maps;

	std::map<std::string, std::shared_ptr<Palette>> m_palettesById;
	std::map<std::string, std::shared_ptr<TileSet>> m_tileSetsById;
	std::map<std::string, std::shared_ptr<MapLayer>> m_effectLayersById;
	std::map<std::string, std::shared_ptr<Map>> m_mapsById;

	QString GetFileName(const std::string& name, const std::string& id, const QString& ext);
	bool SaveProjectFile(const QString& path, const QString& name, const Json::Value& value);
	static bool ReadProjectFile(const QString& path, const QString& name, Json::Value& result);

public:
	Project();

	const std::map<std::string, std::shared_ptr<Palette>>& GetPalettes() const { return m_palettes; }
	std::shared_ptr<Palette> GetPaletteByName(const std::string& name);
	bool AddPalette(std::shared_ptr<Palette> palette);
	bool RenamePalette(std::shared_ptr<Palette> palette, const std::string& name);
	void DeletePalette(std::shared_ptr<Palette> palette);

	const std::map<std::string, std::shared_ptr<TileSet>>& GetTileSets() const { return m_tileSets; }
	std::shared_ptr<TileSet> GetTileSetByName(const std::string& name);
	bool AddTileSet(std::shared_ptr<TileSet> tileSet);
	bool RenameTileSet(std::shared_ptr<TileSet> tileSet, const std::string& name);
	void DeleteTileSet(std::shared_ptr<TileSet> tileSet);

	const std::map<std::string, std::shared_ptr<MapLayer>>& GetEffectLayers() const { return m_effectLayers; }
	std::shared_ptr<MapLayer> GetEffectLayerByName(const std::string& name);
	bool AddEffectLayer(std::shared_ptr<MapLayer> layer);
	bool RenameEffectLayer(std::shared_ptr<MapLayer> layer, const std::string& name);
	void DeleteEffectLayer(std::shared_ptr<MapLayer> layer);

	const std::map<std::string, std::shared_ptr<Map>>& GetMaps() const { return m_maps; }
	std::shared_ptr<Map> GetMapByName(const std::string& name);
	bool AddMap(std::shared_ptr<Map> map);
	bool RenameMap(std::shared_ptr<Map> map, const std::string& name);
	void DeleteMap(std::shared_ptr<Map> map);

	std::vector<std::shared_ptr<TileSet>> GetTileSetsUsingPalette(std::shared_ptr<Palette> palette);
	std::vector<std::shared_ptr<MapLayer>> GetEffectLayersUsingTileSet(std::shared_ptr<TileSet> tileSet);
	std::vector<std::shared_ptr<Map>> GetMapsUsingTileSet(std::shared_ptr<TileSet> tileSet);
	std::vector<std::shared_ptr<Map>> GetMapsUsingEffectLayer(std::shared_ptr<MapLayer> layer);

	bool Save(const QString& path);
	static std::shared_ptr<Project> Open(const QString& path);

	std::shared_ptr<Palette> GetPaletteById(const std::string& id);
	std::shared_ptr<TileSet> GetTileSetById(const std::string& id);
	std::shared_ptr<MapLayer> GetEffectLayerById(const std::string& id);
	std::shared_ptr<Map> GetMapById(const std::string& id);
};
