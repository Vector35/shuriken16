#include <QMessageBox>
#include <QDir>
#include <cassert>
#include <set>
#include "project.h"
#include "palette.h"

using namespace std;


Project::Project()
{
	shared_ptr<Palette> palette = make_shared<Palette>();
	palette->SetName("Default");
	palette->SetEntryCount(16);
	palette->SetEntry(0, Palette::FromRGB32(0x000000));
	palette->SetEntry(1, Palette::FromRGB32(0x56859C));
	palette->SetEntry(2, Palette::FromRGB32(0x698C71));
	palette->SetEntry(3, Palette::FromRGB32(0x609CA0));
	palette->SetEntry(4, Palette::FromRGB32(0x915E64));
	palette->SetEntry(5, Palette::FromRGB32(0x8D8087));
	palette->SetEntry(6, Palette::FromRGB32(0xC69F6C));
	palette->SetEntry(7, Palette::FromRGB32(0xD8D8D8));
	palette->SetEntry(8, Palette::FromRGB32(0x545454));
	palette->SetEntry(9, Palette::FromRGB32(0x80C6E9));
	palette->SetEntry(10, Palette::FromRGB32(0xA2D9AF));
	palette->SetEntry(11, Palette::FromRGB32(0x8EE6ED));
	palette->SetEntry(12, Palette::FromRGB32(0xDE8F97));
	palette->SetEntry(13, Palette::FromRGB32(0xDAC4D1));
	palette->SetEntry(14, Palette::FromRGB32(0xEDDFB3));
	palette->SetEntry(15, Palette::FromRGB32(0xE0E0E0));
	m_palettes[palette->GetName()] = palette;
	m_palettesById[palette->GetId()] = palette;
}


shared_ptr<Palette> Project::GetPaletteByName(const string& name)
{
	auto i = m_palettes.find(name);
	if (i == m_palettes.end())
		return shared_ptr<Palette>();
	return i->second;
}


bool Project::AddPalette(shared_ptr<Palette> palette)
{
	string name = palette->GetName();
	auto i = m_palettes.find(name);
	if (i != m_palettes.end())
		return false;
	assert(m_palettesById.find(palette->GetId()) == m_palettesById.end());
	m_palettes[name] = palette;
	m_palettesById[palette->GetId()] = palette;
	return true;
}


bool Project::RenamePalette(std::shared_ptr<Palette> palette, const string& name)
{
	if (name == palette->GetName())
		return true;
	auto i = m_palettes.find(name);
	if (i != m_palettes.end())
		return false;
	m_palettes.erase(palette->GetName());
	palette->SetName(name);
	m_palettes[name] = palette;
	return true;
}


void Project::DeletePalette(std::shared_ptr<Palette> palette)
{
	m_palettes.erase(palette->GetName());
	m_palettesById.erase(palette->GetId());
}


shared_ptr<TileSet> Project::GetTileSetByName(const string& name)
{
	auto i = m_tileSets.find(name);
	if (i == m_tileSets.end())
		return shared_ptr<TileSet>();
	return i->second;
}


bool Project::AddTileSet(shared_ptr<TileSet> tileSet)
{
	string name = tileSet->GetName();
	auto i = m_tileSets.find(name);
	if (i != m_tileSets.end())
		return false;
	assert(m_tileSetsById.find(tileSet->GetId()) == m_tileSetsById.end());
	m_tileSets[name] = tileSet;
	m_tileSetsById[tileSet->GetId()] = tileSet;
	return true;
}


bool Project::RenameTileSet(shared_ptr<TileSet> tileSet, const string& name)
{
	if (name == tileSet->GetName())
		return true;
	auto i = m_tileSets.find(name);
	if (i != m_tileSets.end())
		return false;
	m_tileSets.erase(tileSet->GetName());
	tileSet->SetName(name);
	m_tileSets[name] = tileSet;
	return true;
}


void Project::DeleteTileSet(shared_ptr<TileSet> tileSet)
{
	m_tileSets.erase(tileSet->GetName());
	m_tileSetsById.erase(tileSet->GetId());
}


shared_ptr<MapLayer> Project::GetEffectLayerByName(const string& name)
{
	auto i = m_effectLayers.find(name);
	if (i == m_effectLayers.end())
		return shared_ptr<MapLayer>();
	return i->second;
}


bool Project::AddEffectLayer(shared_ptr<MapLayer> layer)
{
	string name = layer->GetName();
	auto i = m_effectLayers.find(name);
	if (i != m_effectLayers.end())
		return false;
	assert(m_effectLayersById.find(layer->GetId()) == m_effectLayersById.end());
	m_effectLayers[name] = layer;
	m_effectLayersById[layer->GetId()] = layer;
	return true;
}


