#pragma once

#include <QWidget>
#include <QLabel>
#include "project.h"
#include "map.h"
#include "mapeditorwidget.h"
#include "maplayerwidget.h"
#include "maptilewidget.h"
#include "editorview.h"
#include "toolwidget.h"

class MainWindow;

class MapView: public EditorView
{
	Q_OBJECT

	MapEditorWidget* m_editor;
	MapLayerWidget* m_layers;
	MapTileWidget* m_tiles;
	MapActorWidget* m_actors;
	QLabel* m_mapSize;

	MainWindow* m_mainWindow;
	std::shared_ptr<Project> m_project;
	std::shared_ptr<Map> m_map;

	ToolWidget* m_selectMode;
	ToolWidget* m_penMode;
	ToolWidget* m_rectMode;
	ToolWidget* m_fillRectMode;
	ToolWidget* m_lineMode;
	ToolWidget* m_fillMode;
	ToolWidget* m_zoomInMode;
	ToolWidget* m_zoomOutMode;
	ToolWidget* m_actorMode;

	QTimer* m_deferredUpdateTimer;
	std::chrono::steady_clock::time_point m_lastUpdate;
	bool m_firstUpdate;

public:
	MapView(MainWindow* parent, std::shared_ptr<Project> project, std::shared_ptr<Map> map);

	void UpdateView();
	void UpdateToolState();

	std::shared_ptr<Map> GetMap() const { return m_map; }
	MapEditorWidget* GetEditor() { return m_editor; }

	virtual void Cut() override;
	virtual void Copy() override;
	virtual void Paste() override;
	virtual void SelectAll() override;

private slots:
	void OnDeferredUpdateTimer();
};
