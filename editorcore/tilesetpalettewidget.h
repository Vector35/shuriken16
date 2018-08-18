#pragma once

#include <QWidget>
#include <QGridLayout>
#include "project.h"
#include "tileset.h"
#include "palette.h"

class MainWindow;
class TileSetView;

class TileSetPaletteWidget: public QWidget
{
	Q_OBJECT

	MainWindow* m_mainWindow;
	TileSetView* m_view;
	std::shared_ptr<Project> m_project;
	std::shared_ptr<TileSet> m_tileSet;

	QGridLayout* m_entryLayout;
	std::vector<QWidget*> m_entries;

	void EditPaletteEntry(std::shared_ptr<Palette> palette, size_t i);
	void AddPaletteWidgets(std::shared_ptr<Palette> palette, int& row);

public:
	TileSetPaletteWidget(QWidget* parent, TileSetView* view, MainWindow* mainWindow,
		std::shared_ptr<Project> project, std::shared_ptr<TileSet> tileSet);

	void UpdateView();
};
