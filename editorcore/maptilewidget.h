#pragma once

#include <QWidget>
#include <QGridLayout>
#include "project.h"
#include "map.h"
#include "tileset.h"

class MainWindow;
class MapEditorWidget;

class MapTileWidget: public QWidget
{
	Q_OBJECT

	MainWindow* m_mainWindow;
	MapEditorWidget* m_editor;
	std::shared_ptr<Project> m_project;
	std::shared_ptr<Map> m_map;

	QGridLayout* m_entryLayout;
	std::vector<QWidget*> m_entries;

	void AddTileSetWidgets(std::shared_ptr<TileSet> tileSet, int& row);

public:
	MapTileWidget(QWidget* parent, MapEditorWidget* editor, MainWindow* mainWindow,
		std::shared_ptr<Project> project, std::shared_ptr<Map> map);

	void UpdateView();
};
