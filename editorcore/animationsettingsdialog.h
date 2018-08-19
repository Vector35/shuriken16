#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include "project.h"
#include "spriteanimation.h"

class AnimationSettingsDialog: public QDialog
{
	Q_OBJECT

	QLineEdit* m_name;
	QCheckBox* m_looping;

public:
	AnimationSettingsDialog(QWidget* parent, const std::shared_ptr<SpriteAnimation>& anim);

	std::string GetName();
	bool IsLooping();
};
