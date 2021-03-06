#pragma once

#include <QLineEdit>
#include <QComboBox>
#include "actortype.h"

class ChoiceFieldParameterEditorWidget: public QWidget
{
	std::shared_ptr<ActorFieldValue> m_value;
	QLineEdit* m_text;

public:
	ChoiceFieldParameterEditorWidget(const std::shared_ptr<ActorFieldValue>& value);

private slots:
	void OnTextCommit();
	void OnTextEdited(const QString& text);
};

class ChoiceFieldInstanceEditorWidget: public QWidget
{
	std::shared_ptr<ActorFieldValue> m_value;
	QStringList m_choices;
	QComboBox* m_combo;

public:
	ChoiceFieldInstanceEditorWidget(const QStringList& choices,
		const std::shared_ptr<ActorFieldValue>& value);

private slots:
	void OnValueChanged(int value);
};

class ChoiceFieldType: public ActorFieldType
{
public:
	virtual std::string GetName() override;
	virtual std::string GetDescription() override;
	virtual Json::Value GetDefaultParameters() override;
	virtual Json::Value GetDefaultValue(const Json::Value& params) override;
	virtual QWidget* CreateParameterEditor(MainWindow* mainWindow, const std::shared_ptr<Project>& project,
		const std::shared_ptr<ActorFieldValue>& value) override;
	virtual QWidget* CreateInstanceEditor(MainWindow* mainWindow, const std::shared_ptr<Project>& project,
		const std::shared_ptr<Map>& map, const Json::Value& params,
		const std::shared_ptr<ActorFieldValue>& value) override;

	static void Register();
};
