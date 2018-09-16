#pragma once

#include <QTextEdit>
#include "actortype.h"

class TextFieldEdit: public QTextEdit
{
	std::function<void()> m_focusOutFunc;

public:
	TextFieldEdit(const std::function<void()>& focusOutFunc);

protected:
	virtual void focusOutEvent(QFocusEvent* event) override;
};

class TextFieldEditorWidget: public QWidget
{
	std::shared_ptr<ActorFieldValue> m_value;
	QTextEdit* m_text;

public:
	TextFieldEditorWidget(const std::shared_ptr<ActorFieldValue>& value);

private slots:
	void OnTextCommit();
	void OnTextEdited();
};

class TextFieldType: public ActorFieldType
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
