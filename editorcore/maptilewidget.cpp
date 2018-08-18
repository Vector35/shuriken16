#include <QVBoxLayout>
#include <QGuiApplication>
#include <QStyle>
#include <QColorDialog>
#include <QLabel>
#include "maptilewidget.h"
#include "theme.h"
#include "mapeditorwidget.h"
#include "mainwindow.h"
#include "tileselectwidget.h"
#include "emptytilewidget.h"

using namespace std;


MapTileWidget::MapTileWidget(QWidget* parent, MapEditorWidget* editor, MainWindow* mainWindow,
	shared_ptr<Project> project, shared_ptr<Map> map):
	QWidget(parent), m_mainWindow(mainWindow), m_editor(editor), m_project(project), m_map(map)
{
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);
	m_entryLayout = new QGridLayout();
	m_entryLayout->setSpacing(2);
	m_entryLayout->setColumnStretch(1, 1);
	layout->addLayout(m_entryLayout);
	setLayout(layout);

	QFontMetrics metrics(QGuiApplication::font());
	setMinimumSize(256 + 16 + style()->pixelMetric(QStyle::PM_ScrollBarExtent), 0);
}


void MapTileWidget::AddTileSetWidgets(shared_ptr<TileSet> tileSet, int& row)
{
	QLabel* name = new QLabel(QString::fromStdString(tileSet->GetName()));
	QPalette style(this->palette());
	style.setColor(QPalette::WindowText, Theme::blue);
	name->setPalette(style);
	m_entryLayout->addWidget(name, row, 0, 1, 2);
	m_entries.push_back(name);
	row++;

	TileSelectWidget* widget = new TileSelectWidget(this, m_editor, m_mainWindow, m_project, tileSet);
	m_entryLayout->addWidget(widget, row, 0);
	m_entries.push_back(widget);
	row++;
}


void MapTileWidget::UpdateView()
{
	for (auto i : m_entries)
	{
		m_entryLayout->removeWidget(i);
		i->deleteLater();
	}
	m_entries.clear();

	int row = 0;

	vector<shared_ptr<TileSet>> usedTileSets;
	vector<shared_ptr<TileSet>> availableTileSets;
	for (auto& i : m_project->GetTileSets())
	{
		if ((i.second->GetWidth() != m_editor->GetActiveLayer()->GetTileWidth()) ||
			(i.second->GetHeight() != m_editor->GetActiveLayer()->GetTileHeight()) ||
			(i.second->GetDepth() != m_editor->GetActiveLayer()->GetTileDepth()))
			continue;
		if (m_map->UsesTileSet(i.second))
			usedTileSets.push_back(i.second);
		else
			availableTileSets.push_back(i.second);
	}

	QLabel* empty = new QLabel("Empty Tile");
	QFont headerFont = QGuiApplication::font();
	headerFont.setPointSize(headerFont.pointSize() * 5 / 4);
	empty->setFont(headerFont);
	m_entryLayout->addWidget(empty, row, 0, 1, 2);
	m_entries.push_back(empty);
	row++;

	EmptyTileWidget* emptyTile = new EmptyTileWidget(this, m_editor, m_mainWindow, m_project,
		m_editor->GetActiveLayer()->GetTileWidth(), m_editor->GetActiveLayer()->GetTileHeight());
	m_entryLayout->addWidget(emptyTile, row, 0);
	m_entries.push_back(emptyTile);
	row++;

	if ((usedTileSets.size() != 0) || (availableTileSets.size() != 0))
	{
		m_entryLayout->addWidget(new QLabel(), row, 1);
		row++;
	}

	if (usedTileSets.size() != 0)
	{
		QLabel* available = new QLabel("Active Tiles");
		QFont headerFont = QGuiApplication::font();
		headerFont.setPointSize(headerFont.pointSize() * 5 / 4);
		available->setFont(headerFont);
		m_entryLayout->addWidget(available, row, 0, 1, 2);
		m_entries.push_back(available);
		row++;

		for (auto& i : usedTileSets)
			AddTileSetWidgets(i, row);

		if (availableTileSets.size() != 0)
		{
			m_entryLayout->addWidget(new QLabel(), row, 1);
			row++;
		}
	}

	if (availableTileSets.size() != 0)
	{
		QLabel* available = new QLabel("Available Tiles");
		QFont headerFont = QGuiApplication::font();
		headerFont.setPointSize(headerFont.pointSize() * 5 / 4);
		available->setFont(headerFont);
		m_entryLayout->addWidget(available, row, 0, 1, 2);
		m_entries.push_back(available);
		row++;

		for (auto& i : availableTileSets)
			AddTileSetWidgets(i, row);
	}
}
