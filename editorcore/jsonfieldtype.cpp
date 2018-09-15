#include <QVBoxLayout>
#include "jsonfieldtype.h"
#include "theme.h"

using namespace std;


JsonFieldEditorWidget::JsonFieldEditorWidget(const shared_ptr<ActorFieldValue>& value): m_value(value)
{
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);

	m_text = new QLineEdit();
	try
	{
		Json::FastWriter writer;
		string valueStr = writer.write(value->GetValue());
		m_text->setText(QString::fromStdString(valueStr));
	}
	catch (Json::Exception)
	{
	}

	layout->addWidget(m_text);
	setLayout(layout);

	QPalette style(m_text->palette());
	style.setColor(QPalette::Text, Theme::content);
	m_text->setPalette(style);

	connect(m_text, &QLineEdit::textEdited, this, &JsonFieldEditorWidget::OnTextEdited);
	connect(m_text, &QLineEdit::editingFinished, this, &JsonFieldEditorWidget::OnTextCommit);
}


void JsonFieldEditorWidget::OnTextCommit()
{
	Json::Reader reader;
	Json::Value value;
	if (!reader.parse(m_text->text().toStdString(), value, false))
		return;
	m_value->SetValue(value);

	QPalette style(m_text->palette());
	style.setColor(QPalette::Text, Theme::content);
	m_text->setPalette(style);
}


void JsonFieldEditorWidget::OnTextEdited(const QString&)
{
	QPalette style(m_text->palette());
	style.setColor(QPalette::Text, Theme::orange);
	m_text->setPalette(style);
}


string JsonFieldType::GetName()
{
	return "json";
}


string JsonFieldType::GetDescription()
{
	return "JSON";
}


Json::Value JsonFieldType::GetDefaultParameters()
{
	return Json::Value();
}


Json::Value JsonFieldType::GetDefaultValue(const Json::Value& params)
{
	return params;
}


QWidget* JsonFieldType::CreateParameterEditor(MainWindow*, const shared_ptr<Project>&,
	const shared_ptr<ActorFieldValue>& value)
{
	return new JsonFieldEditorWidget(value);
}


QWidget* JsonFieldType::CreateInstanceEditor(MainWindow*, const shared_ptr<Project>&, const shared_ptr<Map>&,
	const Json::Value&, const shared_ptr<ActorFieldValue>& value)
{
	return new JsonFieldEditorWidget(value);
}


void JsonFieldType::Register()
{
	ActorFieldType::Register(new JsonFieldType());
}
