#include "renderer.h"

using namespace std;


Renderer::Renderer(shared_ptr<Map> map, uint16_t width, uint16_t height):
	m_map(map), m_width(width), m_height(height)
{
	m_pixels = new uint16_t[(size_t)m_width * (size_t)m_height];
	m_singleLayerPixels = new uint16_t[(size_t)m_width * (size_t)m_height];
	m_backgroundColor = 0;
	m_scrollX = 0;
	m_scrollY = 0;
	m_animFrame = 0;
	m_parallaxEnabled = true;
}


void Renderer::RenderPixel(uint16_t* pixels, int16_t x, int16_t y, uint16_t color,
	BlendMode mode, uint8_t alpha)
{
	uint16_t* curPixel = &pixels[((size_t)y * (size_t)m_width) + (size_t)x];
	uint16_t existingColor = *curPixel;

	// Apply the current blending mode
	switch (mode)
	{
	case BlendMode_Add:
		color = Palette::AddColor(existingColor, color);
		break;
	case BlendMode_Subtract:
		color = Palette::SubColor(existingColor, color);
		break;
	case BlendMode_Multiply:
		color = Palette::MultiplyColor(existingColor, color);
		break;
	default:
		break;
	}

	// Compute and store final pixel value using the alpha blending mode
	if (alpha == 0)
		*curPixel = color;
	else
		*curPixel = Palette::BlendColor(existingColor, color, alpha);
}


