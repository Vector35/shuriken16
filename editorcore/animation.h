#pragma once

#include <vector>
#include "json/json.h"

class Animation
{
	std::vector<uint16_t> m_frameLengths;
	uint16_t m_totalLength;
	std::vector<size_t> m_frameForTime;

public:
	Animation();
	Animation(const Animation& anim);

	void AddFrame(uint16_t len);
	void InsertFrame(size_t i, uint16_t len);
	void SetFrameLength(size_t i, uint16_t len);
	void RemoveFrame(size_t i);

	size_t GetFrameCount() const { return m_frameLengths.size(); }
	const std::vector<uint16_t>& GetFrameLengths() const { return m_frameLengths; }
	uint16_t GetFrameLength(size_t i) const;
	uint16_t GetTotalLength() const { return m_totalLength; }

	size_t GetFrameForTime(uint16_t t);

	Json::Value Serialize();
	static std::shared_ptr<Animation> Deserialize(const Json::Value& data);
};
