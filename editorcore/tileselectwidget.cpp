#include <QVBoxLayout>
#include <QPainter>
#include <QImage>
#include <QMouseEvent>
#include <string.h>
#include "tileselectwidget.h"
#include "theme.h"
#include "mapeditorwidget.h"

using namespace std;


TileSelectWidget::TileSelectWidget(QWidget* parent, MapEditorWidget* editor, MainWindow* mainWindow,
	shared_ptr<Project> project, shared_ptr<TileSet> tileSet):
	QWidget(parent), m_mainWindow(mainWindow), m_editor(editor), m_project(project), m_tileSet(tileSet)
{
	m_image = nullptr;
	m_cols = m_rows = 1;
	m_tileWidth = m_tileHeight = 1;
	m_showHover = false;
	UpdateView();

	setMouseTracking(true);
}


TileSelectWidget::~TileSelectWidget()
{
	if (m_image)
		delete m_image;
}


void TileSelectWidget::UpdateView()
{
	m_cols = m_tileSet->GetDisplayColumns();
	if (m_cols > (int)m_tileSet->GetTileCount())
		m_cols = (int)m_tileSet->GetTileCount();
	m_rows = (m_tileSet->GetTileCount() + (m_cols - 1)) / m_cols;
	int newWidth = m_cols * m_tileSet->GetWidth();
	int newHeight = m_rows * m_tileSet->GetHeight();

	m_tileWidth = m_tileSet->GetWidth() * 2;
	m_tileHeight = m_tileSet->GetHeight() * 2;
	while ((m_tileWidth * m_cols) > 256)
	{
		m_tileWidth /= 2;
		m_tileHeight /= 2;
	}

	if ((!m_image) || (newWidth != m_width) || (newHeight != m_height))
	{
		if (m_image)
			delete m_image;
		m_width = newWidth;
		m_height = newHeight;
		m_image = new QImage(m_width, m_height, QImage::Format_ARGB32);
	}

	for (int y = 0; y < m_height; y++)
		memset(m_image->scanLine(y), 0, m_width * 4);

	for (int tileY = 0; tileY < m_rows; tileY++)
	{
		for (int tileX = 0; tileX < m_cols; tileX++)
		{
			size_t tileIndex = (size_t)((tileY * m_cols) + tileX);
			if (tileIndex >= m_tileSet->GetTileCount())
			{
				for (int y = 0; y < (int)m_tileSet->GetHeight(); y++)
				{
					uint32_t* line = (uint32_t*)m_image->scanLine(tileY * m_tileSet->GetHeight() + y);
					for (int x = 0; x < (int)m_tileSet->GetWidth(); x++)
						line[tileX * m_tileSet->GetWidth() + x] = Theme::backgroundWindow.rgba();
				}
				continue;
			}

			shared_ptr<Tile> tile = m_tileSet->GetTile(tileIndex);
			if (!tile)
				continue;
			if ((tile->GetWidth() != m_tileSet->GetWidth()))
				continue;
			if ((tile->GetHeight() != m_tileSet->GetHeight()))
				continue;
			if ((tile->GetDepth() != m_tileSet->GetDepth()))
				continue;

			for (int y = 0; y < (int)m_tileSet->GetHeight(); y++)
			{
				uint32_t* line = (uint32_t*)m_image->scanLine(tileY * m_tileSet->GetHeight() + y);
				for (int x = 0; x < (int)m_tileSet->GetWidth(); x++)
				{
					uint8_t colorIndex;
					if (tile->GetDepth() == 4)
						colorIndex = (tile->GetData()[(y * tile->GetPitch()) + (x / 2)] >> ((x & 1) << 2)) & 0xf;
					else
						colorIndex = tile->GetData()[(y * tile->GetPitch()) + x];
					if (colorIndex == 0)
						continue;
					if (!tile->GetPalette())
						continue;

					uint16_t paletteEntry = tile->GetPalette()->GetEntry(tile->GetPaletteOffset() + colorIndex);
					line[tileX * m_tileSet->GetWidth() + x] = Palette::ToRGB32(paletteEntry) | 0xff000000;
				}
			}
		}
	}

	m_renderSize = QSize(m_tileWidth * m_cols, m_tileHeight * m_rows);
	setMinimumSize(QSize(m_renderSize.width() + 4, m_renderSize.height() + 4));

	update();
}