bool Project::RenameEffectLayer(shared_ptr<MapLayer> layer, const string& name)
{
	if (name == layer->GetName())
		return true;
	auto i = m_effectLayers.find(name);
	if (i != m_effectLayers.end())
		return false;
	m_effectLayers.erase(layer->GetName());
	layer->SetName(name);
	m_effectLayers[name] = layer;
	return true;
}


void Project::DeleteEffectLayer(shared_ptr<MapLayer> layer)
{
	m_effectLayers.erase(layer->GetName());
	m_effectLayersById.erase(layer->GetId());
}


shared_ptr<Map> Project::GetMapByName(const string& name)
{
	auto i = m_maps.find(name);
	if (i == m_maps.end())
		return shared_ptr<Map>();
	return i->second;
}


bool Project::AddMap(shared_ptr<Map> map)
{
	string name = map->GetName();
	auto i = m_maps.find(name);
	if (i != m_maps.end())
		return false;
	assert(m_mapsById.find(map->GetId()) == m_mapsById.end());
	m_maps[name] = map;
	m_mapsById[map->GetId()] = map;
	return true;
}


bool Project::RenameMap(shared_ptr<Map> map, const string& name)
{
	if (name == map->GetName())
		return true;
	auto i = m_maps.find(name);
	if (i != m_maps.end())
		return false;
	m_maps.erase(map->GetName());
	map->SetName(name);
	m_maps[name] = map;
	return true;
}


void Project::DeleteMap(shared_ptr<Map> map)
{
	m_maps.erase(map->GetName());
	m_mapsById.erase(map->GetId());
}


shared_ptr<Sprite> Project::GetSpriteByName(const string& name)
{
	auto i = m_sprites.find(name);
	if (i == m_sprites.end())
		return shared_ptr<Sprite>();
	return i->second;
}


bool Project::AddSprite(shared_ptr<Sprite> sprite)
{
	string name = sprite->GetName();
	auto i = m_sprites.find(name);
	if (i != m_sprites.end())
		return false;
	assert(m_spritesById.find(sprite->GetId()) == m_spritesById.end());
	m_sprites[name] = sprite;
	m_spritesById[sprite->GetId()] = sprite;
	return true;
}


bool Project::RenameSprite(shared_ptr<Sprite> sprite, const string& name)
{
	if (name == sprite->GetName())
		return true;
	auto i = m_sprites.find(name);
	if (i != m_sprites.end())
		return false;
	m_sprites.erase(sprite->GetName());
	sprite->SetName(name);
	m_sprites[name] = sprite;
	return true;
}


void Project::DeleteSprite(shared_ptr<Sprite> sprite)
{
	m_sprites.erase(sprite->GetName());
	m_spritesById.erase(sprite->GetId());
}


shared_ptr<ActorType> Project::GetActorTypeByName(const string& name)
{
	auto i = m_actorTypes.find(name);
	if (i == m_actorTypes.end())
		return shared_ptr<ActorType>();
	return i->second;
}


bool Project::AddActorType(shared_ptr<ActorType> actorType)
{
	string name = actorType->GetName();
	auto i = m_actorTypes.find(name);
	if (i != m_actorTypes.end())
		return false;
	assert(m_actorTypesById.find(actorType->GetId()) == m_actorTypesById.end());
	m_actorTypes[name] = actorType;
	m_actorTypesById[actorType->GetId()] = actorType;
	return true;
}


bool Project::RenameActorType(shared_ptr<ActorType> actorType, const string& name)
{
	if (name == actorType->GetName())
		return true;
	auto i = m_actorTypes.find(name);
	if (i != m_actorTypes.end())
		return false;
	m_actorTypes.erase(actorType->GetName());
	actorType->SetName(name);
	m_actorTypes[name] = actorType;
	return true;
}


void Project::DeleteActorType(shared_ptr<ActorType> actorType)
{
	m_actorTypes.erase(actorType->GetName());
	m_actorTypesById.erase(actorType->GetId());
}


vector<shared_ptr<TileSet>> Project::GetTileSetsUsingPalette(shared_ptr<Palette> palette)
{
	vector<shared_ptr<TileSet>> result;
	for (auto& i : m_tileSets)
	{
		if (i.second->UsesPalette(palette))
			result.push_back(i.second);
	}
	return result;
}


vector<shared_ptr<Sprite>> Project::GetSpritesUsingPalette(shared_ptr<Palette> palette)
{
	vector<shared_ptr<Sprite>> result;
	for (auto& i : m_sprites)
	{
		if (i.second->UsesPalette(palette))
			result.push_back(i.second);
	}
	return result;
}


