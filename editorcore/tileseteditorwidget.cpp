#include <QVBoxLayout>
#include <QPainter>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QGuiApplication>
#include <QClipboard>
#include <QScrollBar>
#include <QMessageBox>
#include <set>
#include <queue>
#include <stdlib.h>
#include <math.h>
#include "tileseteditorwidget.h"
#include "tilesetanimationwidget.h"
#include "tilesetview.h"
#include "theme.h"
#include "mainwindow.h"
#include "json/json.h"

using namespace std;


TileSetEditorWidget::TileSetEditorWidget(QWidget* parent, MainWindow* mainWindow, QScrollArea* scrollArea,
	shared_ptr<Project> project, shared_ptr<TileSet> tileSet):
	QWidget(parent), m_mainWindow(mainWindow), m_scrollArea(scrollArea),
	m_project(project), m_tileSet(tileSet), m_animWidget(nullptr), m_previewWidget(nullptr)
{
	QPalette style(this->palette());
	style.setColor(QPalette::Window, Theme::background);
	setPalette(style);

	ResetPalette();

	m_zoom = 16;
	m_mouseDown = false;
	m_showHover = false;
	m_tool = PenTool;
	m_frame = 0;

	setMouseTracking(true);
}


void TileSetEditorWidget::ResetPalette()
{
	m_palette.reset();
	for (auto& i : m_project->GetPalettes())
	{
		if (m_tileSet->UsesPalette(i.second))
		{
			m_palette = i.second;
			m_rightPaletteEntry = 0;
			if (i.second->GetEntryCount() > 1)
				m_leftPaletteEntry = 1;
			else
				m_leftPaletteEntry = 0;
			break;
		}
	}
	if (!m_palette)
	{
		for (auto& i : m_project->GetPalettes())
		{
			m_palette = i.second;
			m_rightPaletteEntry = 0;
			if (i.second->GetEntryCount() > 1)
				m_leftPaletteEntry = 1;
			else
				m_leftPaletteEntry = 0;
			break;
		}
	}
}


void TileSetEditorWidget::UpdateView()
{
	m_columns = m_tileSet->GetDisplayColumns();
	if (m_columns > m_tileSet->GetTileCount())
		m_columns = m_tileSet->GetTileCount();
	m_rows = (m_tileSet->GetTileCount() + (m_columns - 1)) / m_columns;

	if (m_palette && !m_project->GetPaletteById(m_palette->GetId()))
	{
		// Palette is no longer valid
		ResetPalette();
	}

	resize((int)(m_tileSet->GetWidth() * m_columns * m_zoom),
		(int)(m_tileSet->GetHeight() * m_rows * m_zoom));
	update();
}


shared_ptr<Palette> TileSetEditorWidget::GetSelectedPalette() const
{
	return m_palette;
}


size_t TileSetEditorWidget::GetSelectedLeftPaletteEntry() const
{
	return m_leftPaletteEntry;
}


size_t TileSetEditorWidget::GetSelectedRightPaletteEntry() const
{
	return m_rightPaletteEntry;
}


void TileSetEditorWidget::SetSelectedLeftPaletteEntry(std::shared_ptr<Palette> palette, size_t entry)
{
	m_palette = palette;
	m_leftPaletteEntry = entry;

	if (m_tileSet->GetDepth() == 4)
	{
		m_rightPaletteEntry = (entry & 0xf0) | (m_rightPaletteEntry & 0xf);
		if (m_rightPaletteEntry >= palette->GetEntryCount())
			m_rightPaletteEntry = entry & 0xf0;
	}
}


void TileSetEditorWidget::SetSelectedRightPaletteEntry(std::shared_ptr<Palette> palette, size_t entry)
{
	m_palette = palette;
	m_rightPaletteEntry = entry;

	if (m_tileSet->GetDepth() == 4)
	{
		m_leftPaletteEntry = (entry & 0xf0) | (m_leftPaletteEntry & 0xf);
		if (m_leftPaletteEntry >= palette->GetEntryCount())
			m_leftPaletteEntry = entry & 0xf0;
	}
}


void TileSetEditorWidget::SetFrame(uint16_t frame)
{
	if (m_selectionContents)
	{
		SelectAction action;
		action.oldSelectionContents = m_selectionContents;
		action.oldUnderSelection = m_underSelection;
		m_pendingSelections.push_back(action);
		CommitPendingActions();

		m_selectionContents.reset();
		m_underSelection.reset();
	}

	m_frame = frame;
	UpdateView();

	if (m_animWidget)
		m_animWidget->UpdateView();
	if (m_previewWidget)
		m_previewWidget->SetActiveFrame(frame);
}


