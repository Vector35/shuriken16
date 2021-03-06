#pragma once

#include <QLineEdit>
#include <QComboBox>
#include "actortype.h"
#include "map.h"

class MapFieldEditorWidget: public QWidget
{
	std::shared_ptr<ActorFieldValue> m_value;
	std::vector<std::shared_ptr<Map>> m_maps;
	QComboBox* m_combo;

public:
	MapFieldEditorWidget(const std::shared_ptr<Project>& project,
		const std::shared_ptr<ActorFieldValue>& value);

private slots:
	void OnValueChanged(int value);
};

class MapFieldType: public ActorFieldType
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
