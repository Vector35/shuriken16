#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include "tileset.h"

class MainWindow;
class TileSetEditorWidget;

class TileSetAssociatedSetsWidget: public QWidget
{
	Q_OBJECT

	MainWindow* m_mainWindow;
	std::shared_ptr<Project> m_project;
	std::shared_ptr<TileSet> m_tileSet;

	QVBoxLayout* m_entryLayout;
	std::vector<QWidget*> m_entries;
	QPushButton* m_addButton;

public:
	TileSetAssociatedSetsWidget(QWidget* parent, MainWindow* mainWindow,
		std::shared_ptr<Project> project, std::shared_ptr<TileSet> tileSet);

	void UpdateView();

private slots:
	void OnAddSet();
};
