#pragma once

#include <string>
#include <vector>
#include "spriteanimation.h"
#include "maplayer.h"

class Sprite
{
	std::string m_name;
	size_t m_width, m_height, m_depth;
	std::vector<std::shared_ptr<SpriteAnimation>> m_animations;
	std::string m_id;

public:
	Sprite(size_t width, size_t height, size_t depth);
	Sprite(const Sprite& other);

	const std::string& GetName() const { return m_name; }
	void SetName(const std::string& name) { m_name = name; }

	size_t GetWidth() const { return m_width; }
	size_t GetHeight() const { return m_height; }
	size_t GetDepth() const { return m_depth; }

	const std::vector<std::shared_ptr<SpriteAnimation>>& GetAnimations() const { return m_animations; }
	size_t GetAnimationCount() const { return m_animations.size(); }
	std::shared_ptr<SpriteAnimation> GetAnimation(size_t i);
	void SetAnimation(size_t i, std::shared_ptr<SpriteAnimation> anim);
	void AddAnimation(std::shared_ptr<SpriteAnimation> anim);
	void InsertAnimation(size_t i, std::shared_ptr<SpriteAnimation> anim);
	void RemoveAnimation(size_t i);

	std::shared_ptr<SpriteAnimation> CreateAnimation(const std::string& name);

	bool UsesPalette(std::shared_ptr<Palette> palette);

	const std::string& GetId() const { return m_id; }
	Json::Value Serialize();
	static std::shared_ptr<Sprite> Deserialize(std::shared_ptr<Project> project, const Json::Value& data);
};
