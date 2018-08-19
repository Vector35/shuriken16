#include <QVBoxLayout>
#include <QPainter>
#include <QImage>
#include <QTimer>
#include <string.h>
#include "spritepreviewwidget.h"
#include "theme.h"

using namespace std;


SpritePreviewWidget::SpritePreviewWidget(QWidget* parent, SpriteView* view, MainWindow* mainWindow,
	shared_ptr<Project> project, shared_ptr<Sprite> sprite):
	QWidget(parent), m_mainWindow(mainWindow), m_view(view), m_project(project), m_sprite(sprite)
{
	m_animation = m_sprite->GetAnimation(0);
	m_image = nullptr;
	m_animate = false;
	m_animFrame = 0;
	m_activeFrame = 0;

	m_animTimer = new QTimer(this);
	m_animTimer->setSingleShot(false);
	m_animTimer->setInterval(1000 / 60);
	connect(m_animTimer, &QTimer::timeout, this, &SpritePreviewWidget::OnAnimationTimer);
}


SpritePreviewWidget::~SpritePreviewWidget()
{
	if (m_image)
		delete m_image;
}


void SpritePreviewWidget::UpdateImageData()
{
	if (!m_image)
		return;

	uint16_t frame = m_activeFrame;
	if (m_animate)
		frame = m_animation->GetFrameForTime(m_animFrame);

	for (int y = 0; y < m_height; y++)
		memset(m_image->scanLine(y), 0, m_width * 4);

	shared_ptr<Tile> tile = m_animation->GetTile();
	if (!tile)
		return;
	if ((tile->GetWidth() != m_sprite->GetWidth()))
		return;
	if ((tile->GetHeight() != m_sprite->GetHeight()))
		return;
	if ((tile->GetDepth() != m_sprite->GetDepth()))
		return;

	for (int y = 0; y < (int)m_sprite->GetHeight(); y++)
	{
		uint32_t* line = (uint32_t*)m_image->scanLine(y);
		for (int x = 0; x < (int)m_sprite->GetWidth(); x++)
		{
			uint8_t colorIndex;
			if (tile->GetDepth() == 4)
				colorIndex = (tile->GetData(frame)[(y * tile->GetPitch()) + (x / 2)] >> ((x & 1) << 2)) & 0xf;
			else
				colorIndex = tile->GetData(frame)[(y * tile->GetPitch()) + x];
			if (colorIndex == 0)
				continue;
			if (!tile->GetPalette())
				continue;

			uint16_t paletteEntry = tile->GetPalette()->GetEntry(tile->GetPaletteOffset() + colorIndex);
			line[x] = Palette::ToRGB32(paletteEntry) | 0xff000000;
		}
	}
}


void SpritePreviewWidget::UpdateView()
{
	int newWidth = m_sprite->GetWidth();
	int newHeight = m_sprite->GetHeight();

	if ((!m_image) || (newWidth != m_width) || (newHeight != m_height))
	{
		if (m_image)
			delete m_image;
		m_width = newWidth;
		m_height = newHeight;
		m_image = new QImage(m_width, m_height, QImage::Format_ARGB32);
	}

	UpdateImageData();

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


void SpritePreviewWidget::paintEvent(QPaintEvent*)
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


void SpritePreviewWidget::SetPreviewAnimation(bool anim)
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


void SpritePreviewWidget::SetActiveFrame(const shared_ptr<SpriteAnimation>& anim, uint16_t frame)
{
	m_animation = anim;
	m_activeFrame = frame;
	UpdateImageData();
	update();
}


void SpritePreviewWidget::OnAnimationTimer()
{
	m_animFrame++;

	if ((!m_animation->IsLooping()) && (m_animFrame > (m_animation->GetAnimation()->GetTotalLength() + 60)))
		m_animFrame = 0;

	UpdateImageData();
	update();
}
