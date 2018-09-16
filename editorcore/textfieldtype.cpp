#include <QVBoxLayout>
#include "textfieldtype.h"
#include "theme.h"

using namespace std;


TextFieldEdit::TextFieldEdit(const function<void()>& focusOutFunc): m_focusOutFunc(focusOutFunc)
{
}


void TextFieldEdit::focusOutEvent(QFocusEvent*)
{
	m_focusOutFunc();
}


TextFieldEditorWidget::TextFieldEditorWidget(const shared_ptr<ActorFieldValue>& value): m_value(value)
{
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);

	m_text = new TextFieldEdit([=]() {
		OnTextCommit();
	});

	m_text->setAcceptRichText(false);
	try
	{
		if (!value->GetValue().isNull())
			m_text->setText(QString::fromStdString(value->GetValue().asString()));
	}
	catch (Json::Exception)
	{
	}

	layout->addWidget(m_text);
	setLayout(layout);

	QPalette style(m_text->palette());
	style.setColor(QPalette::Text, Theme::content);
	m_text->setPalette(style);

	connect(m_text, &QTextEdit::textChanged, this, &TextFieldEditorWidget::OnTextEdited);
}


void TextFieldEditorWidget::OnTextCommit()
{
	m_value->SetValue(m_text->toPlainText().toStdString());

	QPalette style(m_text->palette());
	style.setColor(QPalette::Text, Theme::content);
	m_text->setPalette(style);
}


void TextFieldEditorWidget::OnTextEdited()
{
	QPalette style(m_text->palette());
	style.setColor(QPalette::Text, Theme::orange);
	m_text->setPalette(style);
}


string TextFieldType::GetName()
{
	return "text";
}


string TextFieldType::GetDescription()
{
	return "Text";
}


Json::Value TextFieldType::GetDefaultParameters()
{
	return "";
}


Json::Value TextFieldType::GetDefaultValue(const Json::Value& params)
{
	return params;
}


QWidget* TextFieldType::CreateParameterEditor(MainWindow*, const shared_ptr<Project>&,
	const shared_ptr<ActorFieldValue>& value)
{
	return new TextFieldEditorWidget(value);
}


QWidget* TextFieldType::CreateInstanceEditor(MainWindow*, const shared_ptr<Project>&, const shared_ptr<Map>&,
	const Json::Value&, const shared_ptr<ActorFieldValue>& value)
{
	return new TextFieldEditorWidget(value);
}


void TextFieldType::Register()
{
	ActorFieldType::Register(new TextFieldType());
}
