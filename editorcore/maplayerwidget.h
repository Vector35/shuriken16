#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QCheckBox>
#include "project.h"
#include "map.h"

class MainWindow;
class MapEditorWidget;

class MapLayerWidget: public QWidget
{
	Q_OBJECT

	MainWindow* m_mainWindow;
	MapEditorWidget* m_editor;
	std::shared_ptr<Project> m_project;
	std::shared_ptr<Map> m_map;

	QVBoxLayout* m_entryLayout;
	std::vector<QWidget*> m_entries;

	QCheckBox* m_animate;
	QCheckBox* m_fadeOtherLayers;

	void MoveLayerUp(std::shared_ptr<MapLayer> layer);
	void MoveLayerDown(std::shared_ptr<MapLayer> layer);
	void EditLayer(std::shared_ptr<MapLayer> layer);
	void RemoveLayer(std::shared_ptr<MapLayer> layer);

public:
	MapLayerWidget(QWidget* parent, MapEditorWidget* editor, MainWindow* mainWindow,
		std::shared_ptr<Project> project, std::shared_ptr<Map> map);

	void UpdateView();

private slots:
	void OnAddLayer();
	void OnAddEffectLayer();
	void OnFadeOtherLayersChanged(int state);
	void OnAnimationChanged(int state);
};
