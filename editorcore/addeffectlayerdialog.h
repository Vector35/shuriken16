#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include "project.h"
#include "maplayer.h"

class AddEffectLayerDialog: public QDialog
{
	Q_OBJECT

	QLineEdit* m_name;
	QSpinBox* m_width;
	QSpinBox* m_height;
	QSpinBox* m_tileWidth;
	QSpinBox* m_tileHeight;
	QComboBox* m_tileDepth;

	std::shared_ptr<Project> m_project;
	std::shared_ptr<MapLayer> m_layer;

public:
	AddEffectLayerDialog(QWidget* parent, std::shared_ptr<Project> project);

	std::shared_ptr<MapLayer> GetResult() const { return m_layer; }

private slots:
	void OKButton();
};
