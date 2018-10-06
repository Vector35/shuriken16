#pragma once

#include <string>
#include <vector>
#include <functional>
#include <QWidget>
#include "sprite.h"
#include "map.h"

class MainWindow;
class Project;

class ActorFieldValue
{
	std::function<void(const Json::Value& oldValue, const Json::Value& newValue)> m_valueChangedFunc;
	Json::Value m_value;

public:
	ActorFieldValue(const Json::Value& currentValue,
		const std::function<void(const Json::Value& oldValue, const Json::Value& newValue)>& changedFunc);

	Json::Value GetValue() const { return m_value; }
	void SetValue(const Json::Value& value);
};

class ActorFieldType
{
	static std::vector<ActorFieldType*> m_types;
	static std::map<std::string, ActorFieldType*> m_typesByName;

public:
	virtual std::string GetName() = 0;
	virtual std::string GetDescription() = 0;
	virtual Json::Value GetDefaultParameters() = 0;
	virtual Json::Value GetDefaultValue(const Json::Value& params) = 0;
	virtual QWidget* CreateParameterEditor(MainWindow* mainWindow, const std::shared_ptr<Project>& project,
		const std::shared_ptr<ActorFieldValue>& value) = 0;
	virtual QWidget* CreateInstanceEditor(MainWindow* mainWindow, const std::shared_ptr<Project>& project,
		const std::shared_ptr<Map>& map, const Json::Value& params,
		const std::shared_ptr<ActorFieldValue>& value) = 0;

	static void Register(ActorFieldType* type);
	static ActorFieldType* GetTypeForName(const std::string& name);
	static std::vector<ActorFieldType*> GetTypes();
};

struct ActorField
{
	std::string name;
	ActorFieldType* type;
	Json::Value params;
};

class ActorType
{
	std::string m_name;
	bool m_hasBounds;
	std::vector<ActorField> m_fields;
	std::shared_ptr<Sprite> m_editorSprite;
	std::string m_id;

public:
	ActorType();
	ActorType(const ActorType& other);

	const std::string& GetName() const { return m_name; }
	void SetName(const std::string& name) { m_name = name; }

	bool HasBounds() const { return m_hasBounds; }
	void SetHasBounds(bool bounds) { m_hasBounds = bounds; }

	std::shared_ptr<Sprite> GetEditorSprite() { return m_editorSprite; }
	void SetEditorSprite(const std::shared_ptr<Sprite>& sprite) { m_editorSprite = sprite; }

	const std::vector<ActorField>& GetFields() const { return m_fields; }
	size_t GetFieldCount() const { return m_fields.size(); }
	const ActorField& GetField(size_t i) { return m_fields[i]; }
	void SetField(size_t i, const ActorField& field) { m_fields[i] = field; }
	void AddField(const ActorField& field);
	void InsertField(size_t i, const ActorField& field);
	void RemoveField(size_t i);

	const std::string& GetId() const { return m_id; }
	Json::Value Serialize();
	static std::shared_ptr<ActorType> Deserialize(std::shared_ptr<Project> project, const Json::Value& data);
};
