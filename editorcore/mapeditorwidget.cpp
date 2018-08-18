#include <QVBoxLayout>
#include <QPainter>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QScrollBar>
#include <QGuiApplication>
#include <QClipboard>
#include <set>
#include <queue>
#include "mapeditorwidget.h"
#include "mapview.h"
#include "effectlayerview.h"
#include "theme.h"
#include "mainwindow.h"

using namespace std;


MapEditorWidget::MapEditorWidget(QWidget* parent, MainWindow* mainWindow, shared_ptr<Project> project,
	shared_ptr<Map> map, bool effectLayer):
	QAbstractScrollArea(parent), m_mainWindow(mainWindow), m_project(project), m_map(map)
{
	m_zoom = 2;
	m_mouseDown = false;
	m_showHover = false;
	m_image = nullptr;
	m_layerWidget = nullptr;
	m_tileWidget = nullptr;
	m_effectLayerEditor = effectLayer;
	m_fadeOtherLayers = false;
	m_animate = false;
	m_tool = PenTool;

	m_layer = m_map->GetMainLayer();

	m_animTimer = new QTimer(this);
	m_animTimer->setSingleShot(false);
	m_animTimer->setInterval(1000 / 60);
	connect(m_animTimer, &QTimer::timeout, this, &MapEditorWidget::OnAnimationTimer);

	setMouseTracking(true);
}


void MapEditorWidget::UpdateView()
{
	int renderWidth = (viewport()->size().width() + (m_zoom - 1)) / m_zoom;
	int renderHeight = (viewport()->size().height() + (m_zoom - 1)) / m_zoom;
	if ((!m_image) || (renderWidth != m_renderWidth) || (renderHeight != m_renderHeight))
	{
		if (m_image)
			delete m_image;
		m_renderWidth = renderWidth;
		m_renderHeight = renderHeight;
		m_image = new QImage(m_renderWidth, m_renderHeight, QImage::Format_RGB555);
	}

	m_renderer = make_shared<Renderer>(m_map, (uint16_t)m_renderWidth, (uint16_t)m_renderHeight);
	m_renderer->SetActiveLayer(m_layer);
	if (m_floatingLayer)
		m_renderer->SetFloatingLayer(m_floatingLayer);
	if (m_effectLayerEditor)
	{
		m_renderer->SetBackgroundColor(Palette::FromRGB(6, 6, 6));
		m_renderer->SetParallaxEnabled(false);
	}
	m_renderer->SetLayerVisibility(m_visibility);

	horizontalScrollBar()->setPageStep(viewport()->size().width() / m_zoom);
	horizontalScrollBar()->setSingleStep(4);
	horizontalScrollBar()->setRange(0, m_map->GetMainLayer()->GetWidth() *
		m_map->GetMainLayer()->GetTileWidth() - (m_renderWidth - 1));
	verticalScrollBar()->setPageStep(viewport()->size().height() / m_zoom);
	verticalScrollBar()->setSingleStep(4);
	verticalScrollBar()->setRange(0, m_map->GetMainLayer()->GetHeight() *
		m_map->GetMainLayer()->GetTileHeight() - (m_renderHeight - 1));
	viewport()->update();
}


shared_ptr<MapLayer> MapEditorWidget::GetActiveLayer() const
{
	return m_layer;
}


void MapEditorWidget::SetActiveLayer(shared_ptr<MapLayer> layer)
{
	m_layer = layer;

	if (m_leftTileSet && ((m_leftTileSet->GetWidth() != m_layer->GetTileWidth()) ||
		(m_leftTileSet->GetHeight() != m_layer->GetTileHeight()) ||
		(m_leftTileSet->GetDepth() != m_layer->GetTileDepth())))
		m_leftTileSet.reset();

	if (m_rightTileSet && ((m_rightTileSet->GetWidth() != m_layer->GetTileWidth()) ||
		(m_rightTileSet->GetHeight() != m_layer->GetTileHeight()) ||
		(m_rightTileSet->GetDepth() != m_layer->GetTileDepth())))
		m_rightTileSet.reset();

	m_selectionContents.reset();
	m_underSelection.reset();

	UpdateView();

	if (m_layerWidget)
		m_layerWidget->UpdateView();
}


shared_ptr<TileSet> MapEditorWidget::GetSelectedLeftTileSet() const
{
	return m_leftTileSet;
}


size_t MapEditorWidget::GetSelectedLeftTileIndex() const
{
	return m_leftTileIndex;
}


shared_ptr<TileSet> MapEditorWidget::GetSelectedRightTileSet() const
{
	return m_rightTileSet;
}


size_t MapEditorWidget::GetSelectedRightTileIndex() const
{
	return m_rightTileIndex;
}


void MapEditorWidget::SetSelectedLeftTile(std::shared_ptr<TileSet> tileSet, size_t entry)
{
	m_leftTileSet = tileSet;
	m_leftTileIndex = entry;
	if (m_tileWidget)
		m_tileWidget->UpdateView();
}


