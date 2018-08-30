#include <set>
#include "actor.h"
#include "project.h"

using namespace std;


Actor::Actor(const shared_ptr<ActorType>& type, size_t x, size_t y, size_t width, size_t height):
	m_type(type), m_x(x), m_y(y), m_width(width), m_height(height)
{
	for (auto& i : type->GetFields())
		m_fields[i.name] = i.type->GetDefaultValue(i.params);
}


Actor::Actor(const Actor& other)
{
	m_type = other.m_type;
	m_x = other.m_x;
	m_y = other.m_y;
	m_width = other.m_width;
	m_height = other.m_height;
	m_fields = other.m_fields;
}


void Actor::Move(size_t x, size_t y, size_t width, size_t height)
{
	m_x = x;
	m_y = y;
	m_width = width;
	m_height = height;
}


Json::Value Actor::GetFieldValue(const string& name)
{
	auto i = m_fields.find(name);
	if (i == m_fields.end())
		return Json::Value(Json::objectValue);
	return i->second;
}


void Actor::SetFieldValue(const string& name, const Json::Value& value)
{
	m_fields[name] = value;
}


Json::Value Actor::Serialize()
{
	Json::Value actor(Json::objectValue);
	actor["type_id"] = m_type->GetId();
	actor["type_name"] = m_type->GetName();
	actor["x"] = (uint64_t)m_x;
	actor["y"] = (uint64_t)m_y;
	if (m_type->HasBounds())
	{
		actor["width"] = (uint64_t)m_width;
		actor["height"] = (uint64_t)m_height;
	}

	set<string> validFields;
	for (auto& i : m_type->GetFields())
		validFields.insert(i.name);

	Json::Value fields(Json::objectValue);
	for (auto& i : m_fields)
	{
		if (validFields.count(i.first) == 0)
			continue;
		fields[i.first] = i.second;
	}
	actor["data"] = fields;
	return actor;
}


shared_ptr<Actor> Actor::Deserialize(shared_ptr<Project> project, const Json::Value& data)
{
	shared_ptr<ActorType> type = project->GetActorTypeById(data["type_id"].asString());
	if (!type)
		return shared_ptr<Actor>();

	size_t x = (size_t)data["x"].asUInt64();
	size_t y = (size_t)data["y"].asUInt64();
	size_t width = 1;
	size_t height = 1;
	if (data.isMember("width"))
		width = data["width"].asUInt64();
	if (data.isMember("height"))
		height = data["height"].asUInt64();

	shared_ptr<Actor> actor = make_shared<Actor>(type, x, y, width, height);
	for (auto& i : data["data"].getMemberNames())
		actor->m_fields[i] = data["data"][i];
	return actor;
}