vector<shared_ptr<MapLayer>> Project::GetEffectLayersUsingTileSet(shared_ptr<TileSet> tileSet)
{
	vector<shared_ptr<MapLayer>> result;
	for (auto& i : m_effectLayers)
	{
		if (i.second->UsesTileSet(tileSet))
			result.push_back(i.second);
	}
	return result;
}


vector<shared_ptr<Map>> Project::GetMapsUsingTileSet(shared_ptr<TileSet> tileSet)
{
	vector<shared_ptr<Map>> result;
	for (auto& i : m_maps)
	{
		if (i.second->UsesTileSet(tileSet))
			result.push_back(i.second);
	}
	return result;
}


vector<shared_ptr<Map>> Project::GetMapsUsingEffectLayer(shared_ptr<MapLayer> layer)
{
	vector<shared_ptr<Map>> result;
	for (auto& i : m_maps)
	{
		if (i.second->UsesEffectLayer(layer))
			result.push_back(i.second);
	}
	return result;
}


QString Project::GetFileName(const string& name, const string& id, const QString& ext)
{
	QString result;
	for (size_t i = 0; i < name.size(); i++)
	{
		if (((name[i] >= 'a') && (name[i] <= 'z')) ||
			((name[i] >= 'A') && (name[i] <= 'Z')) ||
			((name[i] >= '0') && (name[i] <= '9')))
			result += QString(name[i]);
		else
			result += "_";
	}

	result += ".";

	for (size_t i = 0; i < id.size(); i++)
	{
		if (((id[i] >= 'a') && (id[i] <= 'f')) ||
			((id[i] >= 'A') && (id[i] <= 'F')) ||
			((id[i] >= '0') && (id[i] <= '9')))
			result += QString(id[i]);
	}

	result += ext;
	return result;
}


bool Project::SaveProjectFile(const QString& path, const QString& name, const Json::Value& value)
{
	Json::StyledWriter writer;
	string valueStr = writer.write(value);

	QString fullPath = QDir(path).absoluteFilePath(name);
	FILE* fp = fopen(fullPath.toStdString().c_str(), "w");
	if (!fp)
		return false;

	if (fwrite(valueStr.c_str(), valueStr.size(), 1, fp) != 1)
	{
		fclose(fp);
		return false;
	}
	fclose(fp);
	return true;
}


bool Project::Save(const QString& path)
{
	QDir projectDir(path);
	if (!projectDir.exists())
	{
		if (!projectDir.mkpath("."))
			return false;
	}

	set<QString> files;
	Json::Value project(Json::objectValue);

	Json::Value palettes(Json::arrayValue);
	for (auto& i : m_palettesById)
	{
		QString name = GetFileName(i.second->GetName(), i.second->GetId(), ".s16pal");
		palettes.append(name.toStdString());
		files.insert(name);

		if (!SaveProjectFile(path, name, i.second->Serialize()))
			return false;
	}
	project["palettes"] = palettes;

	Json::Value tileSets(Json::arrayValue);
	for (auto& i : m_tileSetsById)
	{
		QString name = GetFileName(i.second->GetName(), i.second->GetId(), ".s16tile");
		tileSets.append(name.toStdString());
		files.insert(name);

		if (!SaveProjectFile(path, name, i.second->Serialize()))
			return false;
	}
	project["tilesets"] = tileSets;

	Json::Value effectLayers(Json::arrayValue);
	for (auto& i : m_effectLayersById)
	{
		QString name = GetFileName(i.second->GetName(), i.second->GetId(), ".s16layer");
		effectLayers.append(name.toStdString());
		files.insert(name);

		if (!SaveProjectFile(path, name, i.second->Serialize()))
			return false;
	}
	project["effect_layers"] = effectLayers;

	Json::Value maps(Json::arrayValue);
	for (auto& i : m_mapsById)
	{
		QString name = GetFileName(i.second->GetName(), i.second->GetId(), ".s16map");
		maps.append(name.toStdString());
		files.insert(name);

		if (!SaveProjectFile(path, name, i.second->Serialize()))
			return false;
	}
	project["maps"] = maps;

	Json::Value sprites(Json::arrayValue);
	for (auto& i : m_spritesById)
	{
		QString name = GetFileName(i.second->GetName(), i.second->GetId(), ".s16sprite");
		sprites.append(name.toStdString());
		files.insert(name);

		if (!SaveProjectFile(path, name, i.second->Serialize()))
			return false;
	}
	project["sprites"] = sprites;

	Json::Value actorTypes(Json::arrayValue);
	for (auto& i : m_actorTypesById)
	{
		QString name = GetFileName(i.second->GetName(), i.second->GetId(), ".s16actor");
		actorTypes.append(name.toStdString());
		files.insert(name);

		if (!SaveProjectFile(path, name, i.second->Serialize()))
			return false;
	}
	project["actor_types"] = actorTypes;

	if (!SaveProjectFile(path, "manifest.json", project))
		return false;
	files.insert("manifest.json");

	QStringList allFiles = projectDir.entryList(QDir::Files | QDir::NoDotAndDotDot);
	for (auto& i : allFiles)
	{
		if ((files.count(i) == 0) && (i.contains(".s16")))
			projectDir.remove(i);
	}
	return true;
}


