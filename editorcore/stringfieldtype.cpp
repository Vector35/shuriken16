#include <QVBoxLayout>
#include "stringfieldtype.h"
#include "theme.h"

using namespace std;


StringFieldEditorWidget::StringFieldEditorWidget(const shared_ptr<ActorFieldValue>& value): m_value(value)
{
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);
	m_text = new QLineEdit();
	if (!value->GetValue().isNull())
		m_text->setText(QString::fromStdString(value->GetValue().asString()));
	layout->addWidget(m_text);
	setLayout(layout);

	QPalette style(m_text->palette());
	style.setColor(QPalette::Text, Theme::content);
	m_text->setPalette(style);

	connect(m_text, &QLineEdit::textEdited, this, &StringFieldEditorWidget::OnTextEdited);
	connect(m_text, &QLineEdit::editingFinished, this, &StringFieldEditorWidget::OnTextCommit);
}


void StringFieldEditorWidget::OnTextCommit()
{
	string value = m_text->text().toStdString();
	if (value != m_value->GetValue().asString())
		m_value->SetValue(value);

	QPalette style(m_text->palette());
	style.setColor(QPalette::Text, Theme::content);
	m_text->setPalette(style);
}


void StringFieldEditorWidget::OnTextEdited(const QString&)
{
	QPalette style(m_text->palette());
	style.setColor(QPalette::Text, Theme::orange);
	m_text->setPalette(style);
}


string StringFieldType::GetName()
{
	return "string";
}


string StringFieldType::GetDescription()
{
	return "String";
}


Json::Value StringFieldType::GetDefaultParameters()
{
	return "";
}


Json::Value StringFieldType::GetDefaultValue(const Json::Value& params)
{
	return params;
}


QWidget* StringFieldType::CreateParameterEditor(MainWindow*, const shared_ptr<Project>&,
	const shared_ptr<ActorFieldValue>& value)
{
	return new StringFieldEditorWidget(value);
}


QWidget* StringFieldType::CreateInstanceEditor(MainWindow*, const shared_ptr<Project>&, const shared_ptr<Map>&,
	const shared_ptr<ActorFieldValue>& value)
{
	return new StringFieldEditorWidget(value);
}


void StringFieldType::Register()
{
	ActorFieldType::Register(new StringFieldType());
}