void MapEditorWidget::SetSelectedRightTile(std::shared_ptr<TileSet> tileSet, size_t entry)
{
	m_rightTileSet = tileSet;
	m_rightTileIndex = entry;
	if (m_tileWidget)
		m_tileWidget->UpdateView();
}


void MapEditorWidget::SetAnimationEnabled(bool enabled)
{
	if (m_animate == enabled)
		return;
	m_animate = enabled;
	if (m_renderer)
		m_renderer->ResetAnimation();
	if (m_animate)
		m_animTimer->start();
	else
		m_animTimer->stop();
	viewport()->update();
}


bool MapEditorWidget::IsLayerVisible(shared_ptr<MapLayer> layer)
{
	auto i = m_visibility.find(layer);
	if (i == m_visibility.end())
		return true;
	return i->second;
}


void MapEditorWidget::SetLayerVisibility(std::shared_ptr<MapLayer> layer, bool visible)
{
	m_visibility[layer] = visible;
	UpdateView();
}


void MapEditorWidget::paintEvent(QPaintEvent*)
{
	QPainter p(viewport());
	if (!m_image)
		return;
	if (!m_renderer)
		return;

	int scrollX = horizontalScrollBar()->value();
	int scrollY = verticalScrollBar()->value();
	m_renderer->SetScroll((int16_t)scrollX, (int16_t)scrollY);

	if (m_animate)
		m_renderer->TickAnimation();

	if (m_showHover)
	{
		m_renderer->Render();
		if (m_fadeOtherLayers)
		{
			m_renderer->RenderSingleLayer();
			for (int y = 0; y < m_renderHeight; y++)
			{
				uint16_t* dest = (uint16_t*)m_image->scanLine(y);
				const uint16_t* allLayersSrc = m_renderer->GetPixelDataForRow(y);
				const uint16_t* curLayerSrc = m_renderer->GetSingleLayerPixelDataForRow(y);
				for (int x = 0; x < m_renderWidth; x++)
					dest[x] = Palette::BlendColor(allLayersSrc[x], curLayerSrc[x], 5);
			}
		}
		else
		{
			for (int y = 0; y < m_renderHeight; y++)
				memcpy(m_image->scanLine(y), m_renderer->GetPixelDataForRow(y), m_renderWidth * 2);
		}
		p.drawImage(QRect(0, 0, m_renderWidth * m_zoom, m_renderHeight * m_zoom), *m_image);
	}
	else
	{
		m_renderer->Render();
		for (int y = 0; y < m_renderHeight; y++)
			memcpy(m_image->scanLine(y), m_renderer->GetPixelDataForRow(y), m_renderWidth * 2);
		p.drawImage(QRect(0, 0, m_renderWidth * m_zoom, m_renderHeight * m_zoom), *m_image);
	}

	if ((m_tool == SelectTool) && m_floatingLayer)
	{
		// Moving selection, draw border of floating selection
		p.setPen(QPen(QBrush(Theme::backgroundDark), 1));
		p.setBrush(Qt::NoBrush);
		p.drawRect((m_floatingLayer->GetX() * m_layer->GetTileWidth() - scrollX) * m_zoom,
			(m_floatingLayer->GetY() * m_layer->GetTileHeight() - scrollY) * m_zoom,
			m_floatingLayer->GetWidth() * m_layer->GetTileWidth() * m_zoom,
			m_floatingLayer->GetHeight() * m_layer->GetTileHeight() * m_zoom);
		p.setPen(QPen(QBrush(Theme::content), 1, Qt::DashLine));
		p.drawRect((m_floatingLayer->GetX() * m_layer->GetTileWidth() - scrollX) * m_zoom,
			(m_floatingLayer->GetY() * m_layer->GetTileHeight() - scrollY) * m_zoom,
			m_floatingLayer->GetWidth() * m_layer->GetTileWidth() * m_zoom,
			m_floatingLayer->GetHeight() * m_layer->GetTileHeight() * m_zoom);
	}
	else if (m_selectionContents)
	{
		// Selection active, draw border of selection
		p.setPen(QPen(QBrush(Theme::backgroundDark), 1));
		p.setBrush(Qt::NoBrush);
		p.drawRect((m_selectionContents->GetX() * m_layer->GetTileWidth() - scrollX) * m_zoom,
			(m_selectionContents->GetY() * m_layer->GetTileHeight() - scrollY) * m_zoom,
			m_selectionContents->GetWidth() * m_layer->GetTileWidth() * m_zoom,
			m_selectionContents->GetHeight() * m_layer->GetTileHeight() * m_zoom);
		p.setPen(QPen(QBrush(Theme::content), 1, Qt::DashLine));
		p.drawRect((m_selectionContents->GetX() * m_layer->GetTileWidth() - scrollX) * m_zoom,
			(m_selectionContents->GetY() * m_layer->GetTileHeight() - scrollY) * m_zoom,
			m_selectionContents->GetWidth() * m_layer->GetTileWidth() * m_zoom,
			m_selectionContents->GetHeight() * m_layer->GetTileHeight() * m_zoom);
	}

	if (m_showHover)
	{
		int tileX = m_hoverX / m_layer->GetTileWidth();
		int tileY = m_hoverY / m_layer->GetTileHeight();
		if ((tileX >= 0) && (tileY >= 0) && (tileX < (int)m_layer->GetWidth()) && (tileY < (int)m_layer->GetHeight()))
		{
			p.setPen(QPen(QBrush(Theme::blue), 2));
			p.setBrush(Qt::NoBrush);
			p.drawRect((tileX * m_layer->GetTileWidth() - scrollX) * m_zoom - 1,
				(tileY * m_layer->GetTileHeight() - scrollY) * m_zoom - 1,
				m_layer->GetTileWidth() * m_zoom + 2, m_layer->GetTileHeight() * m_zoom + 2);
		}
	}
}