void TileSelectWidget::paintEvent(QPaintEvent*)
{
	QPainter p(this);
	if (!m_image)
		return;

	p.fillRect(rect(), Theme::backgroundWindow);
	p.setBrush(QBrush(Theme::backgroundHighlight, Qt::BDiagPattern));
	p.setPen(Qt::NoPen);
	p.drawRect(2, 2, m_renderSize.width(), m_renderSize.height());
	p.drawImage(QRect(2, 2, m_renderSize.width(), m_renderSize.height()), *m_image);

	if (m_showHover)
	{
		size_t tileIndex = (size_t)((m_hoverY * m_cols) + m_hoverX);
		if (tileIndex < m_tileSet->GetTileCount())
		{
			if (m_tileSet->IsSmartTileSet())
			{
				p.setPen(QPen(QBrush(Theme::red), 2));
				p.setBrush(Qt::NoBrush);
				p.drawRect(1, 1, m_renderSize.width() + 2, m_renderSize.height() + 2);
			}
			else
			{
				p.setPen(QPen(QBrush(Theme::red), 2));
				p.setBrush(Qt::NoBrush);
				p.drawRect(m_hoverX * m_tileWidth + 1, m_hoverY * m_tileHeight + 1, m_tileWidth + 2, m_tileHeight + 2);
			}
		}
	}

	if (m_editor->GetSelectedRightTileSet() == m_tileSet)
	{
		if (m_tileSet->IsSmartTileSet())
		{
			p.setPen(QPen(QBrush(Theme::orange), 2));
			p.setBrush(Qt::NoBrush);
			p.drawRect(1, 1, m_renderSize.width() + 2, m_renderSize.height() + 2);
		}
		else
		{
			int x = m_editor->GetSelectedRightTileIndex() % m_cols;
			int y = m_editor->GetSelectedRightTileIndex() / m_cols;
			p.setPen(QPen(QBrush(Theme::orange), 2));
			p.setBrush(Qt::NoBrush);
			p.drawRect(x * m_tileWidth + 1, y * m_tileHeight + 1, m_tileWidth + 2, m_tileHeight + 2);
		}
	}

	if (m_editor->GetSelectedLeftTileSet() == m_tileSet)
	{
		if (m_tileSet->IsSmartTileSet())
		{
			p.setPen(QPen(QBrush(Theme::blue), 2));
			p.setBrush(Qt::NoBrush);
			p.drawRect(1, 1, m_renderSize.width() + 2, m_renderSize.height() + 2);
		}
		else
		{
			int x = m_editor->GetSelectedLeftTileIndex() % m_cols;
			int y = m_editor->GetSelectedLeftTileIndex() / m_cols;
			p.setPen(QPen(QBrush(Theme::blue), 2));
			p.setBrush(Qt::NoBrush);
			p.drawRect(x * m_tileWidth + 1, y * m_tileHeight + 1, m_tileWidth + 2, m_tileHeight + 2);
		}
	}
}


void TileSelectWidget::mousePressEvent(QMouseEvent* event)
{
	int x = (event->x() - 2) / m_tileWidth;
	int y = (event->y() - 2) / m_tileHeight;

	if ((x < 0) || (y < 0) || (x >= m_cols) || (y >= m_rows))
		return;

	size_t tileIndex = (size_t)((y * m_cols) + x);
	if (tileIndex >= m_tileSet->GetTileCount())
		return;

	if (m_tileSet->IsSmartTileSet())
		tileIndex = TileSet::GetDefaultTileForSmartTileSet(m_tileSet->GetSmartTileSetType());

	if (event->button() == Qt::LeftButton)
		m_editor->SetSelectedLeftTile(m_tileSet, tileIndex);
	else if (event->button() == Qt::RightButton)
		m_editor->SetSelectedRightTile(m_tileSet, tileIndex);
}


void TileSelectWidget::mouseReleaseEvent(QMouseEvent*)
{
}


void TileSelectWidget::mouseMoveEvent(QMouseEvent* event)
{
	m_hoverX = (event->x() - 2) / m_tileWidth;
	m_hoverY = (event->y() - 2) / m_tileHeight;

	if ((m_hoverX < 0) || (m_hoverY < 0) || (m_hoverX >= m_cols) || (m_hoverY >= m_rows))
	{
		m_showHover = false;
		update();
		return;
	}

	size_t tileIndex = (size_t)((m_hoverY * m_cols) + m_hoverX);
	if (tileIndex >= m_tileSet->GetTileCount())
	{
		m_showHover = false;
		update();
		return;
	}

	m_showHover = true;
	update();
}


void TileSelectWidget::leaveEvent(QEvent*)
{
	m_showHover = false;
	update();
}
