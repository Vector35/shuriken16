#pragma once

#include "actortype.h"

class Actor
{
	std::shared_ptr<ActorType> m_type;
	size_t m_x, m_y, m_width, m_height;
	std::map<std::string, Json::Value> m_fields;

public:
	Actor(const std::shared_ptr<ActorType>& type, size_t x, size_t y, size_t width = 1, size_t height = 1);
	Actor(const Actor& other);

	std::shared_ptr<ActorType> GetType() const { return m_type; }

	size_t GetX() const { return m_x; }
	size_t GetY() const { return m_y; }
	size_t GetWidth() const { return m_width; }
	size_t GetHeight() const { return m_height; }
	void Move(size_t x, size_t y, size_t width = 1, size_t height = 1);

	const std::map<std::string, Json::Value>& GetFields() const { return m_fields; }
	Json::Value GetFieldValue(const std::string& name);
	void SetFieldValue(const std::string& name, const Json::Value& value);

	Json::Value Serialize();
	static std::shared_ptr<Actor> Deserialize(std::shared_ptr<Project> project, const Json::Value& data);
};
