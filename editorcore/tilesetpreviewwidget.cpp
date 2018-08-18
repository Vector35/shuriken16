#include <QVBoxLayout>
#include <QPainter>
#include <QImage>
#include <QTimer>
#include <string.h>
#include "tilesetpreviewwidget.h"
#include "theme.h"

using namespace std;


TileSetPreviewWidget::TileSetPreviewWidget(QWidget* parent, TileSetView* view, MainWindow* mainWindow,
	shared_ptr<Project> project, shared_ptr<TileSet> tileSet):
	QWidget(parent), m_mainWindow(mainWindow), m_view(view), m_project(project), m_tileSet(tileSet)
{
	m_image = nullptr;
	m_animate = false;
	m_animFrame = 0;
	m_activeFrame = 0;

	m_animTimer = new QTimer(this);
	m_animTimer->setSingleShot(false);
	m_animTimer->setInterval(1000 / 60);
	connect(m_animTimer, &QTimer::timeout, this, &TileSetPreviewWidget::OnAnimationTimer);
}


TileSetPreviewWidget::~TileSetPreviewWidget()
{
	if (m_image)
		delete m_image;
}


void TileSetPreviewWidget::UpdateImageData(int rows, int cols)
{
	uint16_t frame = m_activeFrame;
	if (m_animate)
		frame = m_tileSet->GetFrameForTime(m_animFrame);

	for (int y = 0; y < m_height; y++)
		memset(m_image->scanLine(y), 0, m_width * 4);

	for (int tileY = 0; tileY < rows; tileY++)
	{
		for (int tileX = 0; tileX < cols; tileX++)
		{
			size_t tileIndex = (size_t)((tileY * cols) + tileX);
			if (tileIndex >= m_tileSet->GetTileCount())
			{
				for (int y = 0; y < (int)m_tileSet->GetWidth(); y++)
				{
					uint32_t* line = (uint32_t*)m_image->scanLine(tileY * m_tileSet->GetHeight() + y);
					for (int x = 0; x < (int)m_tileSet->GetHeight(); x++)
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

			for (int y = 0; y < (int)m_tileSet->GetWidth(); y++)
			{
				uint32_t* line = (uint32_t*)m_image->scanLine(tileY * m_tileSet->GetHeight() + y);
				for (int x = 0; x < (int)m_tileSet->GetHeight(); x++)
				{
					uint8_t colorIndex;
					if (tile->GetDepth() == 4)
						colorIndex = (tile->GetData(frame)[(y * tile->GetWidth() / 2) + (x / 2)] >> ((x & 1) << 2)) & 0xf;
					else
						colorIndex = tile->GetData(frame)[(y * tile->GetWidth()) + x];
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
}


void TileSetPreviewWidget::UpdateImageData()
{
	int cols, rows;
	cols = m_tileSet->GetDisplayColumns();
	if (cols > (int)m_tileSet->GetTileCount())
		cols = (int)m_tileSet->GetTileCount();
	rows = (m_tileSet->GetTileCount() + (cols - 1)) / cols;

	int newWidth = cols * m_tileSet->GetWidth();
	int newHeight = rows * m_tileSet->GetHeight();
	if ((!m_image) || (newWidth != m_width) || (newHeight != m_height))
		return;

	UpdateImageData(rows, cols);
}


void TileSetPreviewWidget::UpdateView()
{
	int cols, rows;
	cols = m_tileSet->GetDisplayColumns();
	if (cols > (int)m_tileSet->GetTileCount())
		cols = (int)m_tileSet->GetTileCount();
	rows = (m_tileSet->GetTileCount() + (cols - 1)) / cols;

	int newWidth = cols * m_tileSet->GetWidth();
	int newHeight = rows * m_tileSet->GetHeight();

	if ((!m_image) || (newWidth != m_width) || (newHeight != m_height))
	{
		if (m_image)
			delete m_image;
		m_width = newWidth;
		m_height = newHeight;
		m_image = new QImage(m_width, m_height, QImage::Format_ARGB32);
	}

	UpdateImageData(rows, cols);

	if (m_width > 128)
	{
		int height = (m_height * 128) / m_width;
		setMinimumSize(QSize(256, height * 2));
	}
	else
	{
		setMinimumSize(QSize(m_width * 2, m_height * 2));
	}

	update();
}


void TileSetPreviewWidget::paintEvent(QPaintEvent*)
{
	QPainter p(this);
	if (!m_image)
		return;

	p.fillRect(rect(), Theme::background);
	p.setBrush(QBrush(Theme::backgroundHighlight, Qt::BDiagPattern));
	p.setPen(Qt::NoPen);
	p.drawRect(rect());
	p.drawImage(rect(), *m_image);
}


void TileSetPreviewWidget::SetPreviewAnimation(bool anim)
{
	if (m_animate == anim)
		return;
	m_animate = anim;
	m_animFrame = 0;
	if (m_animate)
		m_animTimer->start();
	else
		m_animTimer->stop();
	UpdateImageData();
	update();
}


void TileSetPreviewWidget::SetActiveFrame(uint16_t frame)
{
	m_activeFrame = frame;
	UpdateImageData();
	update();
}


void TileSetPreviewWidget::OnAnimationTimer()
{
	m_animFrame++;
	UpdateImageData();
	update();
}
