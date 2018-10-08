#include <QVBoxLayout>
#include "spritefieldtype.h"
#include "theme.h"
#include "project.h"

using namespace std;


SpriteFieldEditorWidget::SpriteFieldEditorWidget(const shared_ptr<Project>& project,
	const shared_ptr<ActorFieldValue>& value): m_value(value)
{
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);

	for (auto& i : project->GetSprites())
		m_sprites.push_back(i.second);
	sort(m_sprites.begin(), m_sprites.end(), [&](const shared_ptr<Sprite>& a, const shared_ptr<Sprite>& b) {
		return a->GetName() < b->GetName();
	});
	m_sprites.insert(m_sprites.begin(), shared_ptr<Sprite>());

	QStringList choices;
	for (auto& i : m_sprites)
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
			shared_ptr<Sprite> sprite = project->GetSpriteById(value->GetValue().asString());
			for (size_t i = 0; i < m_sprites.size(); i++)
			{
				if (m_sprites[i] == sprite)
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
		&SpriteFieldEditorWidget::OnValueChanged);
}


void SpriteFieldEditorWidget::OnValueChanged(int value)
{
	if ((value < 0) || (value >= (int)m_sprites.size()))
		return;
	shared_ptr<Sprite> sprite = m_sprites[value];
	if (sprite)
		m_value->SetValue(sprite->GetId());
	else
		m_value->SetValue("");
}


string SpriteFieldType::GetName()
{
	return "sprite";
}


string SpriteFieldType::GetDescription()
{
	return "Sprite";
}


Json::Value SpriteFieldType::GetDefaultParameters()
{
	return Json::Value();
}


Json::Value SpriteFieldType::GetDefaultValue(const Json::Value&)
{
	return "";
}


QWidget* SpriteFieldType::CreateParameterEditor(MainWindow*, const shared_ptr<Project>&,
	const shared_ptr<ActorFieldValue>&)
{
	return nullptr;
}


QWidget* SpriteFieldType::CreateInstanceEditor(MainWindow*, const shared_ptr<Project>& project, const shared_ptr<Map>&,
	const Json::Value&, const shared_ptr<ActorFieldValue>& value)
{
	return new SpriteFieldEditorWidget(project, value);
}


void SpriteFieldType::Register()
{
	ActorFieldType::Register(new SpriteFieldType());
}
