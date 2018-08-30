#pragma once

#include "map.h"
#include "mapfloatinglayer.h"
#include "sprite.h"

class Renderer
{
	std::shared_ptr<Map> m_map;
	std::shared_ptr<MapLayer> m_activeLayer;
	std::shared_ptr<MapFloatingLayer> m_floatingLayer;
	std::map<std::shared_ptr<MapLayer>, bool> m_visibility;

	uint16_t m_width, m_height;
	uint16_t* m_pixels;
	uint16_t* m_singleLayerPixels;
	uint16_t m_backgroundColor;

	uint32_t m_animFrame;
	uint16_t m_scrollX, m_scrollY;
	bool m_parallaxEnabled;

	void RenderPixel(uint16_t* pixels, int16_t x, int16_t y, uint16_t color,
		BlendMode mode, uint8_t alpha);
	void RenderMapLayer(uint16_t* pixels, std::shared_ptr<MapLayer> layer, bool forceNormalBlend = false);
	void RenderSprite(uint16_t* pixels, int16_t x, int16_t y, std::shared_ptr<Sprite> sprite);
	bool IsLayerVisible(std::shared_ptr<MapLayer> layer);

public:
	Renderer(std::shared_ptr<Map> map, uint16_t width, uint16_t height);

	void SetActiveLayer(std::shared_ptr<MapLayer> layer) { m_activeLayer = layer; }
	void SetFloatingLayer(std::shared_ptr<MapFloatingLayer> layer) { m_floatingLayer = layer; }
	void SetLayerVisibility(const std::map<std::shared_ptr<MapLayer>, bool>& vis) { m_visibility = vis; }
	void SetBackgroundColor(uint16_t color) { m_backgroundColor = color; }
	void SetParallaxEnabled(bool enable) { m_parallaxEnabled = enable; }

	void Render();
	void RenderSingleLayer();
	const uint16_t* GetPixelDataForRow(uint16_t y);
	const uint16_t* GetSingleLayerPixelDataForRow(uint16_t y);

	int16_t GetScrollX() const { return m_scrollX; }
	int16_t GetScrollY() const { return m_scrollY; }
	void SetScroll(int16_t x, int16_t y) { m_scrollX = x; m_scrollY = y; }

	void ResetAnimation() { m_animFrame = 0; }
	void TickAnimation() { m_animFrame++; }
};
