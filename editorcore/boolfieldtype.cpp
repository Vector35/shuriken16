#include <QVBoxLayout>
#include "boolfieldtype.h"
#include "theme.h"

using namespace std;


BoolFieldEditorWidget::BoolFieldEditorWidget(const shared_ptr<ActorFieldValue>& value): m_value(value)
{
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);

	m_check = new QCheckBox();
	try
	{
		if (!value->GetValue().isNull())
			m_check->setCheckState(value->GetValue().asBool() ? Qt::Checked : Qt::Unchecked);
	}
	catch (Json::Exception)
	{
	}

	layout->addWidget(m_check);
	setLayout(layout);

	connect(m_check, &QCheckBox::stateChanged, this, &BoolFieldEditorWidget::OnStateChanged);
}


void BoolFieldEditorWidget::OnStateChanged(int state)
{
	m_value->SetValue(state == Qt::Checked);
}


string BoolFieldType::GetName()
{
	return "bool";
}


string BoolFieldType::GetDescription()
{
	return "Boolean";
}


Json::Value BoolFieldType::GetDefaultParameters()
{
	return false;
}


Json::Value BoolFieldType::GetDefaultValue(const Json::Value& params)
{
	return params;
}


QWidget* BoolFieldType::CreateParameterEditor(MainWindow*, const shared_ptr<Project>&,
	const shared_ptr<ActorFieldValue>& value)
{
	return new BoolFieldEditorWidget(value);
}


QWidget* BoolFieldType::CreateInstanceEditor(MainWindow*, const shared_ptr<Project>&, const shared_ptr<Map>&,
	const Json::Value&, const shared_ptr<ActorFieldValue>& value)
{
	return new BoolFieldEditorWidget(value);
}


void BoolFieldType::Register()
{
	ActorFieldType::Register(new BoolFieldType());
}
