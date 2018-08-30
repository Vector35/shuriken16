#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include "actortype.h"

class AddActorTypeFieldDialog: public QDialog
{
	Q_OBJECT

	QLineEdit* m_name;
	QComboBox* m_type;
	std::vector<ActorFieldType*> m_availableTypes;

public:
	AddActorTypeFieldDialog(QWidget* parent);

	std::string GetName();
	ActorFieldType* GetType();

private slots:
	void OKButton();
};
