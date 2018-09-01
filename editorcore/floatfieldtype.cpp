#include <QVBoxLayout>
#include <QIntValidator>
#include "floatfieldtype.h"
#include "theme.h"

using namespace std;


FloatFieldEditorWidget::FloatFieldEditorWidget(const shared_ptr<ActorFieldValue>& value): m_value(value)
{
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);

	m_text = new QLineEdit();
	try
	{
		if (!value->GetValue().isNull())
			m_text->setText(QString::number(value->GetValue().asDouble()));
	}
	catch (Json::Exception)
	{
	}

	layout->addWidget(m_text);
	setLayout(layout);

	QPalette style(m_text->palette());
	style.setColor(QPalette::Text, Theme::content);
	m_text->setPalette(style);

	connect(m_text, &QLineEdit::textEdited, this, &FloatFieldEditorWidget::OnTextEdited);
	connect(m_text, &QLineEdit::editingFinished, this, &FloatFieldEditorWidget::OnTextCommit);
}


void FloatFieldEditorWidget::OnTextCommit()
{
	bool ok;
	double value = m_text->text().toDouble(&ok);
	if (!ok)
		return;
	m_value->SetValue(value);

	QPalette style(m_text->palette());
	style.setColor(QPalette::Text, Theme::content);
	m_text->setPalette(style);
}


void FloatFieldEditorWidget::OnTextEdited(const QString&)
{
	QPalette style(m_text->palette());
	style.setColor(QPalette::Text, Theme::orange);
	m_text->setPalette(style);
}


string FloatFieldType::GetName()
{
	return "float";
}


string FloatFieldType::GetDescription()
{
	return "Float";
}


Json::Value FloatFieldType::GetDefaultParameters()
{
	return (double)0;
}


Json::Value FloatFieldType::GetDefaultValue(const Json::Value& params)
{
	return params;
}


QWidget* FloatFieldType::CreateParameterEditor(MainWindow*, const shared_ptr<Project>&,
	const shared_ptr<ActorFieldValue>& value)
{
	return new FloatFieldEditorWidget(value);
}


QWidget* FloatFieldType::CreateInstanceEditor(MainWindow*, const shared_ptr<Project>&, const shared_ptr<Map>&,
	const Json::Value&, const shared_ptr<ActorFieldValue>& value)
{
	return new FloatFieldEditorWidget(value);
}


void FloatFieldType::Register()
{
	ActorFieldType::Register(new FloatFieldType());
}