void TileSetEditorWidget::paintEvent(QPaintEvent* event)
{
	QPainter p(this);

	int tileWidth = (int)m_tileSet->GetWidth() * m_zoom;
	int tileHeight = (int)m_tileSet->GetHeight() * m_zoom;

	bool hoverValid = false;

	for (int tileY = 0; tileY < (int)m_rows; tileY++)
	{
		if (((tileY + 1) * tileHeight) < event->rect().top())
			continue;
		if ((tileY * tileHeight) > event->rect().bottom())
			break;

		for (int tileX = 0; tileX < (int)m_columns; tileX++)
		{
			if (((tileX + 1) * tileWidth) < event->rect().left())
				continue;
			if ((tileX * tileWidth) > event->rect().right())
				break;

			size_t tileIndex = (size_t)((tileY * m_columns) + tileX);
			if (tileIndex >= m_tileSet->GetTileCount())
				break;

			shared_ptr<Tile> tile = m_tileSet->GetTile(tileIndex);
			if (!tile)
				continue;
			if ((tile->GetWidth() != m_tileSet->GetWidth()))
				continue;
			if ((tile->GetHeight() != m_tileSet->GetHeight()))
				continue;
			if ((tile->GetDepth() != m_tileSet->GetDepth()))
				continue;

			shared_ptr<Palette> paletteOverride;
			uint8_t paletteOverrideOffset;
			if (m_showHover)
			{
				if ((m_hoverX >= (tileX * (int)m_tileSet->GetWidth())) &&
					(m_hoverX < ((tileX + 1) * (int)m_tileSet->GetWidth())) &&
					(m_hoverY >= (tileY * (int)m_tileSet->GetHeight())) &&
					(m_hoverY < ((tileY + 1) * (int)m_tileSet->GetHeight())))
				{
					hoverValid = true;
					paletteOverride = m_palette;
					if (m_tileSet->GetDepth() == 4)
						paletteOverrideOffset = m_leftPaletteEntry & 0xf0;
					else
						paletteOverrideOffset = 0;
				}
			}

			p.setBrush(QBrush(Theme::backgroundHighlight, Qt::BDiagPattern));
			p.setPen(Qt::NoPen);
			p.drawRect(tileWidth * tileX, tileHeight * tileY, tileWidth, tileHeight);

			for (int y = 0; y < (int)m_tileSet->GetHeight(); y++)
			{
				if ((tileY * tileHeight + (y + 1) * m_zoom) < event->rect().top())
					continue;
				if ((tileY * tileHeight + y * m_zoom) > event->rect().bottom())
					break;

				for (int x = 0; x < (int)m_tileSet->GetWidth(); x++)
				{
					if ((tileX * tileWidth + (x + 1) * m_zoom) < event->rect().left())
						continue;
					if ((tileX * tileWidth + x * m_zoom) > event->rect().right())
						break;

					shared_ptr<Palette> palette = tile->GetPalette();
					uint8_t colorIndex;
					if (tile->GetDepth() == 4)
						colorIndex = (tile->GetData(m_frame)[(y * tile->GetPitch()) + (x / 2)] >> ((x & 1) << 2)) & 0xf;
					else
						colorIndex = tile->GetData(m_frame)[(y * tile->GetPitch()) + x];
					if (paletteOverride)
						colorIndex += paletteOverrideOffset;
					else
						colorIndex += tile->GetPaletteOffset();

					if (m_floatingLayer)
					{
						int absX = (tileX * (int)m_tileSet->GetWidth()) + x;
						int absY = (tileY * (int)m_tileSet->GetHeight()) + y;
						if ((absX >= m_floatingLayer->GetX()) && (absY >= m_floatingLayer->GetY()) &&
							(absX < (m_floatingLayer->GetX() + m_floatingLayer->GetWidth())) &&
							(absY < (m_floatingLayer->GetY() + m_floatingLayer->GetHeight())))
						{
							TileSetFloatingLayerPixel pixel = m_floatingLayer->GetPixel(
								absX - m_floatingLayer->GetX(), absY - m_floatingLayer->GetY());
							if (pixel.valid)
							{
								palette = pixel.palette;
								colorIndex = pixel.entry;
							}
						}
					}

					if (colorIndex == 0)
						continue;
					if (!palette)
						continue;

					uint16_t paletteEntry;
					if (paletteOverride)
						paletteEntry = paletteOverride->GetEntry(colorIndex);
					else
						paletteEntry = palette->GetEntry(colorIndex);

					p.setBrush(QColor::fromRgba(Palette::ToRGB32(paletteEntry) | 0xff000000));
					p.drawRect(tileWidth * tileX + x * m_zoom, tileHeight * tileY + y * m_zoom, m_zoom, m_zoom);
				}
			}

			if (m_zoom >= 8)
			{
				for (int y = 0; y < (int)m_tileSet->GetHeight(); y++)
				{
					if ((tileY * tileHeight + (y + 1) * m_zoom) < event->rect().top())
						continue;
					if ((tileY * tileHeight + y * m_zoom) > event->rect().bottom())
						break;

					for (int x = 0; x < (int)m_tileSet->GetWidth(); x++)
					{
						if ((tileX * tileWidth + (x + 1) * m_zoom) < event->rect().left())
							continue;
						if ((tileX * tileWidth + x * m_zoom) > event->rect().right())
							break;

						p.setPen(QPen(QBrush(Theme::backgroundHighlight), 1));
						p.setBrush(Qt::NoBrush);
						p.drawRect(tileWidth * tileX + x * m_zoom, tileHeight * tileY + y * m_zoom, m_zoom, m_zoom);
					}
				}
			}

			if (m_zoom >= 4)
			{
				for (int y = 0; y < (int)m_tileSet->GetHeight(); y += 8)
				{
					if ((tileY * tileHeight + (y + 8) * m_zoom) < event->rect().top())
						continue;
					if ((tileY * tileHeight + y * m_zoom) > event->rect().bottom())
						break;

					for (int x = 0; x < (int)m_tileSet->GetWidth(); x += 8)
					{
						if ((tileX * tileWidth + (x + 8) * m_zoom) < event->rect().left())
							continue;
						if ((tileX * tileWidth + x * m_zoom) > event->rect().right())
							break;

						p.setPen(QPen(QBrush(Theme::backgroundHighlight), 2));
						p.setBrush(Qt::NoBrush);
						p.drawRect(tileWidth * tileX + x * m_zoom, tileHeight * tileY + y * m_zoom, m_zoom * 8, m_zoom * 8);
					}
				}
			}

			if (m_zoom >= 2)
			{
				if (m_zoom >= 8)
					p.setPen(QPen(QBrush(Theme::disabled), 2));
				else
					p.setPen(QPen(QBrush(Theme::disabled), 1));
				p.setBrush(Qt::NoBrush);
				p.drawRect(tileWidth * tileX, tileHeight * tileY, tileWidth, tileHeight);
			}

			if (m_tool == CollisionTool)
			{
				for (auto& i : tile->GetCollision())
				{
					p.setPen(QPen(QBrush(Theme::red), 2, Qt::DashDotLine));
					p.drawRect(tileWidth * tileX + i.x * m_zoom + (m_zoom / 4), tileHeight * tileY + i.y * m_zoom + (m_zoom / 4),
						i.width * m_zoom - m_zoom / 2, i.height * m_zoom - m_zoom / 2);
				}
			}
		}
	}

	if ((m_tool == SelectTool) && m_floatingLayer)
	{
		// Moving selection, draw border of floating selection
		p.setPen(QPen(QBrush(Theme::backgroundDark), 1));
		p.setBrush(Qt::NoBrush);
		p.drawRect(m_floatingLayer->GetX() * m_zoom, m_floatingLayer->GetY() * m_zoom,
			m_floatingLayer->GetWidth() * m_zoom, m_floatingLayer->GetHeight() * m_zoom);
		p.setPen(QPen(QBrush(Theme::content), 1, Qt::DashLine));
		p.drawRect(m_floatingLayer->GetX() * m_zoom, m_floatingLayer->GetY() * m_zoom,
			m_floatingLayer->GetWidth() * m_zoom, m_floatingLayer->GetHeight() * m_zoom);
	}
	else if (m_selectionContents)
	{
		// Selection active, draw border of selection
		p.setPen(QPen(QBrush(Theme::backgroundDark), 1));
		p.setBrush(Qt::NoBrush);
		p.drawRect(m_selectionContents->GetX() * m_zoom, m_selectionContents->GetY() * m_zoom,
			m_selectionContents->GetWidth() * m_zoom, m_selectionContents->GetHeight() * m_zoom);
		p.setPen(QPen(QBrush(Theme::content), 1, Qt::DashLine));
		p.drawRect(m_selectionContents->GetX() * m_zoom, m_selectionContents->GetY() * m_zoom,
			m_selectionContents->GetWidth() * m_zoom, m_selectionContents->GetHeight() * m_zoom);
	}

	if (hoverValid && (m_zoom >= 8))
	{
		p.setPen(QPen(QBrush(Theme::blue), 2));
		p.setBrush(Qt::NoBrush);
		p.drawRect(m_hoverX * m_zoom, m_hoverY * m_zoom, m_zoom, m_zoom);
	}
}


TileSetFloatingLayerPixel TileSetEditorWidget::GetPixel(int x, int y)
{
	TileSetFloatingLayerPixel result;
	result.valid = false;

	if ((x < 0) || (y < 0))
		return result;

	int tileX = x / m_tileSet->GetWidth();
	int tileY = y / m_tileSet->GetHeight();
	if ((tileX < 0) || (tileX >= (int)m_columns) || (tileY < 0) || (tileY >= (int)m_rows))
		return result;

	size_t tileIndex = (tileY * m_columns) + tileX;
	if (tileIndex >= m_tileSet->GetTileCount())
		return result;
	shared_ptr<Tile> tile = m_tileSet->GetTile(tileIndex);
	if (!tile)
		return result;

	int pixelX = x % m_tileSet->GetWidth();
	int pixelY = y % m_tileSet->GetHeight();
	uint8_t colorIndex;
	if (tile->GetDepth() == 4)
	{
		size_t offset = (pixelY * tile->GetPitch()) + (pixelX / 2);
		colorIndex = (tile->GetData(m_frame)[offset] >> ((pixelX & 1) << 2)) & 0xf;
	}
	else
	{
		size_t offset = (pixelY * tile->GetPitch()) + pixelX;
		colorIndex = tile->GetData(m_frame)[offset];
	}

	if ((colorIndex == 0) || (!tile->GetPalette()))
	{
		result.valid = true;
		result.entry = 0;
		return result;
	}

	result.valid = true;
	result.palette = tile->GetPalette();
	result.entry = colorIndex + tile->GetPaletteOffset();
	return result;
}


