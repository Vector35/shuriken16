#pragma once

#include <QWidget>
#include <QLabel>
#include "project.h"
#include "maplayer.h"
#include "mapeditorwidget.h"
#include "effectlayersettingswidget.h"
#include "maptilewidget.h"
#include "editorview.h"
#include "toolwidget.h"

class MainWindow;

class EffectLayerView: public EditorView
{
	Q_OBJECT

	MapEditorWidget* m_editor;
	EffectLayerSettingsWidget* m_settings;
	MapTileWidget* m_tiles;
	QLabel* m_mapSize;

	MainWindow* m_mainWindow;
	std::shared_ptr<Project> m_project;
	std::shared_ptr<MapLayer> m_layer;

	ToolWidget* m_selectMode;
	ToolWidget* m_penMode;
	ToolWidget* m_rectMode;
	ToolWidget* m_fillRectMode;
	ToolWidget* m_lineMode;
	ToolWidget* m_fillMode;
	ToolWidget* m_zoomInMode;
	ToolWidget* m_zoomOutMode;

public:
	EffectLayerView(MainWindow* parent, std::shared_ptr<Project> project,
		std::shared_ptr<MapLayer> layer);

	void UpdateView();
	void UpdateToolState();

	std::shared_ptr<MapLayer> GetEffectLayer() const { return m_layer; }
	MapEditorWidget* GetEditor() const { return m_editor; }

	virtual void Cut() override;
	virtual void Copy() override;
	virtual void Paste() override;
	virtual void SelectAll() override;
};
