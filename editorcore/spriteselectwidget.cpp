#include <QVBoxLayout>
#include <QPainter>
#include <QImage>
#include <QGuiApplication>
#include <QInputDialog>
#include <string.h>
#include "spriteselectwidget.h"
#include "theme.h"

using namespace std;


SpriteSelectWidget::SpriteSelectWidget(QWidget* parent, shared_ptr<Project> project, shared_ptr<Sprite> sprite,
	const function<void(shared_ptr<Sprite> oldSprite, shared_ptr<Sprite> newSprite)>& updateFunc):
	QWidget(parent), m_project(project), m_sprite(sprite), m_updateFunc(updateFunc)
{
	m_image = nullptr;
}


SpriteSelectWidget::~SpriteSelectWidget()
{
	if (m_image)
		delete m_image;
}


void SpriteSelectWidget::UpdateImageData()
{
	if (!m_image)
		return;

	for (int y = 0; y < m_height; y++)
		memset(m_image->scanLine(y), 0, m_width * 4);

	shared_ptr<SpriteAnimation> animation = m_sprite->GetAnimation(0);
	if (!animation)
		return;
	shared_ptr<Tile> tile = animation->GetTile();
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
				colorIndex = (tile->GetData(0)[(y * tile->GetPitch()) + (x / 2)] >> ((x & 1) << 2)) & 0xf;
			else
				colorIndex = tile->GetData(0)[(y * tile->GetPitch()) + x];
			if (colorIndex == 0)
				continue;
			if (!tile->GetPalette())
				continue;

			uint16_t paletteEntry = tile->GetPalette()->GetEntry(tile->GetPaletteOffset() + colorIndex);
			line[x] = Palette::ToRGB32(paletteEntry) | 0xff000000;
		}
	}
}


void SpriteSelectWidget::SetSprite(const shared_ptr<Sprite>& sprite)
{
	m_sprite = sprite;
	UpdateView();
}


void SpriteSelectWidget::UpdateView()
{
	if (!m_sprite)
	{
		if (m_image)
		{
			delete m_image;
			m_image = nullptr;
		}
		setMinimumSize(QSize(48, 48));
		return;
	}

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


void SpriteSelectWidget::mousePressEvent(QMouseEvent*)
{
	QStringList spriteNames;
	map<QString, shared_ptr<Sprite>> sprites;

	vector<shared_ptr<Sprite>> sortedSprites;
	for (auto& i : m_project->GetSprites())
		sortedSprites.push_back(i.second);
	sort(sortedSprites.begin(), sortedSprites.end(), [&](const shared_ptr<Sprite>& a, const shared_ptr<Sprite>& b) {
		return a->GetName() < b->GetName();
	});

	spriteNames.append("<None>");
	sprites["<None>"] = shared_ptr<Sprite>();

	int selected = 0;
	int cur = 1;
	for (auto& i : sortedSprites)
	{
		QString name = QString::fromStdString(i->GetName());
		spriteNames.append(name);
		sprites[name] = i;

		if (i == m_sprite)
			selected = cur;
		cur++;
	}

	bool ok;
	QString chosenName = QInputDialog::getItem(this, "Select Sprite", "Sprite Name:", spriteNames, selected, false, &ok);
	if (!ok)
		return;

	auto i = sprites.find(chosenName);
	shared_ptr<Sprite> oldSprite = m_sprite;
	shared_ptr<Sprite> newSprite;
	if (i != sprites.end())
		newSprite = i->second;
	m_updateFunc(oldSprite, newSprite);
}


void SpriteSelectWidget::paintEvent(QPaintEvent*)
{
	QPainter p(this);
	if (!m_image)
	{
		p.fillRect(rect(), Theme::background);
		p.setBrush(QBrush(Theme::backgroundHighlight, Qt::BDiagPattern));
		p.setPen(Theme::content);
		p.drawRect(rect());

		QFont font = QGuiApplication::font();
		font.setPointSize(font.pointSize() * 3 / 4);
		font.setItalic(true);
		QFontMetrics metrics(font);

		p.setFont(font);
		p.drawText((rect().width() / 2) - (metrics.boundingRect("No sprite").width() / 2),
			(rect().height() / 2) + (metrics.ascent() / 2), "No sprite");
		return;
	}

	p.fillRect(rect(), Theme::background);
	p.setBrush(QBrush(Theme::backgroundHighlight, Qt::BDiagPattern));
	p.setPen(Qt::NoPen);
	p.drawRect(rect());
	p.drawImage(rect(), *m_image);
}