void TileSetEditorWidget::SetPixel(int x, int y, shared_ptr<Palette> palette, uint8_t entry)
{
	if ((x < 0) || (y < 0))
		return;
	int tileX = x / m_tileSet->GetWidth();
	int tileY = y / m_tileSet->GetHeight();
	if ((tileX < 0) || (tileX >= (int)m_columns) || (tileY < 0) || (tileY >= (int)m_rows))
		return;

	size_t tileIndex = (tileY * m_columns) + tileX;
	if (tileIndex >= m_tileSet->GetTileCount())
		return;
	shared_ptr<Tile> tile = m_tileSet->GetTile(tileIndex);
	if (!tile)
		return;

	if (!palette)
		entry = 0;
	if (entry == 0)
		palette = tile->GetPalette();

	int pixelX = x % m_tileSet->GetWidth();
	int pixelY = y % m_tileSet->GetHeight();
	uint8_t* data;
	uint8_t newData, colorIndex, paletteOffset;
	size_t offset;
	if (tile->GetDepth() == 4)
	{
		colorIndex = (uint8_t)(entry & 0xf);
		paletteOffset = (uint8_t)(entry & 0xf0);
		offset = (pixelY * tile->GetPitch()) + (pixelX / 2);
		data = &tile->GetData(m_frame)[offset];
		newData = ((*data) & (0xf0 >> ((pixelX & 1) << 2))) | (colorIndex << ((pixelX & 1) << 2));
	}
	else
	{
		colorIndex = entry;
		paletteOffset = 0;
		offset = (pixelY * tile->GetPitch()) + pixelX;
		data = &tile->GetData(m_frame)[offset];
		newData = colorIndex;
	}

	if ((newData != *data) || ((colorIndex != 0) && ((palette != tile->GetPalette()) ||
		(paletteOffset != tile->GetPaletteOffset()))))
	{
		EditAction action;
		action.tileIndex = tileIndex;
		action.offset = offset;
		action.oldData = *data;
		action.newData = newData;
		action.oldPalette = tile->GetPalette();
		action.oldPaletteOffset = tile->GetPaletteOffset();
		if ((colorIndex != 0) && ((palette != tile->GetPalette()) ||
			(paletteOffset != tile->GetPaletteOffset())))
		{
			action.newPalette = palette;
			action.newPaletteOffset = paletteOffset;
		}
		else
		{
			action.newPalette = action.oldPalette;
			action.newPaletteOffset = action.oldPaletteOffset;
		}

		if ((action.oldData == action.newData) && (action.oldPalette == action.newPalette) &&
			(action.oldPaletteOffset == action.newPaletteOffset))
			return;

		m_pendingActions.push_back(action);

		*data = newData;
		if ((colorIndex != 0) && ((palette != tile->GetPalette()) ||
			(paletteOffset != tile->GetPaletteOffset())))
		{
			shared_ptr<Palette> oldPalette = tile->GetPalette();
			bool usedNewPalette = false;
			bool usesOldPalette = false;
			if (palette)
				usedNewPalette = m_tileSet->UsesPalette(palette);

			tile->SetPalette(palette, paletteOffset);

			if (oldPalette)
				usesOldPalette = m_tileSet->UsesPalette(oldPalette);

			if (oldPalette && !usesOldPalette)
				m_mainWindow->UpdatePaletteContents(oldPalette);
			if (palette && !usedNewPalette)
				m_mainWindow->UpdatePaletteContents(palette);
		}
	}
}


void TileSetEditorWidget::SetPixelForMouseEvent(QMouseEvent* event)
{
	int x = event->x() / m_zoom;
	int y = event->y() / m_zoom;
	SetPixel(x, y, m_palette, m_mouseDownPaletteEntry);
	m_mainWindow->UpdateTileSetContents(m_tileSet);
}


void TileSetEditorWidget::CaptureLayer(shared_ptr<TileSetFloatingLayer> layer)
{
	int leftX = layer->GetX();
	int topY = layer->GetY();
	int width = layer->GetWidth();
	int height = layer->GetHeight();

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			int tileX = (leftX + x) / m_tileSet->GetWidth();
			int tileY = (topY + y) / m_tileSet->GetHeight();
			int tilePixelX = (leftX + x) % m_tileSet->GetWidth();
			int tilePixelY = (topY + y) % m_tileSet->GetHeight();
			if ((tileX < 0) || (tileX >= (int)m_columns) || (tileY < 0) || (tileY >= (int)m_rows))
			{
				layer->SetPixel(x, y, shared_ptr<Palette>(), 0);
				continue;
			}

			size_t tileIndex = (tileY * m_columns) + tileX;
			if (tileIndex >= m_tileSet->GetTileCount())
			{
				layer->SetPixel(x, y, shared_ptr<Palette>(), 0);
				continue;
			}
			shared_ptr<Tile> tile = m_tileSet->GetTile(tileIndex);
			if (!tile)
			{
				layer->SetPixel(x, y, shared_ptr<Palette>(), 0);
				continue;
			}

			shared_ptr<Palette> palette = tile->GetPalette();
			uint8_t colorIndex;
			if (tile->GetDepth() == 4)
			{
				colorIndex = (tile->GetData(m_frame)[(tilePixelY * tile->GetPitch()) +
					(tilePixelX / 2)] >> ((tilePixelX & 1) << 2)) & 0xf;
			}
			else
			{
				colorIndex = tile->GetData(m_frame)[(tilePixelY * tile->GetPitch()) + tilePixelX];
			}
			colorIndex += tile->GetPaletteOffset();

			layer->SetPixel(x, y, palette, colorIndex);
		}
	}
}


void TileSetEditorWidget::ApplyLayer(shared_ptr<TileSetFloatingLayer> layer)
{
	if (!layer)
		return;

	for (int y = 0; y < layer->GetHeight(); y++)
	{
		int absY = layer->GetY() + y;
		if (absY < 0)
			continue;

		for (int x = 0; x < layer->GetWidth(); x++)
		{
			int absX = layer->GetX() + x;
			if (absX < 0)
				continue;

			TileSetFloatingLayerPixel pixel = layer->GetPixel(x, y);
			if (!pixel.valid)
				continue;

			if (pixel.palette)
				SetPixel(absX, absY, pixel.palette, pixel.entry);
			else
				SetPixel(absX, absY, shared_ptr<Palette>(), 0);
		}
	}

	m_mainWindow->UpdateTileSetContents(m_tileSet);
}


void TileSetEditorWidget::UpdateSelectionLayer(QMouseEvent* event)
{
	int startX = m_startX;
	int startY = m_startY;
	int curX = event->x() / m_zoom;
	int curY = event->y() / m_zoom;

	int tileSetWidth = (int)(m_columns * m_tileSet->GetWidth());
	int tileSetHeight = (int)(m_rows * m_tileSet->GetHeight());

	if ((tileSetWidth == 0) || (tileSetHeight == 0))
		return;

	if (startX < 0)
		startX = 0;
	if (startX >= tileSetWidth)
		startX = tileSetWidth - 1;
	if (startY < 0)
		startY = 0;
	if (startY >= tileSetHeight)
		startY = tileSetHeight - 1;
	if (curX < 0)
		curX = 0;
	if (curX >= tileSetWidth)
		curX = tileSetWidth - 1;
	if (curY < 0)
		curY = 0;
	if (curY >= tileSetHeight)
		curY = tileSetHeight - 1;

	int leftX = (startX < curX) ? startX : curX;
	int rightX = (startX < curX) ? curX : startX;
	int topY = (startY < curY) ? startY : curY;
	int botY = (startY < curY) ? curY : startY;

	int width = (rightX - leftX) + 1;
	int height = (botY - topY) + 1;

	if (m_selectionContents && (leftX == m_selectionContents->GetX()) &&
		(topY == m_selectionContents->GetY()) && (width == m_selectionContents->GetWidth()) &&
		(height == m_selectionContents->GetHeight()))
		return;

	SelectAction action;
	action.oldSelectionContents = m_selectionContents;
	action.oldUnderSelection = m_underSelection;

	m_selectionContents = make_shared<TileSetFloatingLayer>(leftX, topY, width, height);
	m_underSelection = make_shared<TileSetFloatingLayer>(leftX, topY, width, height);

	action.newSelectionContents = m_selectionContents;
	action.newUnderSelection = m_underSelection;
	m_pendingSelections.push_back(action);

	CaptureLayer(m_selectionContents);

	// If selection is moved, replace with transparent
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
			m_underSelection->SetPixel(x, y, shared_ptr<Palette>(), 0);
}


bool TileSetEditorWidget::IsMouseInSelection(QMouseEvent* event)
{
	if (!m_selectionContents)
		return false;

	int curX = event->x() / m_zoom;
	int curY = event->y() / m_zoom;

	if ((curX >= m_selectionContents->GetX()) && (curY >= m_selectionContents->GetY()) &&
		(curX < (m_selectionContents->GetX() + m_selectionContents->GetWidth())) &&
		(curY < (m_selectionContents->GetY() + m_selectionContents->GetHeight())))
		return true;
	return false;
}


void TileSetEditorWidget::BeginMoveSelectionLayer()
{
	ApplyLayer(m_underSelection);
	m_floatingLayer = make_shared<TileSetFloatingLayer>(*m_selectionContents);
}


void TileSetEditorWidget::MoveSelectionLayer(QMouseEvent* event)
{
	if (!m_floatingLayer)
		return;

	int curX = event->x() / m_zoom;
	int curY = event->y() / m_zoom;
	int deltaX = curX - m_startX;
	int deltaY = curY - m_startY;
	m_startX = curX;
	m_startY = curY;

	if ((deltaX == 0) && (deltaY == 0))
		return;

	m_floatingLayer->Move(m_floatingLayer->GetX() + deltaX, m_floatingLayer->GetY() + deltaY);
	update();
}


