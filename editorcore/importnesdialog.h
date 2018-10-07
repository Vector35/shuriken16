#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QCheckBox>
#include "project.h"
#include "tileset.h"

class ImportNESDialog: public QDialog
{
	Q_OBJECT

	QLineEdit* m_name;
	QLineEdit* m_path;
	QComboBox* m_palette;
	QSpinBox* m_width;
	QSpinBox* m_height;
	QCheckBox* m_horizontalLayout;

	std::shared_ptr<Project> m_project;
	std::shared_ptr<TileSet> m_tileSet;
	std::vector<std::shared_ptr<Palette>> m_palettes;

public:
	ImportNESDialog(QWidget* parent, std::shared_ptr<Project> project);

	std::shared_ptr<TileSet> GetResult() const { return m_tileSet; }

	virtual QSize sizeHint() const override;

private slots:
	void OKButton();
	void BrowseButton();
};