MapFloatingLayerTile MapEditorWidget::GetTile(int x, int y)
{
	MapFloatingLayerTile result;
	if ((x >= 0) && (y >= 0) && (x < (int)m_layer->GetWidth()) && (y < (int)m_layer->GetHeight()))
	{
		result.valid = true;
		TileReference ref = m_layer->GetTileAt(x, y);
		result.tileSet = ref.tileSet;
		result.index = ref.index;
		return result;
	}
	result.valid = false;
	return result;
}


bool MapEditorWidget::SetTile(int x, int y, shared_ptr<TileSet> tileSet, uint16_t index)
{
	if ((x >= 0) && (y >= 0) && (x < (int)m_layer->GetWidth()) && (y < (int)m_layer->GetHeight()))
	{
		EditAction action;
		action.layer = m_layer;
		action.x = x;
		action.y = y;

		TileReference oldRef = m_layer->GetTileAt(x, y);
		action.oldTileSet = oldRef.tileSet;
		action.oldTileIndex = oldRef.index;

		TileReference ref;
		ref.tileSet = tileSet;
		ref.index = index;
		action.newTileSet = ref.tileSet;
		action.newTileIndex = ref.index;

		if ((action.oldTileSet == action.newTileSet) && (action.oldTileIndex == action.newTileIndex))
			return false;

		m_layer->SetTileAt(x, y, ref);
		m_pendingActions.push_back(action);
		return true;
	}
	return false;
}


void MapEditorWidget::SetTileForMouseEvent(QMouseEvent* event)
{
	int x = (event->x() / m_zoom) + horizontalScrollBar()->value();
	int y = (event->y() / m_zoom) + verticalScrollBar()->value();
	int tileX = x / m_layer->GetTileWidth();
	int tileY = y / m_layer->GetTileHeight();

	if (SetTile(tileX, tileY, m_mouseDownTileSet, m_mouseDownTileIndex))
	{
		m_layer->UpdateRegionForSmartTiles(tileX, tileY, 1, 1);
		UpdateView();
	}
}


void MapEditorWidget::CaptureLayer(shared_ptr<MapFloatingLayer> layer)
{
	for (int y = 0; y < layer->GetHeight(); y++)
		for (int x = 0; x < layer->GetWidth(); x++)
			layer->SetTile(x, y, GetTile(x + layer->GetX(), y + layer->GetY()));
}


void MapEditorWidget::ApplyLayer(shared_ptr<MapFloatingLayer> layer)
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

			MapFloatingLayerTile tile = layer->GetTile(x, y);
			if (!tile.valid)
				continue;

			SetTile(absX, absY, tile.tileSet, tile.index);
		}
	}

	m_layer->UpdateRegionForSmartTiles(layer->GetX(), layer->GetY(), layer->GetWidth(), layer->GetHeight());
	UpdateView();
}


void MapEditorWidget::UpdateSelectionLayer(QMouseEvent* event)
{
	int startX = m_startX;
	int startY = m_startY;
	int curX = ((event->x() / m_zoom) + horizontalScrollBar()->value()) / m_layer->GetTileWidth();
	int curY = ((event->y() / m_zoom) + verticalScrollBar()->value()) / m_layer->GetTileHeight();

	if ((m_layer->GetWidth() == 0) || (m_layer->GetHeight() == 0))
		return;

	if (startX < 0)
		startX = 0;
	if (startX >= (int)m_layer->GetWidth())
		startX = (int)m_layer->GetWidth() - 1;
	if (startY < 0)
		startY = 0;
	if (startY >= (int)m_layer->GetHeight())
		startY = (int)m_layer->GetHeight() - 1;
	if (curX < 0)
		curX = 0;
	if (curX >= (int)m_layer->GetWidth())
		curX = (int)m_layer->GetWidth() - 1;
	if (curY < 0)
		curY = 0;
	if (curY >= (int)m_layer->GetHeight())
		curY = (int)m_layer->GetHeight() - 1;

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

	m_selectionContents = make_shared<MapFloatingLayer>(m_layer, leftX, topY, width, height);
	m_underSelection = make_shared<MapFloatingLayer>(m_layer, leftX, topY, width, height);

	action.newSelectionContents = m_selectionContents;
	action.newUnderSelection = m_underSelection;
	m_pendingSelections.push_back(action);

	CaptureLayer(m_selectionContents);

	// If selection is moved, replace with transparent
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
			m_underSelection->SetTile(x, y, shared_ptr<TileSet>(), 0);
}