void TileSetEditorWidget::FinishMoveSelectionLayer()
{
	SelectAction action;
	action.oldSelectionContents = m_selectionContents;
	action.oldUnderSelection = m_underSelection;
	action.newSelectionContents = m_floatingLayer;

	m_underSelection = make_shared<TileSetFloatingLayer>(m_floatingLayer->GetX(), m_floatingLayer->GetY(),
		m_floatingLayer->GetWidth(), m_floatingLayer->GetHeight());
	CaptureLayer(m_underSelection);

	action.newUnderSelection = m_underSelection;
	m_pendingSelections.push_back(action);

	m_selectionContents = m_floatingLayer;
	ApplyLayer(m_selectionContents);
	m_floatingLayer.reset();
}


void TileSetEditorWidget::UpdateRectangleLayer(QMouseEvent* event)
{
	int curX = event->x() / m_zoom;
	int curY = event->y() / m_zoom;

	int leftX = (m_startX < curX) ? m_startX : curX;
	int rightX = (m_startX < curX) ? curX : m_startX;
	int topY = (m_startY < curY) ? m_startY : curY;
	int botY = (m_startY < curY) ? curY : m_startY;

	int width = (rightX - leftX) + 1;
	int height = (botY - topY) + 1;

	m_floatingLayer = make_shared<TileSetFloatingLayer>(leftX, topY, width, height);
	for (int y = 0; y < height; y++)
	{
		m_floatingLayer->SetPixel(0, y, m_palette, m_mouseDownPaletteEntry);
		m_floatingLayer->SetPixel(width - 1, y, m_palette, m_mouseDownPaletteEntry);
	}
	for (int x = 0; x < width; x++)
	{
		m_floatingLayer->SetPixel(x, 0, m_palette, m_mouseDownPaletteEntry);
		m_floatingLayer->SetPixel(x, height - 1, m_palette, m_mouseDownPaletteEntry);
	}

	update();
}


void TileSetEditorWidget::UpdateFilledRectangleLayer(QMouseEvent* event)
{
	int curX = event->x() / m_zoom;
	int curY = event->y() / m_zoom;

	int leftX = (m_startX < curX) ? m_startX : curX;
	int rightX = (m_startX < curX) ? curX : m_startX;
	int topY = (m_startY < curY) ? m_startY : curY;
	int botY = (m_startY < curY) ? curY : m_startY;

	int width = (rightX - leftX) + 1;
	int height = (botY - topY) + 1;

	m_floatingLayer = make_shared<TileSetFloatingLayer>(leftX, topY, width, height);
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
			m_floatingLayer->SetPixel(x, y, m_palette, m_mouseDownPaletteEntry);

	update();
}


void TileSetEditorWidget::UpdateCircleLayer(QMouseEvent* event)
{
	int curX = event->x() / m_zoom;
	int curY = event->y() / m_zoom;

	int distx = ((m_startX < curX) ? curX : m_startX) - ((m_startX < curX) ? m_startX : curX);
	int disty = ((m_startY < curY) ? curY : m_startY) - ((m_startY < curY) ? m_startY : curY);
	int radius = sqrt((distx * distx) + (disty * disty)) + 0.5;
	int x = radius-1;
	int y = 0;
	int dx = 1;
	int dy = 1;
	int err = dx - (radius << 1);

	int orgx = m_startX - radius;
	int orgy = m_startY - radius;

	m_floatingLayer = make_shared<TileSetFloatingLayer>(orgx, orgy, (radius * 2) + 1, (radius * 2) + 1);
	while (x >= y)
	{
		m_floatingLayer->SetPixel(m_startX - orgx + x, m_startY - orgy + y, m_palette, m_mouseDownPaletteEntry);
		m_floatingLayer->SetPixel(m_startX - orgx + y, m_startY - orgy + x, m_palette, m_mouseDownPaletteEntry);
		m_floatingLayer->SetPixel(m_startX - orgx - y, m_startY - orgy + x, m_palette, m_mouseDownPaletteEntry);
		m_floatingLayer->SetPixel(m_startX - orgx - x, m_startY - orgy + y, m_palette, m_mouseDownPaletteEntry);
		m_floatingLayer->SetPixel(m_startX - orgx - x, m_startY - orgy - y, m_palette, m_mouseDownPaletteEntry);
		m_floatingLayer->SetPixel(m_startX - orgx - y, m_startY - orgy - x, m_palette, m_mouseDownPaletteEntry);
		m_floatingLayer->SetPixel(m_startX - orgx + y, m_startY - orgy - x, m_palette, m_mouseDownPaletteEntry);
		m_floatingLayer->SetPixel(m_startX - orgx + x, m_startY - orgy - y, m_palette, m_mouseDownPaletteEntry);

		if (err <= 0)
		{
			y++;
			err += dy;
			dy += 2;
		}

		if (err > 0)
		{
			x--;
			dx += 2;
			err += dx - (radius << 1);
		}
    }

	update();
}


void TileSetEditorWidget::UpdateLineLayer(QMouseEvent* event)
{
	int curX = event->x() / m_zoom;
	int curY = event->y() / m_zoom;

	int leftX = (m_startX < curX) ? m_startX : curX;
	int rightX = (m_startX < curX) ? curX : m_startX;
	int topY = (m_startY < curY) ? m_startY : curY;
	int botY = (m_startY < curY) ? curY : m_startY;

	if ((leftX == rightX) && (topY == botY))
	{
		// Single pixel, draw as a rectangle to simplify logic
		UpdateFilledRectangleLayer(event);
		return;
	}

	int width = (rightX - leftX) + 1;
	int height = (botY - topY) + 1;

	int deltaX = curX - m_startX;
	int deltaY = curY - m_startY;
	int deltaMagnitudeX = deltaX;
	int deltaMagnitudeY = deltaY;
	if (deltaMagnitudeX < 0)
		deltaMagnitudeX = -deltaMagnitudeX;
	if (deltaMagnitudeY < 0)
		deltaMagnitudeY = -deltaMagnitudeY;

	m_floatingLayer = make_shared<TileSetFloatingLayer>(leftX, topY, width, height);

	if (deltaMagnitudeX > deltaMagnitudeY)
	{
		int start = m_startX;
		int end = curX;
		int stepX = (deltaX < 0) ? -1 : 1;
		int stepY = (deltaY < 0) ? -1 : 1;
		int yFrac = 0;
		int y = m_startY;
		for (int x = start; x != end; x += stepX)
		{
			m_floatingLayer->SetPixel(x - leftX, y - topY, m_palette, m_mouseDownPaletteEntry);
			yFrac += deltaMagnitudeY;
			if (yFrac > (deltaMagnitudeX / 2))
			{
				yFrac -= deltaMagnitudeX;
				y += stepY;
			}
		}
	}
	else
	{
		int start = m_startY;
		int end = curY;
		int stepX = (deltaX < 0) ? -1 : 1;
		int stepY = (deltaY < 0) ? -1 : 1;
		int xFrac = 0;
		int x = m_startX;
		for (int y = start; y != end; y += stepY)
		{
			m_floatingLayer->SetPixel(x - leftX, y - topY, m_palette, m_mouseDownPaletteEntry);
			xFrac += deltaMagnitudeX;
			if (xFrac > (deltaMagnitudeY / 2))
			{
				xFrac -= deltaMagnitudeY;
				x += stepX;
			}
		}
	}

	m_floatingLayer->SetPixel(curX - leftX, curY - topY, m_palette, m_mouseDownPaletteEntry);
	update();
}


