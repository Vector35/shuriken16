#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include "project.h"
#include "tileset.h"

class AddTileSetDialog: public QDialog
{
	Q_OBJECT

	QLineEdit* m_name;
	QComboBox* m_type;
	QLabel* m_countLabel;
	QSpinBox* m_count;
	QComboBox* m_width;
	QComboBox* m_height;
	QComboBox* m_depth;

	std::shared_ptr<Project> m_project;
	std::shared_ptr<TileSet> m_tileSet;

public:
	AddTileSetDialog(QWidget* parent, std::shared_ptr<Project> project);

	std::shared_ptr<TileSet> GetResult() const { return m_tileSet; }

private slots:
	void OKButton();
	void OnTypeChanged(int type);
};