bool Project::ReadProjectFile(const QString& path, const QString& name, Json::Value& result)
{
	QString fullPath = QDir(path).absoluteFilePath(name);
	FILE* fp = fopen(fullPath.toStdString().c_str(), "r");
	if (!fp)
		return false;

	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char* data = new char[size + 1];
	data[size] = 0;
	if (fread(data, size, 1, fp) != 1)
	{
		delete[] data;
		fclose(fp);
		return false;
	}
	fclose(fp);

	string dataStr = data;
	delete[] data;

	Json::Reader reader;
	return reader.parse(dataStr, result, false);
}


shared_ptr<Project> Project::Open(const QString& path)
{
	Json::Value manifest;
	if (!ReadProjectFile(path, "manifest.json", manifest))
	{
		QMessageBox::critical(nullptr, "Error", "Unable to read project manifest.");
		return nullptr;
	}

	shared_ptr<Project> project = make_shared<Project>();
	project->m_palettes.clear();
	project->m_palettesById.clear();
	project->m_tileSets.clear();
	project->m_tileSetsById.clear();
	project->m_maps.clear();
	project->m_mapsById.clear();
	project->m_sprites.clear();
	project->m_spritesById.clear();
	project->m_actorTypes.clear();
	project->m_actorTypesById.clear();

	for (auto& i : manifest["palettes"])
	{
		Json::Value data;
		if (!ReadProjectFile(path, QString::fromStdString(i.asString()), data))
		{
			QMessageBox::critical(nullptr, "Error", QString("Unable to read palette '") +
				QString::fromStdString(i.asString()) + QString("'."));
			return nullptr;
		}

		shared_ptr<Palette> palette = Palette::Deserialize(data);
		if (!palette)
		{
			if (i.isMember("name"))
			{
				QMessageBox::critical(nullptr, "Error", QString("Palette '") +
					QString::fromStdString(i["name"].asString()) +
					QString("' could not be read from project file."));
			}
			else
			{
				QMessageBox::critical(nullptr, "Error", QString("Palette with missing name could not be read from "
					"project file."));
			}
			return nullptr;
		}

		project->m_palettes[palette->GetName()] = palette;
		project->m_palettesById[palette->GetId()] = palette;
	}

	for (auto& i : manifest["tilesets"])
	{
		Json::Value data;
		if (!ReadProjectFile(path, QString::fromStdString(i.asString()), data))
		{
			QMessageBox::critical(nullptr, "Error", QString("Unable to read tile set '") +
				QString::fromStdString(i.asString()) + QString("'."));
			return nullptr;
		}

		shared_ptr<TileSet> tileSet = TileSet::Deserialize(project, data);
		if (!tileSet)
		{
			if (i.isMember("name"))
			{
				QMessageBox::critical(nullptr, "Error", QString("Tile set '") +
					QString::fromStdString(i["name"].asString()) +
					QString("' could not be read from project file."));
			}
			else
			{
				QMessageBox::critical(nullptr, "Error", QString("Tile set with missing name could not be read from "
					"project file."));
			}
			return nullptr;
		}

		project->m_tileSets[tileSet->GetName()] = tileSet;
		project->m_tileSetsById[tileSet->GetId()] = tileSet;
	}

	for (auto& i : manifest["effect_layers"])
	{
		Json::Value data;
		if (!ReadProjectFile(path, QString::fromStdString(i.asString()), data))
		{
			QMessageBox::critical(nullptr, "Error", QString("Unable to read effect layer '") +
				QString::fromStdString(i.asString()) + QString("'."));
			return nullptr;
		}

		shared_ptr<MapLayer> layer = MapLayer::Deserialize(project, data);
		if (!layer)
		{
			if (i.isMember("name"))
			{
				QMessageBox::critical(nullptr, "Error", QString("Effect layer '") +
					QString::fromStdString(i["name"].asString()) +
					QString("' could not be read from project file."));
			}
			else
			{
				QMessageBox::critical(nullptr, "Error", QString("Effect layer with missing name could not be read from "
					"project file."));
			}
			return nullptr;
		}

		project->m_effectLayers[layer->GetName()] = layer;
		project->m_effectLayersById[layer->GetId()] = layer;
	}

	for (auto& i : manifest["sprites"])
	{
		Json::Value data;
		if (!ReadProjectFile(path, QString::fromStdString(i.asString()), data))
		{
			QMessageBox::critical(nullptr, "Error", QString("Unable to read sprite '") +
				QString::fromStdString(i.asString()) + QString("'."));
			return nullptr;
		}

		shared_ptr<Sprite> sprite = Sprite::Deserialize(project, data);
		if (!sprite)
		{
			if (i.isMember("name"))
			{
				QMessageBox::critical(nullptr, "Error", QString("Sprite '") +
					QString::fromStdString(i["name"].asString()) +
					QString("' could not be read from project file."));
			}
			else
			{
				QMessageBox::critical(nullptr, "Error", QString("Sprite with missing name could not be read from "
					"project file."));
			}
			return nullptr;
		}

		project->m_sprites[sprite->GetName()] = sprite;
		project->m_spritesById[sprite->GetId()] = sprite;
	}

	for (auto& i : manifest["actor_types"])
	{
		Json::Value data;
		if (!ReadProjectFile(path, QString::fromStdString(i.asString()), data))
		{
			QMessageBox::critical(nullptr, "Error", QString("Unable to read actor type '") +
				QString::fromStdString(i.asString()) + QString("'."));
			return nullptr;
		}

		shared_ptr<ActorType> actorType = ActorType::Deserialize(project, data);
		if (!actorType)
		{
			if (i.isMember("name"))
			{
				QMessageBox::critical(nullptr, "Error", QString("Actor type '") +
					QString::fromStdString(i["name"].asString()) +
					QString("' could not be read from project file."));
			}
			else
			{
				QMessageBox::critical(nullptr, "Error", QString("Actor type with missing name could not be read from "
					"project file."));
			}
			return nullptr;
		}

		project->m_actorTypes[actorType->GetName()] = actorType;
		project->m_actorTypesById[actorType->GetId()] = actorType;
	}

	for (auto& i : manifest["maps"])
	{
		Json::Value data;
		if (!ReadProjectFile(path, QString::fromStdString(i.asString()), data))
		{
			QMessageBox::critical(nullptr, "Error", QString("Unable to read map '") +
				QString::fromStdString(i.asString()) + QString("'."));
			return nullptr;
		}

		shared_ptr<Map> map = Map::Deserialize(project, data);
		if (!map)
		{
			if (i.isMember("name"))
			{
				QMessageBox::critical(nullptr, "Error", QString("Map '") +
					QString::fromStdString(i["name"].asString()) +
					QString("' could not be read from project file."));
			}
			else
			{
				QMessageBox::critical(nullptr, "Error", QString("Map with missing name could not be read from "
					"project file."));
			}
			return nullptr;
		}

		project->m_maps[map->GetName()] = map;
		project->m_mapsById[map->GetId()] = map;
	}

	return project;
}