void TileSetEditorWidget::Fill(QMouseEvent* event)
{
	int curX = event->x() / m_zoom;
	int curY = event->y() / m_zoom;
	TileSetFloatingLayerPixel replacement;
	replacement.palette = m_palette;
	replacement.entry = m_mouseDownPaletteEntry;
	if ((m_mouseDownPaletteEntry == 0) || ((m_tileSet->GetDepth() == 4) && ((m_mouseDownPaletteEntry & 0xf) == 0)))
	{
		// Transparent
		replacement.palette.reset();
		replacement.entry = 0;
	}

	TileSetFloatingLayerPixel target = GetPixel(curX, curY);
	if (!target.valid)
		return;
	if ((target.palette == replacement.palette) && (target.entry == replacement.entry))
		return;

	queue<pair<int, int>> workQueue;
	workQueue.push(pair<int, int>(curX, curY));
	while (!workQueue.empty())
	{
		int x = workQueue.front().first;
		int y = workQueue.front().second;
		workQueue.pop();

		int start = x;
		int end = x;

		for (; start > 0; start--)
		{
			TileSetFloatingLayerPixel left = GetPixel(start - 1, y);
			if (!left.valid)
				break;
			if ((left.palette != target.palette) || (left.entry != target.entry))
				break;
		}
		for (; end < (int)((m_columns * m_tileSet->GetWidth()) - 1); end++)
		{
			TileSetFloatingLayerPixel right = GetPixel(end + 1, y);
			if (!right.valid)
				break;
			if ((right.palette != target.palette) || (right.entry != target.entry))
				break;
		}

		for (int fill = start; fill <= end; fill++)
		{
			SetPixel(fill, y, m_palette, m_mouseDownPaletteEntry);

			TileSetFloatingLayerPixel up = GetPixel(fill, y - 1);
			if (up.valid && (up.palette == target.palette) && (up.entry == target.entry))
				workQueue.push(pair<int, int>(fill, y - 1));

			TileSetFloatingLayerPixel down = GetPixel(fill, y + 1);
			if (down.valid && (down.palette == target.palette) && (down.entry == target.entry))
				workQueue.push(pair<int, int>(fill, y + 1));
		}
	}

	m_mainWindow->UpdateTileSetContents(m_tileSet);
}


void TileSetEditorWidget::AddSelectionAsCollision()
{
	shared_ptr<TileSetFloatingLayer> layer = m_selectionContents;
	m_selectionContents.reset();
	m_underSelection.reset();
	m_pendingSelections.clear();
	if (!layer)
		return;

	int tileX = layer->GetX() / m_tileSet->GetWidth();
	int tileY = layer->GetY() / m_tileSet->GetHeight();
	if ((tileX < 0) || (tileX >= (int)m_columns) || (tileY < 0) || (tileY >= (int)m_rows))
		return;

	size_t tileIndex = (tileY * m_columns) + tileX;
	if (tileIndex >= m_tileSet->GetTileCount())
		return;
	shared_ptr<Tile> tile = m_tileSet->GetTile(tileIndex);
	if (!tile)
		return;

	BoundingRect rect;
	rect.x = layer->GetX() % m_tileSet->GetWidth();
	rect.y = layer->GetY() % m_tileSet->GetHeight();
	rect.width = layer->GetWidth();
	rect.height = layer->GetHeight();
	if ((rect.x + rect.width) > m_tileSet->GetWidth())
		rect.width = m_tileSet->GetWidth() - rect.x;
	if ((rect.y + rect.height) > m_tileSet->GetHeight())
		rect.height = m_tileSet->GetHeight() - rect.y;

	vector<BoundingRect> oldCollision = tile->GetCollision();
	vector<BoundingRect> newCollision = oldCollision;
	newCollision.push_back(rect);

	tile->SetCollision(newCollision);
	m_mainWindow->UpdateTileSetContents(m_tileSet);

	MainWindow* mainWindow = m_mainWindow;
	shared_ptr<TileSet> tileSet = m_tileSet;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			shared_ptr<Tile> tile = tileSet->GetTile(tileIndex);
			tile->SetCollision(oldCollision);
			mainWindow->UpdateTileSetContents(tileSet);
			TileSetView* view = mainWindow->GetTileSetView(tileSet);
			if (view)
			{
				view->GetEditor()->SetTool(CollisionTool);
				view->UpdateToolState();
			}
		},
		[=]() { // Redo
			shared_ptr<Tile> tile = tileSet->GetTile(tileIndex);
			tile->SetCollision(newCollision);
			mainWindow->UpdateTileSetContents(tileSet);
			TileSetView* view = mainWindow->GetTileSetView(tileSet);
			if (view)
			{
				view->GetEditor()->SetTool(CollisionTool);
				view->UpdateToolState();
			}
		});
}


void TileSetEditorWidget::SetSelectionAsCollision()
{
	shared_ptr<TileSetFloatingLayer> layer = m_selectionContents;
	m_selectionContents.reset();
	m_underSelection.reset();
	m_pendingSelections.clear();
	if (!layer)
		return;

	int tileX = layer->GetX() / m_tileSet->GetWidth();
	int tileY = layer->GetY() / m_tileSet->GetHeight();
	if ((tileX < 0) || (tileX >= (int)m_columns) || (tileY < 0) || (tileY >= (int)m_rows))
		return;

	size_t tileIndex = (tileY * m_columns) + tileX;
	if (tileIndex >= m_tileSet->GetTileCount())
		return;
	shared_ptr<Tile> tile = m_tileSet->GetTile(tileIndex);
	if (!tile)
		return;

	BoundingRect rect;
	rect.x = layer->GetX() % m_tileSet->GetWidth();
	rect.y = layer->GetY() % m_tileSet->GetHeight();
	rect.width = layer->GetWidth();
	rect.height = layer->GetHeight();
	if ((rect.x + rect.width) > m_tileSet->GetWidth())
		rect.width = m_tileSet->GetWidth() - rect.x;
	if ((rect.y + rect.height) > m_tileSet->GetHeight())
		rect.height = m_tileSet->GetHeight() - rect.y;

	vector<BoundingRect> oldCollision = tile->GetCollision();
	vector<BoundingRect> newCollision;
	newCollision.push_back(rect);

	tile->SetCollision(newCollision);
	m_mainWindow->UpdateTileSetContents(m_tileSet);

	MainWindow* mainWindow = m_mainWindow;
	shared_ptr<TileSet> tileSet = m_tileSet;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			shared_ptr<Tile> tile = tileSet->GetTile(tileIndex);
			tile->SetCollision(oldCollision);
			mainWindow->UpdateTileSetContents(tileSet);
			TileSetView* view = mainWindow->GetTileSetView(tileSet);
			if (view)
			{
				view->GetEditor()->SetTool(CollisionTool);
				view->UpdateToolState();
			}
		},
		[=]() { // Redo
			shared_ptr<Tile> tile = tileSet->GetTile(tileIndex);
			tile->SetCollision(newCollision);
			mainWindow->UpdateTileSetContents(tileSet);
			TileSetView* view = mainWindow->GetTileSetView(tileSet);
			if (view)
			{
				view->GetEditor()->SetTool(CollisionTool);
				view->UpdateToolState();
			}
		});
}


void TileSetEditorWidget::RemoveSingleCollision()
{
	int tileX = m_startX / m_tileSet->GetWidth();
	int tileY = m_startY / m_tileSet->GetHeight();
	if ((tileX < 0) || (tileX >= (int)m_columns) || (tileY < 0) || (tileY >= (int)m_rows))
		return;

	size_t tileIndex = (tileY * m_columns) + tileX;
	if (tileIndex >= m_tileSet->GetTileCount())
		return;
	shared_ptr<Tile> tile = m_tileSet->GetTile(tileIndex);
	if (!tile)
		return;

	vector<BoundingRect> oldCollision = tile->GetCollision();
	vector<BoundingRect> newCollision;

	tile->SetCollision(newCollision);
	m_mainWindow->UpdateTileSetContents(m_tileSet);

	MainWindow* mainWindow = m_mainWindow;
	shared_ptr<TileSet> tileSet = m_tileSet;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			shared_ptr<Tile> tile = tileSet->GetTile(tileIndex);
			tile->SetCollision(oldCollision);
			mainWindow->UpdateTileSetContents(tileSet);
			TileSetView* view = mainWindow->GetTileSetView(tileSet);
			if (view)
			{
				view->GetEditor()->SetTool(CollisionTool);
				view->UpdateToolState();
			}
		},
		[=]() { // Redo
			shared_ptr<Tile> tile = tileSet->GetTile(tileIndex);
			tile->SetCollision(newCollision);
			mainWindow->UpdateTileSetContents(tileSet);
			TileSetView* view = mainWindow->GetTileSetView(tileSet);
			if (view)
			{
				view->GetEditor()->SetTool(CollisionTool);
				view->UpdateToolState();
			}
		});
}


