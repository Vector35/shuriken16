#include "animation.h"

using namespace std;


Animation::Animation()
{
	m_totalLength = 0;
}


Animation::Animation(const Animation& anim)
{
	m_frameLengths = anim.m_frameLengths;
	m_totalLength = anim.m_totalLength;
}


void Animation::AddFrame(uint16_t len)
{
	m_frameLengths.push_back(len);
	m_totalLength += len;
	m_frameForTime.clear();
}


void Animation::InsertFrame(size_t i, uint16_t len)
{
	if (i >= m_frameLengths.size())
		i = m_frameLengths.size();
	m_frameLengths.insert(m_frameLengths.begin() + i, len);
	m_totalLength += len;
	m_frameForTime.clear();
}


void Animation::SetFrameLength(size_t i, uint16_t len)
{
	if (i >= m_frameLengths.size())
		return;
	m_totalLength -= m_frameLengths[i];
	m_frameLengths[i] = len;
	m_totalLength += len;
	m_frameForTime.clear();
}


void Animation::RemoveFrame(size_t i)
{
	if (i >= m_frameLengths.size())
		return;
	m_totalLength -= m_frameLengths[i];
	m_frameLengths.erase(m_frameLengths.begin() + i);
	m_frameForTime.clear();
}


uint16_t Animation::GetFrameLength(size_t i) const
{
	if (i >= m_frameLengths.size())
		return 0;
	return m_frameLengths[i];
}


size_t Animation::GetFrameForTime(uint16_t t)
{
	if ((t == 0) || (m_frameLengths.size() == 0))
		return 0;

	if (m_frameForTime.size() == 0)
	{
		for (size_t i = 0; i < m_frameLengths.size(); i++)
			for (uint16_t j = 0; j < m_frameLengths[i]; j++)
				m_frameForTime.push_back(i);
	}

	if (t >= m_frameForTime.size())
		return m_frameLengths.size() - 1;
	return m_frameForTime[t];
}


Json::Value Animation::Serialize()
{
	Json::Value anim(Json::arrayValue);
	for (auto i : m_frameLengths)
		anim.append(i);
	return anim;
}


shared_ptr<Animation> Animation::Deserialize(const Json::Value& data)
{
	shared_ptr<Animation> result = make_shared<Animation>();
	for (auto& i : data)
		result->AddFrame((uint16_t)i.asUInt());
	return result;
}
