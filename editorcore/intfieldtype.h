#pragma once

#include <QLineEdit>
#include "actortype.h"

class IntFieldEditorWidget: public QWidget
{
	std::shared_ptr<ActorFieldValue> m_value;
	QLineEdit* m_text;

public:
	IntFieldEditorWidget(const std::shared_ptr<ActorFieldValue>& value);

private slots:
	void OnTextCommit();
	void OnTextEdited(const QString& text);
};

class IntFieldType: public ActorFieldType
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
