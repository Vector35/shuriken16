#include <QVBoxLayout>
#include <QPainter>
#include <QImage>
#include <QMouseEvent>
#include <string.h>
#include "emptytilewidget.h"
#include "theme.h"
#include "mapeditorwidget.h"

using namespace std;


EmptyTileWidget::EmptyTileWidget(QWidget* parent, MapEditorWidget* editor, MainWindow* mainWindow,
	shared_ptr<Project> project, int width, int height):
	QWidget(parent), m_mainWindow(mainWindow), m_editor(editor), m_project(project)
{
	m_width = width;
	m_height = height;

	if (width >= 128)
	{
		m_tileWidth = 256;
		m_tileHeight = height * 256 / width;
	}
	else
	{
		m_tileWidth = width * 2;
		m_tileHeight = height * 2;
	}

	m_renderSize = QSize(m_tileWidth, m_tileHeight);
	setMinimumSize(QSize(m_renderSize.width() + 4, m_renderSize.height() + 4));

	m_showHover = false;
	setMouseTracking(true);
}


void EmptyTileWidget::paintEvent(QPaintEvent*)
{
	QPainter p(this);
	p.fillRect(rect(), Theme::backgroundWindow);
	p.setBrush(QBrush(Theme::backgroundHighlight, Qt::BDiagPattern));
	p.setPen(Qt::NoPen);
	p.drawRect(2, 2, m_renderSize.width(), m_renderSize.height());

	if (m_showHover)
	{
		p.setPen(QPen(QBrush(Theme::red), 2));
		p.setBrush(Qt::NoBrush);
		p.drawRect(1, 1, m_tileWidth + 2, m_tileHeight + 2);
	}

	if (!m_editor->GetSelectedRightTileSet())
	{
		p.setPen(QPen(QBrush(Theme::orange), 2));
		p.setBrush(Qt::NoBrush);
		p.drawRect(1, 1, m_tileWidth + 2, m_tileHeight + 2);
	}

	if (!m_editor->GetSelectedLeftTileSet())
	{
		p.setPen(QPen(QBrush(Theme::blue), 2));
		p.setBrush(Qt::NoBrush);
		p.drawRect(1, 1, m_tileWidth + 2, m_tileHeight + 2);
	}
}


void EmptyTileWidget::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
		m_editor->SetSelectedLeftTile(shared_ptr<TileSet>(), 0);
	else if (event->button() == Qt::RightButton)
		m_editor->SetSelectedRightTile(shared_ptr<TileSet>(), 0);
}


void EmptyTileWidget::mouseMoveEvent(QMouseEvent*)
{
	m_showHover = true;
	update();
}


void EmptyTileWidget::leaveEvent(QEvent*)
{
	m_showHover = false;
	update();
}
