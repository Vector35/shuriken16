#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include "project.h"
#include "sprite.h"

class AddSpriteDialog: public QDialog
{
	Q_OBJECT

	QLineEdit* m_name;
	QSpinBox* m_width;
	QSpinBox* m_height;
	QComboBox* m_depth;

	std::shared_ptr<Project> m_project;
	std::shared_ptr<Sprite> m_sprite;

public:
	AddSpriteDialog(QWidget* parent, std::shared_ptr<Project> project);

	std::shared_ptr<Sprite> GetResult() const { return m_sprite; }

private slots:
	void OKButton();
};