bool MapEditorWidget::IsMouseInSelection(QMouseEvent* event)
{
	if (!m_selectionContents)
		return false;

	int curX = ((event->x() / m_zoom) + horizontalScrollBar()->value()) / m_layer->GetTileWidth();
	int curY = ((event->y() / m_zoom) + verticalScrollBar()->value()) / m_layer->GetTileHeight();

	if ((curX >= m_selectionContents->GetX()) && (curY >= m_selectionContents->GetY()) &&
		(curX < (m_selectionContents->GetX() + m_selectionContents->GetWidth())) &&
		(curY < (m_selectionContents->GetY() + m_selectionContents->GetHeight())))
		return true;
	return false;
}


void MapEditorWidget::BeginMoveSelectionLayer()
{
	ApplyLayer(m_underSelection);
	m_floatingLayer = make_shared<MapFloatingLayer>(*m_selectionContents);
	UpdateView();
}


void MapEditorWidget::MoveSelectionLayer(QMouseEvent* event)
{
	if (!m_floatingLayer)
		return;

	int curX = ((event->x() / m_zoom) + horizontalScrollBar()->value()) / m_layer->GetTileWidth();
	int curY = ((event->y() / m_zoom) + verticalScrollBar()->value()) / m_layer->GetTileHeight();
	int deltaX = curX - m_startX;
	int deltaY = curY - m_startY;
	m_startX = curX;
	m_startY = curY;

	if ((deltaX == 0) && (deltaY == 0))
		return;

	m_floatingLayer->Move(m_floatingLayer->GetX() + deltaX, m_floatingLayer->GetY() + deltaY);
	UpdateView();
}


void MapEditorWidget::FinishMoveSelectionLayer()
{
	SelectAction action;
	action.oldSelectionContents = m_selectionContents;
	action.oldUnderSelection = m_underSelection;
	action.newSelectionContents = m_floatingLayer;

	m_underSelection = make_shared<MapFloatingLayer>(m_layer, m_floatingLayer->GetX(), m_floatingLayer->GetY(),
		m_floatingLayer->GetWidth(), m_floatingLayer->GetHeight());
	CaptureLayer(m_underSelection);

	action.newUnderSelection = m_underSelection;
	m_pendingSelections.push_back(action);

	m_selectionContents = m_floatingLayer;
	ApplyLayer(m_selectionContents);
	m_floatingLayer.reset();
}


void MapEditorWidget::UpdateRectangleLayer(QMouseEvent* event)
{
	int curX = ((event->x() / m_zoom) + horizontalScrollBar()->value()) / m_layer->GetTileWidth();
	int curY = ((event->y() / m_zoom) + verticalScrollBar()->value()) / m_layer->GetTileHeight();

	int leftX = (m_startX < curX) ? m_startX : curX;
	int rightX = (m_startX < curX) ? curX : m_startX;
	int topY = (m_startY < curY) ? m_startY : curY;
	int botY = (m_startY < curY) ? curY : m_startY;

	int width = (rightX - leftX) + 1;
	int height = (botY - topY) + 1;

	m_floatingLayer = make_shared<MapFloatingLayer>(m_layer, leftX, topY, width, height);
	for (int y = 0; y < height; y++)
	{
		m_floatingLayer->SetTile(0, y, m_mouseDownTileSet, m_mouseDownTileIndex);
		m_floatingLayer->SetTile(width - 1, y, m_mouseDownTileSet, m_mouseDownTileIndex);
	}
	for (int x = 0; x < width; x++)
	{
		m_floatingLayer->SetTile(x, 0, m_mouseDownTileSet, m_mouseDownTileIndex);
		m_floatingLayer->SetTile(x, height - 1, m_mouseDownTileSet, m_mouseDownTileIndex);
	}

	UpdateView();
}


void MapEditorWidget::UpdateFilledRectangleLayer(QMouseEvent* event)
{
	int curX = ((event->x() / m_zoom) + horizontalScrollBar()->value()) / m_layer->GetTileWidth();
	int curY = ((event->y() / m_zoom) + verticalScrollBar()->value()) / m_layer->GetTileHeight();

	int leftX = (m_startX < curX) ? m_startX : curX;
	int rightX = (m_startX < curX) ? curX : m_startX;
	int topY = (m_startY < curY) ? m_startY : curY;
	int botY = (m_startY < curY) ? curY : m_startY;

	int width = (rightX - leftX) + 1;
	int height = (botY - topY) + 1;

	m_floatingLayer = make_shared<MapFloatingLayer>(m_layer, leftX, topY, width, height);
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
			m_floatingLayer->SetTile(x, y, m_mouseDownTileSet, m_mouseDownTileIndex);

	UpdateView();
}


