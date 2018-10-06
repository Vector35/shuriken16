#pragma once

#include <string>
#include <vector>
#include <memory>
#include <inttypes.h>
#include "json/json.h"

class Project;

class Palette
{
	std::string m_name;
	std::vector<uint16_t> m_entries;
	std::string m_id;

public:
	Palette();
	Palette(const Palette& other);

	const std::string& GetName() const { return m_name; }
	void SetName(const std::string& name) { m_name = name; }

	const std::vector<uint16_t>& GetEntries() const { return m_entries; }
	size_t GetEntryCount() const { return m_entries.size(); }
	uint16_t GetEntry(size_t i);
	void SetEntry(size_t i, uint16_t value);
	void SetEntryCount(size_t count);

	const std::string& GetId() const { return m_id; }
	Json::Value Serialize();
	static std::shared_ptr<Palette> Deserialize(const Json::Value& data);

	static uint16_t FromRGB(uint8_t r, uint8_t g, uint8_t b); // 5-bit components
	static uint16_t FromRGB32(uint32_t value);
	static uint32_t ToRGB32(uint16_t value);

	static uint16_t AddColor(uint16_t a, uint16_t b);
	static uint16_t SubColor(uint16_t a, uint16_t b);
	static uint16_t MultiplyColor(uint16_t a, uint16_t b);
	static uint16_t BlendColor(uint16_t a, uint16_t b, uint8_t alpha);
};
