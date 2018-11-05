#pragma once

#include <QCheckBox>
#include "actortype.h"

class BoolFieldEditorWidget: public QWidget
{
	std::shared_ptr<ActorFieldValue> m_value;
	QCheckBox* m_check;

public:
	BoolFieldEditorWidget(const std::shared_ptr<ActorFieldValue>& value);

private slots:
	void OnStateChanged(int state);
};

class BoolFieldType: public ActorFieldType
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