void MapEditorWidget::UpdateLineLayer(QMouseEvent* event)
{
	int curX = ((event->x() / m_zoom) + horizontalScrollBar()->value()) / m_layer->GetTileWidth();
	int curY = ((event->y() / m_zoom) + verticalScrollBar()->value()) / m_layer->GetTileHeight();

	int leftX = (m_startX < curX) ? m_startX : curX;
	int rightX = (m_startX < curX) ? curX : m_startX;
	int topY = (m_startY < curY) ? m_startY : curY;
	int botY = (m_startY < curY) ? curY : m_startY;

	if ((leftX == rightX) && (topY == botY))
	{
		// Single tile, draw as a rectangle to simplify logic
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

	m_floatingLayer = make_shared<MapFloatingLayer>(m_layer, leftX, topY, width, height);

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
			m_floatingLayer->SetTile(x - leftX, y - topY, m_mouseDownTileSet, m_mouseDownTileIndex);
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
			m_floatingLayer->SetTile(x - leftX, y - topY, m_mouseDownTileSet, m_mouseDownTileIndex);
			xFrac += deltaMagnitudeX;
			if (xFrac > (deltaMagnitudeY / 2))
			{
				xFrac -= deltaMagnitudeY;
				x += stepX;
			}
		}
	}

	m_floatingLayer->SetTile(curX - leftX, curY - topY, m_mouseDownTileSet, m_mouseDownTileIndex);
	UpdateView();
}


void MapEditorWidget::Fill(QMouseEvent* event)
{
	int curX = ((event->x() / m_zoom) + horizontalScrollBar()->value()) / m_layer->GetTileWidth();
	int curY = ((event->y() / m_zoom) + verticalScrollBar()->value()) / m_layer->GetTileHeight();

	MapFloatingLayerTile replacement;
	replacement.tileSet = m_mouseDownTileSet;
	replacement.index = m_mouseDownTileIndex;

	MapFloatingLayerTile target = GetTile(curX, curY);
	if (!target.valid)
		return;
	if ((target.tileSet == replacement.tileSet) && (target.index == replacement.index))
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
			MapFloatingLayerTile left = GetTile(start - 1, y);
			if (!left.valid)
				break;
			if ((left.tileSet != target.tileSet) || (left.index != target.index))
				break;
		}
		for (; end < (int)(m_layer->GetWidth() - 1); end++)
		{
			MapFloatingLayerTile right = GetTile(end + 1, y);
			if (!right.valid)
				break;
			if ((right.tileSet != target.tileSet) || (right.index != target.index))
				break;
		}

		for (int fill = start; fill <= end; fill++)
		{
			SetTile(fill, y, m_mouseDownTileSet, m_mouseDownTileIndex);

			MapFloatingLayerTile up = GetTile(fill, y - 1);
			if (up.valid && (up.tileSet == target.tileSet) && (up.index == target.index))
				workQueue.push(pair<int, int>(fill, y - 1));

			MapFloatingLayerTile down = GetTile(fill, y + 1);
			if (down.valid && (down.tileSet == target.tileSet) && (down.index == target.index))
				workQueue.push(pair<int, int>(fill, y + 1));
		}
	}

	m_layer->UpdateRegionForSmartTiles(0, 0, m_layer->GetWidth(), m_layer->GetHeight());
	UpdateView();
}


void MapEditorWidget::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		m_mouseDownTileSet = m_leftTileSet;
		m_mouseDownTileIndex = m_leftTileIndex;
	}
	else if (event->button() == Qt::RightButton)
	{
		m_mouseDownTileSet = m_rightTileSet;
		m_mouseDownTileIndex = m_rightTileIndex;
	}
	else
	{
		return;
	}
	m_mouseDown = true;

	switch (m_tool)
	{
	case SelectTool:
		if (IsMouseInSelection(event))
		{
			m_startX = ((event->x() / m_zoom) + horizontalScrollBar()->value()) / m_layer->GetTileWidth();
			m_startY = ((event->y() / m_zoom) + verticalScrollBar()->value()) / m_layer->GetTileHeight();
			m_moveSelection = true;
			m_waitForSelection = 0;
			BeginMoveSelectionLayer();
		}
		else
		{
			m_startX = ((event->x() / m_zoom) + horizontalScrollBar()->value()) / m_layer->GetTileWidth();
			m_startY = ((event->y() / m_zoom) + verticalScrollBar()->value()) / m_layer->GetTileHeight();
			m_selectionContents.reset();
			m_underSelection.reset();
			m_moveSelection = false;
			m_waitForSelection = 6;
			UpdateView();
		}
		break;

	case PenTool:
		SetTileForMouseEvent(event);
		break;

	case RectangleTool:
		m_startX = ((event->x() / m_zoom) + horizontalScrollBar()->value()) / m_layer->GetTileWidth();
		m_startY = ((event->y() / m_zoom) + verticalScrollBar()->value()) / m_layer->GetTileHeight();
		UpdateRectangleLayer(event);
		break;

	case FilledRectangleTool:
		m_startX = ((event->x() / m_zoom) + horizontalScrollBar()->value()) / m_layer->GetTileWidth();
		m_startY = ((event->y() / m_zoom) + verticalScrollBar()->value()) / m_layer->GetTileHeight();
		UpdateFilledRectangleLayer(event);
		break;

	case LineTool:
		m_startX = ((event->x() / m_zoom) + horizontalScrollBar()->value()) / m_layer->GetTileWidth();
		m_startY = ((event->y() / m_zoom) + verticalScrollBar()->value()) / m_layer->GetTileHeight();
		UpdateLineLayer(event);
		break;

	case FillTool:
		Fill(event);
		CommitPendingActions();
		break;

	default:
		break;
	}
}