void TileSetEditorWidget::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
		m_mouseDownPaletteEntry = m_leftPaletteEntry;
	else if (event->button() == Qt::RightButton)
		m_mouseDownPaletteEntry = m_rightPaletteEntry;
	else
		return;
	m_mouseDown = true;

	switch (m_tool)
	{
	case SelectTool:
		if (IsMouseInSelection(event))
		{
			m_startX = event->x() / m_zoom;
			m_startY = event->y() / m_zoom;
			m_moveSelection = true;
			BeginMoveSelectionLayer();
		}
		else
		{
			m_startX = event->x() / m_zoom;
			m_startY = event->y() / m_zoom;
			m_selectionContents.reset();
			m_underSelection.reset();
			m_moveSelection = false;
			m_waitForSelection = 6;
			update();
		}
		break;

	case PenTool:
		SetPixelForMouseEvent(event);
		break;

	case RectangleTool:
		m_startX = event->x() / m_zoom;
		m_startY = event->y() / m_zoom;
		UpdateRectangleLayer(event);
		update();
		break;

	case FilledRectangleTool:
		m_startX = event->x() / m_zoom;
		m_startY = event->y() / m_zoom;
		UpdateFilledRectangleLayer(event);
		update();
		break;

	case CircleTool:
		m_startX = event->x() / m_zoom;
		m_startY = event->y() / m_zoom;
		UpdateCircleLayer(event);
		update();
		break;

	case LineTool:
		m_startX = event->x() / m_zoom;
		m_startY = event->y() / m_zoom;
		UpdateLineLayer(event);
		update();
		break;

	case FillTool:
		Fill(event);
		CommitPendingActions();
		break;

	case CollisionTool:
		m_startX = event->x() / m_zoom;
		m_startY = event->y() / m_zoom;
		m_selectionContents.reset();
		m_underSelection.reset();
		m_moveSelection = false;
		m_waitForSelection = 6;
		update();
		break;

	default:
		break;
	}
}


void TileSetEditorWidget::CommitPendingActions()
{
	if ((m_pendingActions.size() != 0) || (m_pendingSelections.size() != 0) || (m_pendingSetColumns.size() != 0))
	{
		vector<EditAction> editActions = m_pendingActions;
		vector<SelectAction> selectActions = m_pendingSelections;
		vector<SetDisplayColumnsAction> setColumnsActions = m_pendingSetColumns;
		m_pendingActions.clear();
		m_pendingSelections.clear();
		m_pendingSetColumns.clear();

		// Merge selection actions
		if (selectActions.size() > 1)
		{
			selectActions[0].newSelectionContents = selectActions[selectActions.size() - 1].newSelectionContents;
			selectActions[0].newUnderSelection = selectActions[selectActions.size() - 1].newUnderSelection;
			selectActions.erase(selectActions.begin() + 1, selectActions.end());
		}

		// Merge column actions
		if (setColumnsActions.size() > 1)
		{
			setColumnsActions[0].newCols = setColumnsActions[setColumnsActions.size() - 1].newCols;
			setColumnsActions.erase(setColumnsActions.begin() + 1, setColumnsActions.end());
		}

		shared_ptr<TileSet> tileSet = m_tileSet;
		MainWindow* mainWindow = m_mainWindow;
		uint16_t frame = m_frame;
		m_mainWindow->AddUndoAction(
			[=]() { // Undo
				set<shared_ptr<Palette>> palettes;
				for (size_t i = 0; i < editActions.size(); i++)
				{
					EditAction action = editActions[editActions.size() - (i + 1)];
					shared_ptr<Tile> tile = tileSet->GetTile(action.tileIndex);
					if (!tile)
						continue;
					tile->GetData(frame)[action.offset] = action.oldData;
					tile->SetPalette(action.oldPalette, action.oldPaletteOffset);
					if (action.oldPalette)
						palettes.insert(action.oldPalette);
					if (action.newPalette)
						palettes.insert(action.newPalette);
				}
				for (auto& i : selectActions)
				{
					TileSetView* view = mainWindow->GetTileSetView(tileSet);
					if (!view)
						break;
					view->GetEditor()->m_selectionContents = i.oldSelectionContents;
					view->GetEditor()->m_underSelection = i.oldUnderSelection;
					if (view->GetEditor()->m_selectionContents)
					{
						view->GetEditor()->m_tool = SelectTool;
						view->GetEditor()->m_showHover = false;
						view->UpdateToolState();
					}
				}
				TileSetView* view = mainWindow->GetTileSetView(tileSet);
				if (view)
					view->GetEditor()->m_frame = frame;
				for (auto& i : setColumnsActions)
					tileSet->SetDisplayColumns(i.oldCols);
				for (auto& i : palettes)
					mainWindow->UpdatePaletteContents(i);
				mainWindow->UpdateTileSetContents(tileSet);
			},
			[=]() { // Redo
				set<shared_ptr<Palette>> palettes;
				for (size_t i = 0; i < editActions.size(); i++)
				{
					EditAction action = editActions[i];
					shared_ptr<Tile> tile = tileSet->GetTile(action.tileIndex);
					if (!tile)
						continue;
					tile->GetData(frame)[action.offset] = action.newData;
					tile->SetPalette(action.newPalette, action.newPaletteOffset);
					if (action.oldPalette)
						palettes.insert(action.oldPalette);
					if (action.newPalette)
						palettes.insert(action.newPalette);
				}
				for (auto& i : selectActions)
				{
					TileSetView* view = mainWindow->GetTileSetView(tileSet);
					if (!view)
						break;
					view->GetEditor()->m_selectionContents = i.newSelectionContents;
					view->GetEditor()->m_underSelection = i.newUnderSelection;
					if (view->GetEditor()->m_selectionContents)
					{
						view->GetEditor()->m_tool = SelectTool;
						view->GetEditor()->m_showHover = false;
						view->UpdateToolState();
					}
				}
				TileSetView* view = mainWindow->GetTileSetView(tileSet);
				if (view)
					view->GetEditor()->m_frame = frame;
				for (auto& i : setColumnsActions)
					tileSet->SetDisplayColumns(i.newCols);
				for (auto& i : palettes)
					mainWindow->UpdatePaletteContents(i);
				mainWindow->UpdateTileSetContents(tileSet);
			});
	}
}


void TileSetEditorWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if (!m_mouseDown)
		return;

	m_mouseDown = false;

	switch (m_tool)
	{
	case SelectTool:
		if (m_waitForSelection > 0)
			break;
		if (m_moveSelection)
		{
			MoveSelectionLayer(event);
			FinishMoveSelectionLayer();
		}
		else
		{
			UpdateSelectionLayer(event);
			update();
		}
		break;

	case RectangleTool:
		UpdateRectangleLayer(event);
		ApplyLayer(m_floatingLayer);
		m_floatingLayer.reset();
		break;

	case FilledRectangleTool:
		UpdateFilledRectangleLayer(event);
		ApplyLayer(m_floatingLayer);
		m_floatingLayer.reset();
		break;

	case CircleTool:
		UpdateCircleLayer(event);
		ApplyLayer(m_floatingLayer);
		m_floatingLayer.reset();
		break;

	case LineTool:
		UpdateLineLayer(event);
		ApplyLayer(m_floatingLayer);
		m_floatingLayer.reset();
		break;

	case CollisionTool:
		if (m_waitForSelection > 0)
		{
			if (!(event->modifiers() & Qt::ShiftModifier))
				RemoveSingleCollision();
			update();
			break;
		}
		UpdateSelectionLayer(event);
		if (event->modifiers() & Qt::ShiftModifier)
			AddSelectionAsCollision();
		else
			SetSelectionAsCollision();
		update();
		break;

	default:
		break;
	}

	CommitPendingActions();
}


void TileSetEditorWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (m_tool != SelectTool)
		m_showHover = true;

	m_hoverX = event->x() / m_zoom;
	m_hoverY = event->y() / m_zoom;

	if (m_mouseDown)
	{
		switch (m_tool)
		{
		case SelectTool:
			if (m_waitForSelection > 0)
			{
				m_waitForSelection--;
				break;
			}
			if (m_moveSelection)
				MoveSelectionLayer(event);
			else
				UpdateSelectionLayer(event);
			break;

		case PenTool:
			SetPixelForMouseEvent(event);
			break;

		case RectangleTool:
			UpdateRectangleLayer(event);
			break;

		case FilledRectangleTool:
			UpdateFilledRectangleLayer(event);
			break;

		case CircleTool:
			UpdateCircleLayer(event);
			break;

		case LineTool:
			UpdateLineLayer(event);
			break;

		case CollisionTool:
			if (m_waitForSelection > 0)
			{
				m_waitForSelection--;
				break;
			}
			UpdateSelectionLayer(event);
			break;

		default:
			break;
		}
	}

	update();
}


void TileSetEditorWidget::leaveEvent(QEvent*)
{
	m_showHover = false;
	update();
}


