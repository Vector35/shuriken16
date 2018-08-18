#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include "project.h"
#include "palette.h"

class AddPaletteDialog: public QDialog
{
	Q_OBJECT

	QLineEdit* m_name;
	QSpinBox* m_size;

	std::shared_ptr<Project> m_project;
	std::shared_ptr<Palette> m_palette;

public:
	AddPaletteDialog(QWidget* parent, std::shared_ptr<Project> project);

	std::shared_ptr<Palette> GetResult() const { return m_palette; }

private slots:
	void OKButton();
};
