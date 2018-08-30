#include <QVBoxLayout>
#include <QGuiApplication>
#include <QStyle>
#include <QLabel>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>
#include "mapactorwidget.h"
#include "theme.h"
#include "mapeditorwidget.h"
#include "mainwindow.h"
#include "actoritemwidget.h"

using namespace std;


MapActorWidget::MapActorWidget(QWidget* parent, MapEditorWidget* editor, MainWindow* mainWindow,
	shared_ptr<Project> project, shared_ptr<Map> map):
	QWidget(parent), m_mainWindow(mainWindow), m_editor(editor), m_project(project), m_map(map)
{
	m_positionValid = false;

	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);
	m_entryLayout = new QGridLayout();
	m_entryLayout->setSpacing(4);
	m_entryLayout->setColumnStretch(1, 1);
	layout->addLayout(m_entryLayout);
	setLayout(layout);

	QFontMetrics metrics(QGuiApplication::font());
	setMinimumSize(256 + 16 + style()->pixelMetric(QStyle::PM_ScrollBarExtent), 0);
}


void MapActorWidget::RemoveActor(shared_ptr<Actor> actor)
{
	size_t i = m_map->RemoveActor(actor);
	m_mainWindow->UpdateMapContents(m_map);

	MainWindow* mainWindow = m_mainWindow;
	shared_ptr<Map> map = m_map;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			map->InsertActor(i, actor);
			mainWindow->UpdateMapContents(map);
		},
		[=]() { // Redo
			map->RemoveActor(actor);
			mainWindow->UpdateMapContents(map);
		}
	);
}


void MapActorWidget::UpdateView()
{
	for (auto i : m_entries)
	{
		m_entryLayout->removeWidget(i);
		i->deleteLater();
	}
	m_entries.clear();

	int row = 0;

	if (!m_positionValid)
	{
		QLabel* message = new QLabel("Mark a location to edit actors.");
		QFont messageFont = QGuiApplication::font();
		messageFont.setItalic(true);
		message->setFont(messageFont);
		m_entryLayout->addWidget(message, row, 0, 1, 2);
		m_entries.push_back(message);
		return;
	}

	QLabel* actorHeader = new QLabel("Actors");
	QFont headerFont = QGuiApplication::font();
	headerFont.setPointSize(headerFont.pointSize() * 5 / 4);
	actorHeader->setFont(headerFont);
	m_entryLayout->addWidget(actorHeader, row, 0, 1, 2);
	m_entries.push_back(actorHeader);
	row++;

	vector<shared_ptr<Actor>> selectedActors;
	for (auto& i : m_map->GetActors())
	{
		if ((i->GetX() < (m_x + m_width)) && (m_x < (i->GetX() + i->GetWidth())) &&
			(i->GetY() < (m_y + m_height)) && (m_y < (i->GetY() + i->GetHeight())))
		{
			selectedActors.push_back(i);
		}
	}

	if (selectedActors.size() == 0)
	{
		QLabel* message = new QLabel("No actors in selected region.");
		QFont messageFont = QGuiApplication::font();
		messageFont.setItalic(true);
		message->setFont(messageFont);
		m_entryLayout->addWidget(message, row, 0, 1, 2);
		m_entries.push_back(message);
		row++;

		QPushButton* newActor = new QPushButton("Add...");
		m_entryLayout->addWidget(newActor, row, 0);
		m_entries.push_back(newActor);
		connect(newActor, &QPushButton::clicked, this, &MapActorWidget::OnCreateActor);

		if (m_selectedActor)
		{
			m_selectedActor.reset();
			m_editor->UpdateView();
		}
		return;
	}

	if (!m_selectedActor)
	{
		m_selectedActor = selectedActors[0];
		m_editor->UpdateView();
	}

	bool hasSelection = false;
	for (auto& i : selectedActors)
	{
		bool isSelected = m_selectedActor == i;
		if (isSelected)
			hasSelection = true;

		ActorItemWidget* widget = new ActorItemWidget(this, i->GetType()->GetName(), isSelected,
			[=]() {
				m_selectedActor = i;
				m_editor->UpdateView();
				UpdateView();
			},
			[=]() { RemoveActor(i); }
		);

		m_entryLayout->addWidget(widget, row, 0, 1, 2);
		m_entries.push_back(widget);
		row++;
	}

	if (!hasSelection)
	{
		m_selectedActor.reset();
		m_editor->UpdateView();
	}

	QPushButton* newActor = new QPushButton("Add...");
	m_entryLayout->addWidget(newActor, row, 0);
	m_entries.push_back(newActor);
	connect(newActor, &QPushButton::clicked, this, &MapActorWidget::OnCreateActor);
	row++;

	shared_ptr<Actor> actor = m_selectedActor;
	if (!actor)
		return;

	QLabel* propertyHeader = new QLabel("Properties");
	propertyHeader->setFont(headerFont);
	m_entryLayout->addWidget(propertyHeader, row, 0, 1, 2);
	m_entries.push_back(propertyHeader);
	row++;

	shared_ptr<ActorType> type = actor->GetType();
	if (type->GetFieldCount() == 0)
	{
		QLabel* message = new QLabel("No properties for this actor.");
		QFont messageFont = QGuiApplication::font();
		messageFont.setItalic(true);
		message->setFont(messageFont);
		m_entryLayout->addWidget(message, row, 0, 1, 2);
		m_entries.push_back(message);
		return;
	}

	for (auto& i : type->GetFields())
	{
		QLabel* nameLabel = new QLabel(QString::fromStdString(i.name));
		m_entryLayout->addWidget(nameLabel, row, 0);
		m_entries.push_back(nameLabel);

		ActorField field = i;
		shared_ptr<ActorFieldValue> value = make_shared<ActorFieldValue>(actor->GetFieldValue(i.name),
			[=](const Json::Value& oldValue, const Json::Value& newValue) {
				actor->SetFieldValue(field.name, newValue);
				m_mainWindow->UpdateMapContents(m_map);

				MainWindow* mainWindow = m_mainWindow;
				shared_ptr<Map> map = m_map;
				m_mainWindow->AddUndoAction(
					[=]() { // Undo
						actor->SetFieldValue(field.name, oldValue);
						mainWindow->UpdateMapContents(map);
					},
					[=]() { // Redo
						actor->SetFieldValue(field.name, newValue);
						mainWindow->UpdateMapContents(map);
					}
				);
			});

		QWidget* widget = i.type->CreateInstanceEditor(m_mainWindow, m_project, m_map, value);
		if (widget)
		{
			m_entryLayout->addWidget(widget, row, 1);
			m_entries.push_back(widget);
		}
		else
		{
			QLabel* message = new QLabel("No editor for this field.");
			QFont messageFont = QGuiApplication::font();
			messageFont.setItalic(true);
			message->setFont(messageFont);
			m_entryLayout->addWidget(message, row, 1);
			m_entries.push_back(message);
		}

		row++;
	}
}


