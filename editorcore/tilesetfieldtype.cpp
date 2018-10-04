#include <QVBoxLayout>
#include "tilesetfieldtype.h"
#include "theme.h"
#include "project.h"

using namespace std;


TileSetFieldEditorWidget::TileSetFieldEditorWidget(const shared_ptr<Project>& project,
	const shared_ptr<ActorFieldValue>& value): m_value(value)
{
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);

	for (auto& i : project->GetTileSets())
		m_tileSets.push_back(i.second);
	sort(m_tileSets.begin(), m_tileSets.end(), [&](const shared_ptr<TileSet>& a, const shared_ptr<TileSet>& b) {
		return a->GetName() < b->GetName();
	});
	m_tileSets.insert(m_tileSets.begin(), shared_ptr<TileSet>());

	QStringList choices;
	for (auto& i : m_tileSets)
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
			shared_ptr<TileSet> tileSet = project->GetTileSetById(value->GetValue().asString());
			for (size_t i = 0; i < m_tileSets.size(); i++)
			{
				if (m_tileSets[i] == tileSet)
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
		&TileSetFieldEditorWidget::OnValueChanged);
}


void TileSetFieldEditorWidget::OnValueChanged(int value)
{
	if ((value < 0) || (value >= (int)m_tileSets.size()))
		return;
	shared_ptr<TileSet> tileSet = m_tileSets[value];
	if (tileSet)
		m_value->SetValue(tileSet->GetId());
	else
		m_value->SetValue("");
}


string TileSetFieldType::GetName()
{
	return "tile_set";
}


string TileSetFieldType::GetDescription()
{
	return "Tile Set";
}


Json::Value TileSetFieldType::GetDefaultParameters()
{
	return Json::Value();
}


Json::Value TileSetFieldType::GetDefaultValue(const Json::Value&)
{
	return "";
}


QWidget* TileSetFieldType::CreateParameterEditor(MainWindow*, const shared_ptr<Project>&,
	const shared_ptr<ActorFieldValue>&)
{
	return nullptr;
}


QWidget* TileSetFieldType::CreateInstanceEditor(MainWindow*, const shared_ptr<Project>& project, const shared_ptr<Map>&,
	const Json::Value&, const shared_ptr<ActorFieldValue>& value)
{
	return new TileSetFieldEditorWidget(project, value);
}


void TileSetFieldType::Register()
{
	ActorFieldType::Register(new TileSetFieldType());
}