void Renderer::RenderMapLayer(uint16_t* pixels, shared_ptr<MapLayer> layer, bool forceNormalBlend)
{
	int16_t scrollX = m_scrollX;
	int16_t scrollY = m_scrollY;
	if (m_parallaxEnabled)
	{
		int animX = ((int)layer->GetAutoScrollX() * m_animFrame) / 0x100;
		int animY = ((int)layer->GetAutoScrollY() * m_animFrame) / 0x100;
		scrollX = (int16_t)((scrollX * layer->GetParallaxFactorX()) / 0x100 + animX);
		scrollY = (int16_t)((scrollY * layer->GetParallaxFactorY()) / 0x100 + animY);
	}

	// Capture layer settings
	uint16_t tileWidth = layer->GetTileWidth();
	uint16_t tileHeight = layer->GetTileHeight();
	size_t tileDepth = layer->GetTileDepth();
	BlendMode blendMode = layer->GetBlendMode();
	uint8_t alpha = layer->GetAlpha();

	if (forceNormalBlend)
	{
		blendMode = BlendMode_Normal;
		alpha = 0;
	}

	uint16_t leftTile = scrollX / tileWidth;
	uint16_t leftPixel = scrollX % tileWidth;
	uint16_t rightTile = (scrollX + m_width - 1) / tileWidth;
	uint16_t rightPixel = (scrollX + m_width - 1) % tileWidth;
	uint16_t topTile = scrollY / tileHeight;
	uint16_t topPixel = scrollY % tileHeight;
	uint16_t bottomTile = (scrollY + m_height - 1) / tileHeight;
	uint16_t bottomPixel = (scrollY + m_height - 1) % tileHeight;

	// Render layer
	uint16_t targetY = 0;
	for (uint16_t tileY = topTile; tileY <= bottomTile; tileY++)
	{
		// Compute rendering extents for current row of tiles
		uint16_t curTopPixel, curBottomPixel;
		if (tileY == topTile)
			curTopPixel = topPixel;
		else
			curTopPixel = 0;
		if (tileY == bottomTile)
			curBottomPixel = bottomPixel;
		else
			curBottomPixel = tileHeight - 1;

		uint16_t targetX = 0;
		for (uint16_t tileX = leftTile; tileX <= rightTile; tileX++)
		{
			// Look up tile in map layer
			TileReference ref = layer->GetTileAt(tileX, tileY);
			if (m_floatingLayer && (m_floatingLayer->GetMapLayer() == layer) &&
				((int)tileX >= m_floatingLayer->GetX()) && ((int)tileY >= m_floatingLayer->GetY()) &&
				((int)tileX < (m_floatingLayer->GetX() + m_floatingLayer->GetWidth())) &&
				((int)tileY < (m_floatingLayer->GetY() + m_floatingLayer->GetHeight())))
			{
				MapFloatingLayerTile floatingTile = m_floatingLayer->GetTile(tileX - m_floatingLayer->GetX(),
					tileY - m_floatingLayer->GetY());
				if (floatingTile.valid)
				{
					ref.tileSet = floatingTile.tileSet;
					ref.index = floatingTile.index;
				}
			}

			if (!ref.tileSet)
			{
				if (tileX == leftTile)
					targetX += tileWidth - leftPixel;
				else
					targetX += tileWidth;
				continue;
			}

			shared_ptr<Tile> tile = ref.tileSet->GetTile(ref.index);
			if (!tile)
			{
				if (tileX == leftTile)
					targetX += tileWidth - leftPixel;
				else
					targetX += tileWidth;
				continue;
			}
			shared_ptr<Palette> palette = tile->GetPalette();
			if ((!palette) && (tileDepth != 16))
			{
				if (tileX == leftTile)
					targetX += tileWidth - leftPixel;
				else
					targetX += tileWidth;
				continue;
			}

			uint16_t frame = ref.tileSet->GetFrameForTime(m_animFrame);
			const uint8_t* tileData = tile->GetData(frame);

			// Compute rendering extents for current tile
			uint16_t curLeftPixel, curRightPixel;
			if (tileX == leftTile)
				curLeftPixel = leftPixel;
			else
				curLeftPixel = 0;
			if (tileX == rightTile)
				curRightPixel = rightPixel;
			else
				curRightPixel = tileWidth - 1;

			for (uint16_t pixelY = curTopPixel; pixelY <= curBottomPixel; pixelY++)
			{
				const uint8_t* tileDataRow = &tileData[pixelY * tile->GetPitch()];
				for (uint16_t pixelX = curLeftPixel; pixelX <= curRightPixel; pixelX++)
				{
					uint16_t color = 0;
					if (tile->GetDepth() == 4)
					{
						uint8_t colorIndex = (tileDataRow[pixelX / 2] >> (((pixelX) & 1) << 2)) & 0xf;
						if (colorIndex == 0)
							continue;
						color = palette->GetEntry(tile->GetPaletteOffset() + colorIndex);
					}
					else if (tile->GetDepth() == 8)
					{
						uint8_t colorIndex = tileDataRow[pixelX];
						if (colorIndex == 0)
							continue;
						color = palette->GetEntry(tile->GetPaletteOffset() + colorIndex);
					}
					else if (tile->GetDepth() == 16)
					{
						color = *(const uint16_t*)&tileDataRow[pixelX * 2];
						if (color & 0x8000)
							continue;
					}

					RenderPixel(pixels, targetX + pixelX - curLeftPixel, targetY + pixelY - curTopPixel, color, blendMode, alpha);
				}
			}

			if (tileX == leftTile)
				targetX += tileWidth - leftPixel;
			else
				targetX += tileWidth;
		}

		if (tileY == topTile)
			targetY += tileHeight - topPixel;
		else
			targetY += tileHeight;
	}
}


bool Renderer::IsLayerVisible(shared_ptr<MapLayer> layer)
{
	auto i = m_visibility.find(layer);
	if (i == m_visibility.end())
		return true;
	return i->second;
}


void Renderer::Render()
{
	for (size_t i = 0; i < ((size_t)m_width * (size_t)m_height); i++)
		m_pixels[i] = m_backgroundColor;

	for (auto& i : m_map->GetLayers())
	{
		if ((i != m_activeLayer) && !IsLayerVisible(i))
			continue;
		RenderMapLayer(m_pixels, i);
	}
}


void Renderer::RenderSingleLayer()
{
	for (size_t i = 0; i < ((size_t)m_width * (size_t)m_height); i++)
		m_singleLayerPixels[i] = m_backgroundColor;

	if (m_activeLayer)
		RenderMapLayer(m_singleLayerPixels, m_activeLayer, true);
}


const uint16_t* Renderer::GetPixelDataForRow(uint16_t y)
{
	return &m_pixels[(size_t)y * (size_t)m_width];
}


const uint16_t* Renderer::GetSingleLayerPixelDataForRow(uint16_t y)
{
	return &m_singleLayerPixels[(size_t)y * (size_t)m_width];
}
