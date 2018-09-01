#include <QVBoxLayout>
#include "choicefieldtype.h"
#include "theme.h"

using namespace std;


ChoiceFieldParameterEditorWidget::ChoiceFieldParameterEditorWidget(const shared_ptr<ActorFieldValue>& value): m_value(value)
{
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);

	m_text = new QLineEdit();
	try
	{
		if (!value->GetValue().isNull())
		{
			string text;
			bool first = true;
			for (auto& i : value->GetValue())
			{
				if (!first)
					text += ",";
				text += i.asString();
				first = false;
			}
			m_text->setText(QString::fromStdString(text));
		}
	}
	catch (Json::Exception)
	{
	}

	layout->addWidget(m_text);
	setLayout(layout);

	QPalette style(m_text->palette());
	style.setColor(QPalette::Text, Theme::content);
	m_text->setPalette(style);

	connect(m_text, &QLineEdit::textEdited, this, &ChoiceFieldParameterEditorWidget::OnTextEdited);
	connect(m_text, &QLineEdit::editingFinished, this, &ChoiceFieldParameterEditorWidget::OnTextCommit);
}


void ChoiceFieldParameterEditorWidget::OnTextCommit()
{
	string value = m_text->text().toStdString();
	Json::Value choices(Json::arrayValue);
	if (value.size() != 0)
	{
		size_t prev = string::npos;
		for (size_t i = 0; i < value.size(); i++)
		{
			if (value[i] == ',')
			{
				if (prev == string::npos)
					choices.append(value.substr(0, i));
				else
					choices.append(value.substr(prev, i - prev));
				prev = i + 1;
			}
		}
		if ((prev == string::npos) || (prev < value.size()))
		{
			if (prev == string::npos)
				choices.append(value.substr(0));
			else
				choices.append(value.substr(prev));
		}
	}

	m_value->SetValue(choices);

	QPalette style(m_text->palette());
	style.setColor(QPalette::Text, Theme::content);
	m_text->setPalette(style);
}


void ChoiceFieldParameterEditorWidget::OnTextEdited(const QString&)
{
	QPalette style(m_text->palette());
	style.setColor(QPalette::Text, Theme::orange);
	m_text->setPalette(style);
}


ChoiceFieldInstanceEditorWidget::ChoiceFieldInstanceEditorWidget(const QStringList& choices,
	const shared_ptr<ActorFieldValue>& value): m_value(value), m_choices(choices)
{
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);

	m_combo = new QComboBox();
	m_combo->setEditable(false);
	m_combo->addItems(m_choices);
	try
	{
		if (!value->GetValue().isNull())
		{
			QString choice = QString::fromStdString(value->GetValue().asString());
			for (size_t i = 0; i < m_choices.size(); i++)
			{
				if (m_choices[i] == choice)
					m_combo->setCurrentIndex(i);
			}
		}
	}
	catch (Json::Exception)
	{
	}

	layout->addWidget(m_combo);
	setLayout(layout);

	connect(m_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&ChoiceFieldInstanceEditorWidget::OnValueChanged);
}


void ChoiceFieldInstanceEditorWidget::OnValueChanged(int value)
{
	if ((value < 0) || (value >= m_choices.size()))
		return;
	m_value->SetValue(m_choices[value].toStdString());
}


string ChoiceFieldType::GetName()
{
	return "choice";
}


string ChoiceFieldType::GetDescription()
{
	return "Choice";
}


Json::Value ChoiceFieldType::GetDefaultParameters()
{
	return Json::Value(Json::arrayValue);
}


Json::Value ChoiceFieldType::GetDefaultValue(const Json::Value& params)
{
	try
	{
		if (params.isNull())
			return "";
		if (params.size() == 0)
			return "";
		return params[0];
	}
	catch (Json::Exception)
	{
		return "";
	}
}


QWidget* ChoiceFieldType::CreateParameterEditor(MainWindow*, const shared_ptr<Project>&,
	const shared_ptr<ActorFieldValue>& value)
{
	return new ChoiceFieldParameterEditorWidget(value);
}


QWidget* ChoiceFieldType::CreateInstanceEditor(MainWindow*, const shared_ptr<Project>&, const shared_ptr<Map>&,
	const Json::Value& params, const shared_ptr<ActorFieldValue>& value)
{
	QStringList choices;
	try
	{
		for (auto& i : params)
			choices.append(QString::fromStdString(i.asString()));
	}
	catch (Json::Exception)
	{
	}

	return new ChoiceFieldInstanceEditorWidget(choices, value);
}


void ChoiceFieldType::Register()
{
	ActorFieldType::Register(new ChoiceFieldType());
}