void MapEditorWidget::CommitPendingActions()
{
	if ((m_pendingActions.size() != 0) || (m_pendingSelections.size() != 0))
	{
		vector<EditAction> editActions = m_pendingActions;
		vector<SelectAction> selectActions = m_pendingSelections;
		m_pendingActions.clear();
		m_pendingSelections.clear();

		// Merge selection actions
		if (selectActions.size() > 1)
		{
			selectActions[0].newSelectionContents = selectActions[selectActions.size() - 1].newSelectionContents;
			selectActions[0].newUnderSelection = selectActions[selectActions.size() - 1].newUnderSelection;
			selectActions.erase(selectActions.begin() + 1, selectActions.end());
		}

		shared_ptr<MapLayer> layer = m_layer;
		shared_ptr<Map> map = m_map;
		MainWindow* mainWindow = m_mainWindow;
		bool effectLayerEditor = m_effectLayerEditor;
		m_mainWindow->AddUndoAction(
			[=]() { // Undo
				for (size_t i = 0; i < editActions.size(); i++)
				{
					EditAction action = editActions[editActions.size() - (i + 1)];
					TileReference ref;
					ref.tileSet = action.oldTileSet;
					ref.index = action.oldTileIndex;
					action.layer->SetTileAt(action.x, action.y, ref);
					action.layer->UpdateRegionForSmartTiles(action.x, action.y, 1, 1);
				}
				for (auto& i : selectActions)
				{
					MapEditorWidget* editor;
					if (effectLayerEditor)
					{
						EffectLayerView* view = mainWindow->GetEffectLayerView(layer);
						if (!view)
							break;
						editor = view->GetEditor();
					}
					else
					{
						MapView* view = mainWindow->GetMapView(map);
						if (!view)
							break;
						editor = view->GetEditor();
						editor->SetActiveLayer(layer);
					}
					editor->m_selectionContents = i.oldSelectionContents;
					editor->m_underSelection = i.oldUnderSelection;
					if (editor->m_selectionContents)
					{
						editor->m_tool = SelectTool;
						editor->m_showHover = false;
						if (effectLayerEditor)
						{
							EffectLayerView* view = mainWindow->GetEffectLayerView(layer);
							if (view)
								view->UpdateToolState();
						}
						else
						{
							MapView* view = mainWindow->GetMapView(map);
							if (view)
								view->UpdateToolState();
						}
					}
				}
				if (effectLayerEditor)
					mainWindow->UpdateEffectLayerContents(layer);
				else
					mainWindow->UpdateMapContents(map);
			},
			[=]() { // Redo
				for (size_t i = 0; i < editActions.size(); i++)
				{
					EditAction action = editActions[i];
					TileReference ref;
					ref.tileSet = action.newTileSet;
					ref.index = action.newTileIndex;
					action.layer->SetTileAt(action.x, action.y, ref);
					action.layer->UpdateRegionForSmartTiles(action.x, action.y, 1, 1);
				}
				for (auto& i : selectActions)
				{
					MapEditorWidget* editor;
					if (effectLayerEditor)
					{
						EffectLayerView* view = mainWindow->GetEffectLayerView(layer);
						if (!view)
							break;
						editor = view->GetEditor();
					}
					else
					{
						MapView* view = mainWindow->GetMapView(map);
						if (!view)
							break;
						editor = view->GetEditor();
						editor->SetActiveLayer(layer);
					}
					editor->m_selectionContents = i.newSelectionContents;
					editor->m_underSelection = i.newUnderSelection;
					if (editor->m_selectionContents)
					{
						editor->m_tool = SelectTool;
						editor->m_showHover = false;
						if (effectLayerEditor)
						{
							EffectLayerView* view = mainWindow->GetEffectLayerView(layer);
							if (view)
								view->UpdateToolState();
						}
						else
						{
							MapView* view = mainWindow->GetMapView(map);
							if (view)
								view->UpdateToolState();
						}
					}
				}
				if (effectLayerEditor)
					mainWindow->UpdateEffectLayerContents(layer);
				else
					mainWindow->UpdateMapContents(map);
			});

		if (effectLayerEditor)
			mainWindow->UpdateEffectLayerContents(layer);
		else
			mainWindow->UpdateMapContents(map);
	}
}


void MapEditorWidget::mouseReleaseEvent(QMouseEvent* event)
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
			UpdateView();
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

	case LineTool:
		UpdateLineLayer(event);
		ApplyLayer(m_floatingLayer);
		m_floatingLayer.reset();
		break;

	default:
		break;
	}

	CommitPendingActions();
}


void MapEditorWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (m_tool != SelectTool)
		m_showHover = true;

	m_hoverX = (event->x() / m_zoom) + horizontalScrollBar()->value();
	m_hoverY = (event->y() / m_zoom) + verticalScrollBar()->value();

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
			SetTileForMouseEvent(event);
			break;

		case RectangleTool:
			UpdateRectangleLayer(event);
			break;

		case FilledRectangleTool:
			UpdateFilledRectangleLayer(event);
			break;

		case LineTool:
			UpdateLineLayer(event);
			break;

		default:
			break;
		}
	}

	viewport()->update();
}


