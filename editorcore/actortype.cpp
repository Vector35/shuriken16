#include <QUuid>
#include "actortype.h"
#include "project.h"

using namespace std;


vector<ActorFieldType*> ActorFieldType::m_types = vector<ActorFieldType*>();
map<string, ActorFieldType*> ActorFieldType::m_typesByName = map<string, ActorFieldType*>();


ActorFieldValue::ActorFieldValue(const Json::Value& currentValue,
	const function<void(const Json::Value& oldValue, const Json::Value& newValue)>& changedFunc):
	m_valueChangedFunc(changedFunc), m_value(currentValue)
{
}


void ActorFieldValue::SetValue(const Json::Value& value)
{
	Json::Value oldValue = m_value;
	if (m_value != value)
	{
		m_value = value;
		m_valueChangedFunc(oldValue, value);
	}
}


void ActorFieldType::Register(ActorFieldType* type)
{
	m_types.push_back(type);
	m_typesByName[type->GetName()] = type;
}


ActorFieldType* ActorFieldType::GetTypeForName(const string& name)
{
	auto i = m_typesByName.find(name);
	if (i == m_typesByName.end())
		return nullptr;
	return i->second;
}


vector<ActorFieldType*> ActorFieldType::GetTypes()
{
	return m_types;
}


ActorType::ActorType()
{
	m_id = QUuid::createUuid().toString().toStdString();
	m_hasBounds = false;
}


ActorType::ActorType(const ActorType& other)
{
	m_id = QUuid::createUuid().toString().toStdString();
	m_name = other.m_name;
	m_hasBounds = other.m_hasBounds;
	m_fields = other.m_fields;
	m_editorSprite = other.m_editorSprite;
}


void ActorType::AddField(const ActorField& field)
{
	m_fields.push_back(field);
}


void ActorType::InsertField(size_t i, const ActorField& field)
{
	if (i > m_fields.size())
		i = m_fields.size();
	m_fields.insert(m_fields.begin() + i, field);
}


void ActorType::RemoveField(size_t i)
{
	if (i < m_fields.size())
		m_fields.erase(m_fields.begin() + i);
}


Json::Value ActorType::Serialize()
{
	Json::Value result(Json::objectValue);
	result["name"] = m_name;
	result["id"] = m_id;
	result["bounds"] = m_hasBounds;

	if (m_editorSprite)
		result["sprite"] = m_editorSprite->GetId();

	Json::Value fields(Json::arrayValue);
	for (auto& i : m_fields)
	{
		Json::Value field(Json::objectValue);
		field["name"] = i.name;
		field["type"] = i.type->GetName();
		field["params"] = i.params;
		fields.append(field);
	}
	result["fields"] = fields;
	return result;
}


shared_ptr<ActorType> ActorType::Deserialize(shared_ptr<Project> project, const Json::Value& data)
{
	shared_ptr<ActorType> result = make_shared<ActorType>();
	result->m_name = data["name"].asString();
	result->m_id = data["id"].asString();
	result->m_hasBounds = data["bounds"].asBool();

	if (data.isMember("sprite"))
		result->m_editorSprite = project->GetSpriteById(data["sprite"].asString());

	for (auto& i : data["fields"])
	{
		ActorField field;
		field.name = i["name"].asString();
		field.type = ActorFieldType::GetTypeForName(i["type"].asString());
		if (!field.type)
			return shared_ptr<ActorType>();
		field.params = i["params"];
		result->m_fields.push_back(field);
	}

	return result;
}
