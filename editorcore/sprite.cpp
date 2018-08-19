#include <QUuid>
#include "sprite.h"

using namespace std;


Sprite::Sprite(size_t width, size_t height, size_t depth)
{
	m_id = QUuid::createUuid().toString().toStdString();
	m_width = width;
	m_height = height;
	m_depth = depth;

	m_animations.push_back(CreateAnimation("Idle"));
}


Sprite::Sprite(const Sprite& other)
{
	m_id = QUuid::createUuid().toString().toStdString();
	m_name = other.m_name;
	m_width = other.m_width;
	m_height = other.m_height;
	m_depth = other.m_depth;

	for (auto& i : other.m_animations)
		m_animations.push_back(make_shared<SpriteAnimation>(*i));
}


shared_ptr<SpriteAnimation> Sprite::GetAnimation(size_t i)
{
	if (i >= m_animations.size())
		return nullptr;
	return m_animations[i];
}


void Sprite::SetAnimation(size_t i, shared_ptr<SpriteAnimation> anim)
{
	if (i >= m_animations.size())
		return;
	m_animations[i] = anim;
}


void Sprite::AddAnimation(shared_ptr<SpriteAnimation> anim)
{
	m_animations.push_back(anim);
}


void Sprite::InsertAnimation(size_t i, shared_ptr<SpriteAnimation> anim)
{
	if (i > m_animations.size())
		i = m_animations.size();
	m_animations.insert(m_animations.begin() + i, anim);
}


void Sprite::RemoveAnimation(size_t i)
{
	if (i >= m_animations.size())
		return;
	m_animations.erase(m_animations.begin() + i);
}


shared_ptr<SpriteAnimation> Sprite::CreateAnimation(const std::string& name)
{
	return make_shared<SpriteAnimation>(name, m_width, m_height, m_depth);
}


bool Sprite::UsesPalette(shared_ptr<Palette> palette)
{
	for (auto& i : m_animations)
	{
		if (i->UsesPalette(palette))
			return true;
	}
	return false;
}


Json::Value Sprite::Serialize()
{
	Json::Value sprite(Json::objectValue);
	sprite["name"] = m_name;
	sprite["id"] = m_id;
	sprite["width"] = (uint64_t)m_width;
	sprite["height"] = (uint64_t)m_height;
	sprite["depth"] = (uint64_t)m_depth;

	Json::Value anims(Json::arrayValue);
	for (auto& i : m_animations)
		anims.append(i->Serialize());
	sprite["anim"] = anims;

	return sprite;
}


shared_ptr<Sprite> Sprite::Deserialize(shared_ptr<Project> project, const Json::Value& data)
{
	size_t width = (size_t)data["width"].asUInt64();
	size_t height = (size_t)data["height"].asUInt64();
	size_t depth = (size_t)data["depth"].asUInt64();

	shared_ptr<Sprite> result = make_shared<Sprite>(width, height, depth);
	result->m_name = data["name"].asString();
	result->m_id = data["id"].asString();

	result->m_animations.clear();
	for (auto& i : data["anim"])
	{
		shared_ptr<SpriteAnimation> anim = SpriteAnimation::Deserialize(project, i, width, height, depth);
		if (!anim)
			return shared_ptr<Sprite>();
		result->m_animations.push_back(anim);
	}

	return result;
}
