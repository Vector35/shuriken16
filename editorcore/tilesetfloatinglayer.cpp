#include "tilesetfloatinglayer.h"

using namespace std;


TileSetFloatingLayer::TileSetFloatingLayer(int x, int y, int width, int height)
{
	m_x = x;
	m_y = y;
	m_width = width;
	m_height = height;
	m_pixels = new TileSetFloatingLayerPixel[width * height];
	for (int i = 0; i < (width * height); i++)
		m_pixels[i].valid = false;
}


TileSetFloatingLayer::TileSetFloatingLayer(const TileSetFloatingLayer& other)
{
	m_x = other.m_x;
	m_y = other.m_y;
	m_width = other.m_width;
	m_height = other.m_height;
	m_pixels = new TileSetFloatingLayerPixel[m_width * m_height];
	for (int i = 0; i < (m_width * m_height); i++)
		m_pixels[i] = other.m_pixels[i];
}


TileSetFloatingLayer::~TileSetFloatingLayer()
{
	delete[] m_pixels;
}


TileSetFloatingLayerPixel TileSetFloatingLayer::GetPixel(int x, int y)
{
	if ((x < 0) || (y < 0) || (x >= m_width) || (y >= m_height))
	{
		TileSetFloatingLayerPixel result;
		result.valid = false;
		return result;
	}
	return m_pixels[(y * m_width) + x];
}


void TileSetFloatingLayer::ClearPixel(int x, int y)
{
	if ((x < 0) || (y < 0) || (x >= m_width) || (y >= m_height))
		return;
	m_pixels[(y * m_width) + x].valid = false;
}


void TileSetFloatingLayer::SetPixel(int x, int y, shared_ptr<Palette> palette, uint8_t entry)
{
	if ((x < 0) || (y < 0) || (x >= m_width) || (y >= m_height))
		return;
	m_pixels[(y * m_width) + x].valid = true;
	m_pixels[(y * m_width) + x].palette = palette;
	m_pixels[(y * m_width) + x].entry = entry;
}


void TileSetFloatingLayer::SetPixel(int x, int y, const TileSetFloatingLayerPixel& pixel)
{
	if ((x < 0) || (y < 0) || (x >= m_width) || (y >= m_height))
		return;
	m_pixels[(y * m_width) + x] = pixel;
}