void TileSetEditorWidget::SetTool(EditorTool tool)
{
	m_tool = tool;

	if (m_selectionContents)
	{
		SelectAction action;
		action.oldSelectionContents = m_selectionContents;
		action.oldUnderSelection = m_underSelection;
		m_pendingSelections.push_back(action);
		CommitPendingActions();

		m_selectionContents.reset();
		m_underSelection.reset();
	}

	update();
}


void TileSetEditorWidget::FlipHorizontal()
{
	shared_ptr<TileSetFloatingLayer> layer = m_selectionContents;
	if (!layer)
	{
		if (m_tileSet->GetTileCount() != (m_rows * m_columns))
		{
			QMessageBox::critical(this, "Error", "This tile set is not rectangular. To flip, either "
				"select the region to flip or adjust the tiles per row so that the tile set is "
				"rectangular.");
			return;
		}

		layer = make_shared<TileSetFloatingLayer>(0, 0, m_columns * m_tileSet->GetWidth(),
			m_rows * m_tileSet->GetHeight());
		CaptureLayer(layer);
	}

	shared_ptr<TileSetFloatingLayer> newLayer = make_shared<TileSetFloatingLayer>(layer->GetX(), layer->GetY(),
		layer->GetWidth(), layer->GetHeight());
	for (int y = 0; y < layer->GetHeight(); y++)
		for (int x = 0; x < layer->GetWidth(); x++)
			newLayer->SetPixel(x, y, layer->GetPixel((layer->GetWidth() - 1) - x, y));

	ApplyLayer(newLayer);

	if (layer == m_selectionContents)
	{
		SelectAction action;
		action.oldSelectionContents = m_selectionContents;
		action.oldUnderSelection = m_underSelection;
		action.newSelectionContents = newLayer;
		action.newUnderSelection = m_underSelection;
		m_pendingSelections.push_back(action);

		m_selectionContents = newLayer;
	}

	CommitPendingActions();
}


void TileSetEditorWidget::FlipVertical()
{
	shared_ptr<TileSetFloatingLayer> layer = m_selectionContents;
	if (!layer)
	{
		if (m_tileSet->GetTileCount() != (m_rows * m_columns))
		{
			QMessageBox::critical(this, "Error", "This tile set is not rectangular. To flip, either "
				"select the region to flip or adjust the tiles per row so that the tile set is "
				"rectangular.");
			return;
		}

		layer = make_shared<TileSetFloatingLayer>(0, 0, m_columns * m_tileSet->GetWidth(),
			m_rows * m_tileSet->GetHeight());
		CaptureLayer(layer);
	}

	shared_ptr<TileSetFloatingLayer> newLayer = make_shared<TileSetFloatingLayer>(layer->GetX(), layer->GetY(),
		layer->GetWidth(), layer->GetHeight());
	for (int y = 0; y < layer->GetHeight(); y++)
		for (int x = 0; x < layer->GetWidth(); x++)
			newLayer->SetPixel(x, y, layer->GetPixel(x, (layer->GetHeight() - 1) - y));

	ApplyLayer(newLayer);

	if (layer == m_selectionContents)
	{
		SelectAction action;
		action.oldSelectionContents = m_selectionContents;
		action.oldUnderSelection = m_underSelection;
		action.newSelectionContents = newLayer;
		action.newUnderSelection = m_underSelection;
		m_pendingSelections.push_back(action);

		m_selectionContents = newLayer;
	}

	CommitPendingActions();
}


void TileSetEditorWidget::Rotate()
{
	int oldCols = m_tileSet->GetDisplayColumns();
	int newCols = oldCols;

	shared_ptr<TileSetFloatingLayer> layer = m_selectionContents;
	if (!layer)
	{
		if (m_tileSet->GetTileCount() != (m_rows * m_columns))
		{
			QMessageBox::critical(this, "Error", "This tile set is not rectangular. To rotate, either "
				"select the region to rotate or adjust the tiles per row so that the tile set is "
				"rectangular.");
			return;
		}

		layer = make_shared<TileSetFloatingLayer>(0, 0, m_columns * m_tileSet->GetWidth(),
			m_rows * m_tileSet->GetHeight());
		CaptureLayer(layer);

		// Also rotate displayed area
		int cols = m_columns;
		m_columns = m_rows;
		m_rows = cols;

		newCols = m_columns;
	}

	shared_ptr<TileSetFloatingLayer> newLayer = make_shared<TileSetFloatingLayer>(layer->GetX(), layer->GetY(),
		layer->GetHeight(), layer->GetWidth());
	for (int y = 0; y < layer->GetWidth(); y++)
		for (int x = 0; x < layer->GetHeight(); x++)
			newLayer->SetPixel(x, y, layer->GetPixel(y, (layer->GetHeight() - 1) - x));

	if (layer == m_selectionContents)
	{
		// For selections, reposition by rotating around the center
		int centerX = layer->GetX() + (layer->GetWidth() / 2);
		int centerY = layer->GetY() + (layer->GetHeight() / 2);

		int posX = centerX - (newLayer->GetWidth() / 2);
		int posY = centerY - (newLayer->GetHeight() / 2);
		if ((posX + newLayer->GetWidth()) > (int)(m_columns * m_tileSet->GetWidth()))
			posX = (m_columns * m_tileSet->GetWidth()) - newLayer->GetWidth();
		if ((posY + newLayer->GetHeight()) > (int)(m_rows * m_tileSet->GetHeight()))
			posY = (m_rows * m_tileSet->GetHeight()) - newLayer->GetHeight();
		if (posX < 0)
			posX = 0;
		if (posY < 0)
			posY = 0;

		SelectAction action;
		action.oldSelectionContents = m_selectionContents;
		action.oldUnderSelection = m_underSelection;
		action.newSelectionContents = newLayer;

		ApplyLayer(m_underSelection);
		m_underSelection = make_shared<TileSetFloatingLayer>(posX, posY, newLayer->GetWidth(), newLayer->GetHeight());
		newLayer->Move(posX, posY);
		CaptureLayer(m_underSelection);

		action.newUnderSelection = m_underSelection;
		m_pendingSelections.push_back(action);
	}

	ApplyLayer(newLayer);

	if (oldCols != newCols)
	{
		SetDisplayColumnsAction colAction;
		colAction.oldCols = oldCols;
		colAction.newCols = newCols;
		m_pendingSetColumns.push_back(colAction);

		m_tileSet->SetDisplayColumns(newCols);
	}

	CommitPendingActions();

	if (layer == m_selectionContents)
		m_selectionContents = newLayer;

	m_mainWindow->UpdateTileSetContents(m_tileSet);
}


void TileSetEditorWidget::ZoomIn()
{
	if (m_zoom < 32)
	{
		m_zoom *= 2;
		UpdateView();
	}
}


void TileSetEditorWidget::ZoomOut()
{
	if (m_zoom > 1)
	{
		m_zoom /= 2;
		UpdateView();
	}
}


bool TileSetEditorWidget::Cut()
{
	if (!m_selectionContents)
		return false;
	if (!Copy())
		return false;

	SelectAction action;
	action.oldSelectionContents = m_selectionContents;
	action.oldUnderSelection = m_underSelection;
	m_pendingSelections.push_back(action);

	ApplyLayer(m_underSelection);
	m_selectionContents.reset();
	m_underSelection.reset();
	CommitPendingActions();
	return true;
}


bool TileSetEditorWidget::Copy()
{
	if (!m_selectionContents)
		return false;

	Json::Value data(Json::objectValue);

	Json::Value image(Json::objectValue);
	image["width"] = m_selectionContents->GetWidth();
	image["height"] = m_selectionContents->GetHeight();

	map<shared_ptr<Palette>, int> paletteIndex;
	Json::Value palettes(Json::arrayValue);
	int curPaletteIndex = 1;
	for (int y = 0; y < m_selectionContents->GetHeight(); y++)
	{
		for (int x = 0; x < m_selectionContents->GetWidth(); x++)
		{
			TileSetFloatingLayerPixel pixel = m_selectionContents->GetPixel(x, y);
			if (!pixel.valid)
				continue;
			auto i = paletteIndex.find(pixel.palette);
			if (i == paletteIndex.end())
			{
				paletteIndex[pixel.palette] = curPaletteIndex++;
				if (pixel.palette)
					palettes.append(pixel.palette->GetId());
				else
					palettes.append("");
			}
		}
	}
	image["palettes"] = palettes;

	string imageData, imagePalette;
	for (int y = 0; y < m_selectionContents->GetHeight(); y++)
	{
		for (int x = 0; x < m_selectionContents->GetWidth(); x++)
		{
			TileSetFloatingLayerPixel pixel = m_selectionContents->GetPixel(x, y);
			if (!pixel.valid)
			{
				imageData += "00";
				imagePalette += "00";
				continue;
			}

			char dataStr[32];
			sprintf(dataStr, "%.2x", (uint8_t)pixel.entry);
			imageData += dataStr;
			sprintf(dataStr, "%.2x", (uint8_t)paletteIndex[pixel.palette]);
			imagePalette += dataStr;
		}
	}
	image["pixel_data"] = imageData;
	image["palette_data"] = imagePalette;

	data["image"] = image;

	Json::StyledWriter writer;
	string contents = writer.write(data);

	QClipboard* clipboard = QGuiApplication::clipboard();
	clipboard->setText(QString::fromStdString(contents));
	return true;
}


