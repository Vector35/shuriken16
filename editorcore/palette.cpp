#include <QUuid>
#include "palette.h"
#include "project.h"

using namespace std;


Palette::Palette()
{
	m_id = QUuid::createUuid().toString().toStdString();
	m_entries.resize(16);
}


Palette::Palette(const Palette& other)
{
	m_id = QUuid::createUuid().toString().toStdString();
	m_name = other.m_name;
	m_entries = other.m_entries;
}


uint16_t Palette::GetEntry(size_t i)
{
	if (i >= m_entries.size())
		return 0;
	return m_entries[i];
}


void Palette::SetEntry(size_t i, uint16_t value)
{
	if (i >= m_entries.size())
		return;
	m_entries[i] = value;
}


void Palette::SetEntryCount(size_t count)
{
	m_entries.resize(count);
}


Json::Value Palette::Serialize()
{
	Json::Value palette(Json::objectValue);
	palette["name"] = m_name;
	palette["id"] = m_id;

	Json::Value entries(Json::arrayValue);
	for (auto i : m_entries)
		entries.append(i);
	palette["entries"] = entries;

	return palette;
}


shared_ptr<Palette> Palette::Deserialize(const Json::Value& data)
{
	shared_ptr<Palette> result = make_shared<Palette>();
	result->m_name = data["name"].asString();
	result->m_id = data["id"].asString();

	result->m_entries.clear();
	for (auto& i : data["entries"])
		result->m_entries.push_back((uint16_t)i.asUInt());

	return result;
}


uint16_t Palette::FromRGB(uint8_t r, uint8_t g, uint8_t b)
{
	return ((uint16_t)(r & 31) << 10) | ((uint16_t)(g & 31) << 5) | ((uint16_t)(b & 31));
}


uint16_t Palette::FromRGB32(uint32_t value)
{
	return FromRGB((uint8_t)(value >> 19), (uint8_t)(value >> 11), (uint8_t)(value >> 3));
}


uint32_t Palette::ToRGB32(uint16_t value)
{
	return ((uint32_t)(value & 0x7c00) << 9) | ((uint32_t)(value & 0x3e0) << 6) |
		((uint32_t)(value & 0x1f) << 3);
}


uint16_t Palette::AddColor(uint16_t a, uint16_t b)
{
	uint16_t red = (a & 0x7c00) + (b & 0x7c00);
	uint16_t green = (a & 0x3e0) + (b & 0x3e0);
	uint16_t blue = (a & 0x1f) + (b & 0x1f);
	if (red & 0x8000)
		red = 0x7c00;
	if (green & 0x400)
		green = 0x3e0;
	if (blue & 0x20)
		blue = 0x1f;
	return red | green | blue;
}


uint16_t Palette::SubColor(uint16_t a, uint16_t b)
{
	uint16_t red = (a & 0x7c00) - (b & 0x7c00);
	uint16_t green = (a & 0x3e0) - (b & 0x3e0);
	uint16_t blue = (a & 0x1f) - (b & 0x1f);
	if (red & 0x8000)
		red = 0;
	if (green & 0x400)
		green = 0;
	if (blue & 0x20)
		blue = 0;
	return red | green | blue;
}


uint16_t Palette::MultiplyColor(uint16_t a, uint16_t b)
{
	uint16_t red = ((((a & 0x7c00) >> 10) * ((b & 0x7c00) >> 10)) << 6) & 0xfc00;
	uint16_t green = ((((a & 0x3e0) >> 5) * ((b & 0x3e0) >> 5)) << 1) & 0x7e0;
	uint16_t blue = (((a & 0x1f) * (b & 0x1f)) >> 4) & 0x3f;
	if (red & 0x8000)
		red = 0x7c00;
	if (green & 0x400)
		green = 0x3e0;
	if (blue & 0x20)
		blue = 0x1f;
	return red | green | blue;
}


uint16_t Palette::BlendColor(uint16_t a, uint16_t b, uint8_t alpha)
{
	alpha &= 15;
	uint16_t red = (((((a & 0x7c00) >> 10) * alpha) +
		(((b & 0x7c00) >> 10) * (16 - alpha))) << 6) & 0x7c00;
	uint16_t green = (((((a & 0x3e0) >> 5) * alpha) +
		(((b & 0x3e0) >> 5) * (16 - alpha))) << 1) & 0x3e0;
	uint16_t blue = (((a & 0x1f) * alpha) +
		((b & 0x1f) * (16 - alpha))) >> 4;
	return red | green | blue;
}
