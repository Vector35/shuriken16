#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include "project.h"
#include "maplayer.h"

class LayerSettingsDialog: public QDialog
{
	Q_OBJECT

	QLineEdit* m_name;
	QComboBox* m_blendMode;
	QSlider* m_alpha;

	std::shared_ptr<MapLayer> m_layer;

public:
	LayerSettingsDialog(QWidget* parent, std::shared_ptr<MapLayer> layer);

	std::shared_ptr<MapLayer> GetResult() const { return m_layer; }

	std::string GetName();
	BlendMode GetBlendMode();
	uint8_t GetAlpha();
};
