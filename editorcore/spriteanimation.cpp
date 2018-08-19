#include "spriteanimation.h"

using namespace std;


SpriteAnimation::SpriteAnimation(const string& name, size_t width, size_t height, size_t depth)
{
	m_name = name;
	m_tile = make_shared<Tile>(width, height, depth);
	m_animation = make_shared<Animation>();
	m_animation->AddFrame(8);
	m_loop = true;
}


SpriteAnimation::SpriteAnimation(const SpriteAnimation& other)
{
	m_name = other.m_name;
	m_tile = make_shared<Tile>(*other.m_tile);
	m_animation = make_shared<Animation>(*other.m_animation);
	m_loop = other.m_loop;
}


size_t SpriteAnimation::GetFrameCount() const
{
	return m_animation->GetFrameCount();
}


void SpriteAnimation::SetAnimation(shared_ptr<Animation> anim)
{
	m_animation = anim;
	m_tile->SetFrameCount((uint16_t)GetFrameCount());
}


uint16_t SpriteAnimation::GetFrameForTime(uint32_t ticks)
{
	if (m_loop)
	{
		ticks %= (uint32_t)m_animation->GetTotalLength();
		return m_animation->GetFrameForTime((uint16_t)ticks);
	}

	if (ticks >= m_animation->GetTotalLength())
		return m_animation->GetFrameCount() - 1;
	return m_animation->GetFrameForTime((uint16_t)ticks);
}


void SpriteAnimation::CopyFrame(uint16_t from, uint16_t to)
{
	m_tile->CopyFrame(from, to);
}


void SpriteAnimation::SwapFrames(uint16_t from, uint16_t to)
{
	m_tile->SwapFrames(from, to);
	uint16_t tempFrameLen = m_animation->GetFrameLength(to);
	m_animation->SetFrameLength(to, m_animation->GetFrameLength(from));
	m_animation->SetFrameLength(from, tempFrameLen);
}


void SpriteAnimation::DuplicateFrame(uint16_t frame)
{
	m_tile->DuplicateFrame(frame);
	uint16_t len = m_animation->GetFrameLength(frame);
	m_animation->InsertFrame(frame, len);
}


void SpriteAnimation::RemoveFrame(uint16_t frame)
{
	if ((frame >= m_animation->GetFrameCount()) || (m_animation->GetFrameCount() == 1))
		return;
	m_tile->RemoveFrame(frame);
	m_animation->RemoveFrame(frame);
}


shared_ptr<Tile> SpriteAnimation::GetTileForSingleFrame(uint16_t frame)
{
	return m_tile->GetTileForSingleFrame(frame);
}


void SpriteAnimation::InsertFrameFromTile(uint16_t frame, const shared_ptr<Tile>& tile, uint16_t length)
{
	m_tile->InsertFrameFromTile(frame, tile);
	m_animation->InsertFrame(frame, length);
}


bool SpriteAnimation::UsesPalette(shared_ptr<Palette> palette)
{
	return m_tile->GetPalette() == palette;
}


Json::Value SpriteAnimation::Serialize()
{
	Json::Value anim(Json::objectValue);
	anim["name"] = m_name;
	anim["tile"] = m_tile->Serialize();
	anim["anim"] = m_animation->Serialize();
	anim["looping"] = m_loop;
	return anim;
}


shared_ptr<SpriteAnimation> SpriteAnimation::Deserialize(shared_ptr<Project> project, const Json::Value& data,
	size_t width, size_t height, size_t depth)
{
	string name = data["name"].asString();
	shared_ptr<SpriteAnimation> result = make_shared<SpriteAnimation>(name, width, height, depth);

	result->m_animation = Animation::Deserialize(data["anim"]);
	if (!result->m_animation)
		return shared_ptr<SpriteAnimation>();

	result->m_tile = Tile::Deserialize(project, data["tile"], width, height, depth, result->m_animation->GetFrameCount());
	if (!result->m_tile)
		return shared_ptr<SpriteAnimation>();

	result->m_loop = data["looping"].asBool();
	return result;
}