shared_ptr<Palette> Project::GetPaletteById(const string& id)
{
	auto i = m_palettesById.find(id);
	if (i == m_palettesById.end())
		return shared_ptr<Palette>();
	return i->second;
}


shared_ptr<TileSet> Project::GetTileSetById(const string& id)
{
	auto i = m_tileSetsById.find(id);
	if (i == m_tileSetsById.end())
		return shared_ptr<TileSet>();
	return i->second;
}


shared_ptr<MapLayer> Project::GetEffectLayerById(const string& id)
{
	auto i = m_effectLayersById.find(id);
	if (i == m_effectLayersById.end())
		return shared_ptr<MapLayer>();
	return i->second;
}


shared_ptr<Map> Project::GetMapById(const string& id)
{
	auto i = m_mapsById.find(id);
	if (i == m_mapsById.end())
		return shared_ptr<Map>();
	return i->second;
}


shared_ptr<Sprite> Project::GetSpriteById(const string& id)
{
	auto i = m_spritesById.find(id);
	if (i == m_spritesById.end())
		return shared_ptr<Sprite>();
	return i->second;
}


shared_ptr<ActorType> Project::GetActorTypeById(const string& id)
{
	auto i = m_actorTypesById.find(id);
	if (i == m_actorTypesById.end())
		return shared_ptr<ActorType>();
	return i->second;
}