void MapEditorWidget::leaveEvent(QEvent*)
{
	m_showHover = false;
	viewport()->update();
}


void MapEditorWidget::resizeEvent(QResizeEvent*)
{
	UpdateView();
}


void MapEditorWidget::OnAnimationTimer()
{
	viewport()->update();
}


void MapEditorWidget::SetTool(EditorTool tool)
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

	viewport()->update();
}


void MapEditorWidget::ZoomIn()
{
	if (m_zoom < 8)
	{
		m_zoom *= 2;
		UpdateView();
	}
}


void MapEditorWidget::ZoomOut()
{
	if (m_zoom > 1)
	{
		m_zoom /= 2;
		UpdateView();
	}
}


bool MapEditorWidget::Cut()
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


bool MapEditorWidget::Copy()
{
	if (!m_selectionContents)
		return false;

	Json::Value data(Json::objectValue);

	Json::Value tiles(Json::objectValue);
	tiles["width"] = m_selectionContents->GetWidth();
	tiles["height"] = m_selectionContents->GetHeight();

	map<shared_ptr<TileSet>, int> tileSetIndex;
	Json::Value tileSets(Json::arrayValue);
	int curTileSetIndex = 1;
	for (int y = 0; y < m_selectionContents->GetHeight(); y++)
	{
		for (int x = 0; x < m_selectionContents->GetWidth(); x++)
		{
			MapFloatingLayerTile tile = m_selectionContents->GetTile(x, y);
			if (!tile.valid)
				continue;
			auto i = tileSetIndex.find(tile.tileSet);
			if (i == tileSetIndex.end())
			{
				tileSetIndex[tile.tileSet] = curTileSetIndex++;
				if (tile.tileSet)
					tileSets.append(tile.tileSet->GetId());
				else
					tileSets.append("");
			}
		}
	}
	tiles["tile_sets"] = tileSets;

	string tileIndexData, tileSetData;
	for (int y = 0; y < m_selectionContents->GetHeight(); y++)
	{
		for (int x = 0; x < m_selectionContents->GetWidth(); x++)
		{
			MapFloatingLayerTile tile = m_selectionContents->GetTile(x, y);
			if (!tile.valid)
			{
				tileIndexData += "0000";
				tileSetData += "00";
				continue;
			}

			char dataStr[32];
			sprintf(dataStr, "%.4x", (uint8_t)tile.index);
			tileIndexData += dataStr;
			sprintf(dataStr, "%.2x", (uint8_t)tileSetIndex[tile.tileSet]);
			tileSetData += dataStr;
		}
	}
	tiles["tile_index_data"] = tileIndexData;
	tiles["tile_set_data"] = tileSetData;

	data["tiles"] = tiles;

	Json::Value image(Json::objectValue);
	image["width"] = (uint32_t)(m_selectionContents->GetWidth() * m_layer->GetTileWidth());
	image["height"] = (uint32_t)(m_selectionContents->GetHeight() * m_layer->GetTileHeight());

	map<shared_ptr<Palette>, int> paletteIndex;
	Json::Value palettes(Json::arrayValue);
	int curPaletteIndex = 1;
	for (int y = 0; y < m_selectionContents->GetHeight(); y++)
	{
		for (int x = 0; x < m_selectionContents->GetWidth(); x++)
		{
			MapFloatingLayerTile tile = m_selectionContents->GetTile(x, y);
			if (!tile.valid)
				continue;
			if (!tile.tileSet)
				continue;

			shared_ptr<Tile> tileObj = tile.tileSet->GetTile(tile.index);
			if (!tileObj)
				continue;

			auto i = paletteIndex.find(tileObj->GetPalette());
			if (i == paletteIndex.end())
			{
				paletteIndex[tileObj->GetPalette()] = curPaletteIndex++;
				if (tileObj->GetPalette())
					palettes.append(tileObj->GetPalette()->GetId());
				else
					palettes.append("");
			}
		}
	}
	image["palettes"] = palettes;

	string imageData, imagePalette;
	for (int y = 0; y < (int)(m_selectionContents->GetHeight() * m_layer->GetTileWidth()); y++)
	{
		for (int x = 0; x < (int)(m_selectionContents->GetWidth() * m_layer->GetTileHeight()); x++)
		{
			MapFloatingLayerTile tile = m_selectionContents->GetTile(x / m_layer->GetTileWidth(),
				y / m_layer->GetTileHeight());
			if ((!tile.valid) || (!tile.tileSet))
			{
				imageData += "00";
				imagePalette += "00";
				continue;
			}

			shared_ptr<Tile> tileObj = tile.tileSet->GetTile(tile.index);
			if ((!tileObj) || (tileObj->GetWidth() != m_layer->GetTileWidth()) ||
				(tileObj->GetHeight() != m_layer->GetTileHeight()))
			{
				imageData += "00";
				imagePalette += "00";
				continue;
			}

			int pixelX = x % m_layer->GetTileWidth();
			int pixelY = y % m_layer->GetTileHeight();
			uint8_t colorIndex;
			if (tileObj->GetDepth() == 4)
			{
				size_t offset = (pixelY * tileObj->GetWidth() / 2) + (pixelX / 2);
				colorIndex = (tileObj->GetData()[offset] >> ((x & 1) << 2)) & 0xf;
			}
			else
			{
				size_t offset = (pixelY * tileObj->GetWidth()) + pixelX;
				colorIndex = tileObj->GetData()[offset];
			}

			if ((colorIndex == 0) || (!tileObj->GetPalette()))
			{
				imageData += "00";
				imagePalette += "00";
				continue;
			}

			char dataStr[32];
			sprintf(dataStr, "%.2x", (uint8_t)(colorIndex + tileObj->GetPaletteOffset()));
			imageData += dataStr;
			sprintf(dataStr, "%.2x", (uint8_t)paletteIndex[tileObj->GetPalette()]);
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


bool MapEditorWidget::Paste()
{
	QClipboard* clipboard = QGuiApplication::clipboard();
	QString text = clipboard->text();

	Json::Reader reader;
	Json::Value jsonData;
	if (!reader.parse(text.toStdString(), jsonData, false))
		return false;

	try
	{
		const Json::Value& tiles = jsonData["tiles"];
		int width = tiles["width"].asInt();
		int height = tiles["height"].asInt();

		vector<shared_ptr<TileSet>> tileSets;
		tileSets.push_back(shared_ptr<TileSet>());
		for (auto& i : tiles["tile_sets"])
		{
			string id = i.asString();
			if (id.size() == 0)
				tileSets.push_back(shared_ptr<TileSet>());
			else
				tileSets.push_back(m_project->GetTileSetById(id));
		}

		string tileIndexData = tiles["tile_index_data"].asString();
		string tileSetData = tiles["tile_set_data"].asString();
		if (tileIndexData.size() != (size_t)(width * height * 4))
			return false;
		if (tileSetData.size() != (size_t)(width * height * 2))
			return false;
		if ((width <= 0) || (height <= 0) || (width >= 0x8000) || (height >= 0x8000))
			return false;

		int centerX = (viewport()->rect().center().x() + (horizontalScrollBar()->value() * m_zoom)) /
			(m_zoom * m_layer->GetTileWidth());
		int centerY = (viewport()->rect().center().y() + (verticalScrollBar()->value() * m_zoom)) /
			(m_zoom * m_layer->GetTileHeight());

		int posX = centerX - (width / 2);
		int posY = centerY - (height / 2);
		if ((posX + width) > (int)m_layer->GetWidth())
			posX = (int)m_layer->GetWidth() - width;
		if ((posY + height) > (int)m_layer->GetHeight())
			posY = (int)m_layer->GetHeight() - height;
		if (posX < 0)
			posX = 0;
		if (posY < 0)
			posY = 0;

		shared_ptr<MapFloatingLayer> selectionContents = make_shared<MapFloatingLayer>(m_layer,
			posX, posY, width, height);
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				uint8_t tileSetIndex = (uint8_t)strtoul(
					tileSetData.substr((y * width * 2) + (x * 2), 2).c_str(), nullptr, 16);
				uint8_t tileIndex = (uint8_t)strtoul(
					tileIndexData.substr((y * width * 4) + (x * 4), 4).c_str(), nullptr, 16);

				if ((size_t)tileSetIndex >= tileSets.size())
				{
					tileSetIndex = 0;
					tileIndex = 0;
				}

				shared_ptr<TileSet> tileSet = tileSets[tileSetIndex];
				if (tileSet && ((size_t)tileIndex >= tileSet->GetTileCount()))
				{
					tileSet.reset();
					tileIndex = 0;
				}

				selectionContents->SetTile(x, y, tileSet, tileIndex);
			}
		}

		SelectAction action;
		action.oldSelectionContents = m_selectionContents;
		action.oldUnderSelection = m_underSelection;

		m_selectionContents = selectionContents;
		m_underSelection = make_shared<MapFloatingLayer>(m_layer, posX, posY, width, height);

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


void MapEditorWidget::SelectAll()
{
	m_tool = SelectTool;
	m_showHover = false;

	SelectAction action;
	action.oldSelectionContents = m_selectionContents;
	action.oldUnderSelection = m_underSelection;

	int width = (int)m_layer->GetWidth();
	int height = (int)m_layer->GetHeight();

	m_selectionContents = make_shared<MapFloatingLayer>(m_layer, 0, 0, width, height);
	m_underSelection = make_shared<MapFloatingLayer>(m_layer, 0, 0, width, height);

	action.newSelectionContents = m_selectionContents;
	action.newUnderSelection = m_underSelection;
	m_pendingSelections.push_back(action);

	CaptureLayer(m_selectionContents);

	// If selection is moved, replace with transparent
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
			m_underSelection->SetTile(x, y, shared_ptr<TileSet>(), 0);

	CommitPendingActions();
	UpdateView();
}
