#include <QVBoxLayout>
#include "mapfieldtype.h"
#include "theme.h"
#include "project.h"

using namespace std;


MapFieldEditorWidget::MapFieldEditorWidget(const shared_ptr<Project>& project,
	const shared_ptr<ActorFieldValue>& value): m_value(value)
{
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);

	for (auto& i : project->GetMaps())
		m_maps.push_back(i.second);
	sort(m_maps.begin(), m_maps.end(), [&](const shared_ptr<Map>& a, const shared_ptr<Map>& b) {
		return a->GetName() < b->GetName();
	});
	m_maps.insert(m_maps.begin(), shared_ptr<Map>());

	QStringList choices;
	for (auto& i : m_maps)
	{
		if (i)
			choices.append(QString::fromStdString(i->GetName()));
		else
			choices.append("<None>");
	}

	m_combo = new QComboBox();
	m_combo->setEditable(false);
	m_combo->addItems(choices);
	try
	{
		if (!value->GetValue().isNull())
		{
			shared_ptr<Map> map = project->GetMapById(value->GetValue().asString());
			for (size_t i = 0; i < m_maps.size(); i++)
			{
				if (m_maps[i] == map)
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
		&MapFieldEditorWidget::OnValueChanged);
}


void MapFieldEditorWidget::OnValueChanged(int value)
{
	if ((value < 0) || (value >= (int)m_maps.size()))
		return;
	shared_ptr<Map> map = m_maps[value];
	if (map)
		m_value->SetValue(map->GetId());
	else
		m_value->SetValue("");
}


string MapFieldType::GetName()
{
	return "map";
}


string MapFieldType::GetDescription()
{
	return "Map";
}


Json::Value MapFieldType::GetDefaultParameters()
{
	return Json::Value();
}


Json::Value MapFieldType::GetDefaultValue(const Json::Value&)
{
	return "";
}


QWidget* MapFieldType::CreateParameterEditor(MainWindow*, const shared_ptr<Project>&,
	const shared_ptr<ActorFieldValue>&)
{
	return nullptr;
}


QWidget* MapFieldType::CreateInstanceEditor(MainWindow*, const shared_ptr<Project>& project, const shared_ptr<Map>&,
	const Json::Value&, const shared_ptr<ActorFieldValue>& value)
{
	return new MapFieldEditorWidget(project, value);
}


void MapFieldType::Register()
{
	ActorFieldType::Register(new MapFieldType());
}