bool TileSetEditorWidget::Paste()
{
	QClipboard* clipboard = QGuiApplication::clipboard();
	QString text = clipboard->text();

	Json::Reader reader;
	Json::Value jsonData;
	if (!reader.parse(text.toStdString(), jsonData, false))
		return false;

	try
	{
		const Json::Value& image = jsonData["image"];
		int width = image["width"].asInt();
		int height = image["height"].asInt();

		vector<shared_ptr<Palette>> palettes;
		palettes.push_back(shared_ptr<Palette>());
		for (auto& i : image["palettes"])
		{
			string id = i.asString();
			if (id.size() == 0)
				palettes.push_back(shared_ptr<Palette>());
			else
				palettes.push_back(m_project->GetPaletteById(id));
		}

		string pixelData = image["pixel_data"].asString();
		string paletteData = image["palette_data"].asString();
		if (pixelData.size() != (size_t)(width * height * 2))
			return false;
		if (paletteData.size() != (size_t)(width * height * 2))
			return false;
		if ((width <= 0) || (height <= 0) || (width >= 0x8000) || (height >= 0x8000))
			return false;

		int centerX = (m_scrollArea->viewport()->rect().center().x() - geometry().x()) / m_zoom;
		int centerY = (m_scrollArea->viewport()->rect().center().y() - geometry().y()) / m_zoom;

		int posX = centerX - (width / 2);
		int posY = centerY - (height / 2);
		if ((posX + width) > (int)(m_columns * m_tileSet->GetWidth()))
			posX = (m_columns * m_tileSet->GetWidth()) - width;
		if ((posY + height) > (int)(m_rows * m_tileSet->GetHeight()))
			posY = (m_rows * m_tileSet->GetHeight()) - height;
		if (posX < 0)
			posX = 0;
		if (posY < 0)
			posY = 0;

		shared_ptr<TileSetFloatingLayer> selectionContents = make_shared<TileSetFloatingLayer>(posX, posY, width, height);
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				uint8_t paletteIndex = (uint8_t)strtoul(
					paletteData.substr((y * width * 2) + (x * 2), 2).c_str(), nullptr, 16);
				uint8_t pixel = (uint8_t)strtoul(
					pixelData.substr((y * width * 2) + (x * 2), 2).c_str(), nullptr, 16);

				if ((size_t)paletteIndex >= palettes.size())
				{
					paletteIndex = 0;
					pixel = 0;
				}

				shared_ptr<Palette> palette = palettes[paletteIndex];
				if (palette && ((size_t)pixel >= palette->GetEntryCount()))
				{
					palette.reset();
					pixel = 0;
				}

				selectionContents->SetPixel(x, y, palette, pixel);
			}
		}

		SelectAction action;
		action.oldSelectionContents = m_selectionContents;
		action.oldUnderSelection = m_underSelection;

		m_selectionContents = selectionContents;
		m_underSelection = make_shared<TileSetFloatingLayer>(posX, posY, width, height);

		action.newSelectionContents = selectionContents;
		action.newUnderSelection = m_underSelection;
		m_pendingSelections.push_back(action);

		m_tool = SelectTool;
		m_showHover = false;
		CaptureLayer(m_underSelection);
		ApplyLayer(m_selectionContents);
		CommitPendingActions();
		return true;
	}
	catch (exception&)
	{
		return false;
	}
}


void TileSetEditorWidget::SelectAll()
{
	m_tool = SelectTool;
	m_showHover = false;

	SelectAction action;
	action.oldSelectionContents = m_selectionContents;
	action.oldUnderSelection = m_underSelection;

	int width = (int)(m_columns * m_tileSet->GetWidth());
	int height = (int)(m_rows * m_tileSet->GetHeight());

	m_selectionContents = make_shared<TileSetFloatingLayer>(0, 0, width, height);
	m_underSelection = make_shared<TileSetFloatingLayer>(0, 0, width, height);

	action.newSelectionContents = m_selectionContents;
	action.newUnderSelection = m_underSelection;
	m_pendingSelections.push_back(action);

	CaptureLayer(m_selectionContents);

	// If selection is moved, replace with transparent
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
			m_underSelection->SetPixel(x, y, shared_ptr<Palette>(), 0);

	CommitPendingActions();
	update();
}


void TileSetEditorWidget::RemoveCollisions()
{
	m_tool = CollisionTool;

	vector<CollisionUpdateAction> actions;
	for (size_t i = 0; i < m_tileSet->GetTileCount(); i++)
	{
		shared_ptr<Tile> tile = m_tileSet->GetTile(i);
		if (!tile)
			continue;

		CollisionUpdateAction action;
		action.tileIndex = i;
		action.oldCollision = tile->GetCollision();
		tile->SetCollision(action.newCollision);
		actions.push_back(action);
	}

	m_mainWindow->UpdateTileSetContents(m_tileSet);

	MainWindow* mainWindow = m_mainWindow;
	shared_ptr<TileSet> tileSet = m_tileSet;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			for (auto& i : actions)
				tileSet->GetTile(i.tileIndex)->SetCollision(i.oldCollision);
			mainWindow->UpdateTileSetContents(tileSet);
			TileSetView* view = mainWindow->GetTileSetView(tileSet);
			if (view)
			{
				view->GetEditor()->SetTool(CollisionTool);
				view->UpdateToolState();
			}
		},
		[=]() { // Redo
			for (auto& i : actions)
				tileSet->GetTile(i.tileIndex)->SetCollision(i.newCollision);
			mainWindow->UpdateTileSetContents(tileSet);
			TileSetView* view = mainWindow->GetTileSetView(tileSet);
			if (view)
			{
				view->GetEditor()->SetTool(CollisionTool);
				view->UpdateToolState();
			}
		});
}


void TileSetEditorWidget::CollideWithAll()
{
	m_tool = CollisionTool;

	BoundingRect rect;
	rect.x = 0;
	rect.y = 0;
	rect.width = m_tileSet->GetWidth();
	rect.height = m_tileSet->GetHeight();

	vector<CollisionUpdateAction> actions;
	for (size_t i = 0; i < m_tileSet->GetTileCount(); i++)
	{
		shared_ptr<Tile> tile = m_tileSet->GetTile(i);
		if (!tile)
			continue;

		CollisionUpdateAction action;
		action.tileIndex = i;
		action.oldCollision = tile->GetCollision();
		action.newCollision.push_back(rect);
		tile->SetCollision(action.newCollision);
		actions.push_back(action);
	}

	m_mainWindow->UpdateTileSetContents(m_tileSet);

	MainWindow* mainWindow = m_mainWindow;
	shared_ptr<TileSet> tileSet = m_tileSet;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			for (auto& i : actions)
				tileSet->GetTile(i.tileIndex)->SetCollision(i.oldCollision);
			mainWindow->UpdateTileSetContents(tileSet);
			TileSetView* view = mainWindow->GetTileSetView(tileSet);
			if (view)
			{
				view->GetEditor()->SetTool(CollisionTool);
				view->UpdateToolState();
			}
		},
		[=]() { // Redo
			for (auto& i : actions)
				tileSet->GetTile(i.tileIndex)->SetCollision(i.newCollision);
			mainWindow->UpdateTileSetContents(tileSet);
			TileSetView* view = mainWindow->GetTileSetView(tileSet);
			if (view)
			{
				view->GetEditor()->SetTool(CollisionTool);
				view->UpdateToolState();
			}
		});
}
