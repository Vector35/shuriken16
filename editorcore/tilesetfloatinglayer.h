#pragma once

#include "palette.h"

struct TileSetFloatingLayerPixel
{
	bool valid;
	std::shared_ptr<Palette> palette;
	uint8_t entry;
};

class TileSetFloatingLayer
{
	int m_x, m_y, m_width, m_height;
	TileSetFloatingLayerPixel* m_pixels;

public:
	TileSetFloatingLayer(int x, int y, int width, int height);
	TileSetFloatingLayer(const TileSetFloatingLayer& other);
	~TileSetFloatingLayer();

	int GetX() const { return m_x; }
	int GetY() const { return m_y; }
	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }
	void Move(int x, int y) { m_x = x; m_y = y; }

	TileSetFloatingLayerPixel GetPixel(int x, int y);
	void ClearPixel(int x, int y);
	void SetPixel(int x, int y, std::shared_ptr<Palette> palette, uint8_t entry);
	void SetPixel(int x, int y, const TileSetFloatingLayerPixel& pixel);
};
