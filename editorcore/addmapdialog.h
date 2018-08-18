#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include "project.h"
#include "map.h"

class AddMapDialog: public QDialog
{
	Q_OBJECT

	QLineEdit* m_name;
	QSpinBox* m_width;
	QSpinBox* m_height;
	QComboBox* m_tileWidth;
	QComboBox* m_tileHeight;
	QComboBox* m_tileDepth;

	std::shared_ptr<Project> m_project;
	std::shared_ptr<Map> m_map;

public:
	AddMapDialog(QWidget* parent, std::shared_ptr<Project> project);

	std::shared_ptr<Map> GetResult() const { return m_map; }

private slots:
	void OKButton();
};
