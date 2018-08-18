#pragma once

#include <QWidget>
#include <QComboBox>
#include <QSlider>
#include <QLineEdit>
#include <memory>
#include "project.h"
#include "maplayer.h"

class MainWindow;
class MapEditorWidget;

class EffectLayerSettingsWidget: public QWidget
{
	Q_OBJECT

	MapEditorWidget* m_editor;
	MainWindow* m_mainWindow;
	std::shared_ptr<Project> m_project;
	std::shared_ptr<MapLayer> m_layer;

	QComboBox* m_blendMode;
	QSlider* m_alpha;
	QLineEdit* m_parallaxFactorX;
	QLineEdit* m_parallaxFactorY;
	QLineEdit* m_autoScrollX;
	QLineEdit* m_autoScrollY;

public:
	EffectLayerSettingsWidget(QWidget* parent, MapEditorWidget* editor, MainWindow* mainWindow,
		std::shared_ptr<Project> project, std::shared_ptr<MapLayer> layer);

	void UpdateView();

private slots:
	void OnBlendModeChanged(int value);
	void OnAlphaChanged(int value);
	void OnParallaxFactorXChanged();
	void OnParallaxFactorYChanged();
	void OnAutoScrollXChanged();
	void OnAutoScrollYChanged();
};