void MapActorWidget::ClearSelection()
{
	m_positionValid = false;
	m_selectedActor.reset();
	UpdateView();
}


void MapActorWidget::SetSelection(size_t x, size_t y, size_t width, size_t height)
{
	m_positionValid = true;
	m_x = x;
	m_y = y;
	m_width = width;
	m_height = height;
	UpdateView();
}


void MapActorWidget::OnCreateActor()
{
	if (!m_positionValid)
		return;

	bool requiresBounds = (m_width != 1) || (m_height != 1);

	QStringList actorTypeNames;
	map<QString, shared_ptr<ActorType>> actorTypes;

	vector<shared_ptr<ActorType>> sortedActorTypes;
	for (auto& i : m_project->GetActorTypes())
	{
		if ((!requiresBounds) || i.second->HasBounds())
			sortedActorTypes.push_back(i.second);
	}
	sort(sortedActorTypes.begin(), sortedActorTypes.end(), [&](const shared_ptr<ActorType>& a, const shared_ptr<ActorType>& b) {
		return a->GetName() < b->GetName();
	});

	if (sortedActorTypes.size() == 0)
	{
		QMessageBox::critical(this, "Error", "No valid actor types are available.");
		return;
	}

	int selected = 0;
	int cur = 0;
	for (auto& i : sortedActorTypes)
	{
		QString name = QString::fromStdString(i->GetName());
		actorTypeNames.append(name);
		actorTypes[name] = i;

		if (i == m_lastActorType)
			selected = cur;
		cur++;
	}

	bool ok;
	QString chosenName = QInputDialog::getItem(this, "Create Actor", "Actor Type:", actorTypeNames, selected, false, &ok);
	if (!ok)
		return;

	auto i = actorTypes.find(chosenName);
	if (i == actorTypes.end())
		return;
	shared_ptr<ActorType> type = i->second;
	m_lastActorType = type;

	shared_ptr<Actor> actor = make_shared<Actor>(type, m_x, m_y, m_width, m_height);
	m_map->AddActor(actor);
	m_mainWindow->UpdateMapContents(m_map);

	MainWindow* mainWindow = m_mainWindow;
	shared_ptr<Map> map = m_map;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			map->RemoveActor(actor);
			mainWindow->UpdateMapContents(map);
		},
		[=]() { // Redo
			map->AddActor(actor);
			mainWindow->UpdateMapContents(map);
		}
	);
}
