#include <QVBoxLayout>
#include <QIntValidator>
#include "intfieldtype.h"
#include "theme.h"

using namespace std;


IntFieldEditorWidget::IntFieldEditorWidget(const shared_ptr<ActorFieldValue>& value): m_value(value)
{
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);

	m_text = new QLineEdit();
	try
	{
		if (!value->GetValue().isNull())
			m_text->setText(QString::number(value->GetValue().asInt64()));
	}
	catch (Json::Exception)
	{
	}

	layout->addWidget(m_text);
	setLayout(layout);

	QPalette style(m_text->palette());
	style.setColor(QPalette::Text, Theme::content);
	m_text->setPalette(style);

	connect(m_text, &QLineEdit::textEdited, this, &IntFieldEditorWidget::OnTextEdited);
	connect(m_text, &QLineEdit::editingFinished, this, &IntFieldEditorWidget::OnTextCommit);
}


void IntFieldEditorWidget::OnTextCommit()
{
	bool ok;
	int64_t value = m_text->text().toLongLong(&ok, 0);
	if (!ok)
		return;
	m_value->SetValue(value);

	QPalette style(m_text->palette());
	style.setColor(QPalette::Text, Theme::content);
	m_text->setPalette(style);
}


void IntFieldEditorWidget::OnTextEdited(const QString&)
{
	QPalette style(m_text->palette());
	style.setColor(QPalette::Text, Theme::orange);
	m_text->setPalette(style);
}


string IntFieldType::GetName()
{
	return "int";
}


string IntFieldType::GetDescription()
{
	return "Integer";
}


Json::Value IntFieldType::GetDefaultParameters()
{
	return (int64_t)0;
}


Json::Value IntFieldType::GetDefaultValue(const Json::Value& params)
{
	return params;
}


QWidget* IntFieldType::CreateParameterEditor(MainWindow*, const shared_ptr<Project>&,
	const shared_ptr<ActorFieldValue>& value)
{
	return new IntFieldEditorWidget(value);
}


QWidget* IntFieldType::CreateInstanceEditor(MainWindow*, const shared_ptr<Project>&, const shared_ptr<Map>&,
	const Json::Value&, const shared_ptr<ActorFieldValue>& value)
{
	return new IntFieldEditorWidget(value);
}


void IntFieldType::Register()
{
	ActorFieldType::Register(new IntFieldType());
}
