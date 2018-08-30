#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QGuiApplication>
#include <QInputDialog>
#include <QMessageBox>
#include <QScrollBar>
#include "actortypeview.h"
#include "theme.h"
#include "mainwindow.h"
#include "addactortypefielddialog.h"

using namespace std;


ActorTypeEditorWidget::ActorTypeEditorWidget(QWidget* parent, MainWindow* mainWindow, shared_ptr<Project> project,
	shared_ptr<ActorType> actorType):
	QWidget(parent), m_mainWindow(mainWindow), m_project(project), m_actorType(actorType)
{
	QPalette style(this->palette());
	style.setColor(QPalette::Window, Theme::background);
	setPalette(style);

	QVBoxLayout* layout = new QVBoxLayout();

	QHBoxLayout* headerLayout = new QHBoxLayout();
	m_sprite = new SpriteSelectWidget(this, project, m_actorType->GetEditorSprite(),
		[=](shared_ptr<Sprite> oldSprite, shared_ptr<Sprite> newSprite) {
			OnSpriteChanged(oldSprite, newSprite);
		});
	headerLayout->addWidget(m_sprite);
	m_name = new QLabel();
	QFont headerFont = QGuiApplication::font();
	headerFont.setPointSize(headerFont.pointSize() * 4 / 3);
	m_name->setFont(headerFont);
	headerLayout->addWidget(m_name, 1);

	m_bounds = new QCheckBox("Has bounding box");
	headerLayout->addWidget(m_bounds);
	connect(m_bounds, &QCheckBox::stateChanged, this, &ActorTypeEditorWidget::OnBoundsChanged);

	QPushButton* addFieldButton = new QPushButton("Add Field...");
	connect(addFieldButton, &QPushButton::clicked, this, &ActorTypeEditorWidget::OnAddField);
	headerLayout->addWidget(addFieldButton);
	layout->addLayout(headerLayout);

	m_fieldLayout = new QGridLayout();
	m_fieldLayout->setSpacing(16);
	m_fieldLayout->setColumnStretch(2, 1);
	layout->addLayout(m_fieldLayout);

	layout->addStretch(1);
	setLayout(layout);
	UpdateView();
}


void ActorTypeEditorWidget::OnAddField()
{
	AddActorTypeFieldDialog dialog(this);
	if (dialog.exec() != QDialog::Accepted)
		return;

	string name = dialog.GetName();
	ActorFieldType* type = dialog.GetType();

	ActorField field;
	field.name = name;
	field.type = type;
	field.params = type->GetDefaultParameters();
	size_t fieldIndex = m_actorType->GetFieldCount();
	m_actorType->AddField(field);
	m_mainWindow->UpdateActorTypeContents(m_actorType);

	MainWindow* mainWindow = m_mainWindow;
	shared_ptr<ActorType> actorType = m_actorType;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			actorType->RemoveField(fieldIndex);
			mainWindow->UpdateActorTypeContents(actorType);
		},
		[=]() { // Redo
			actorType->AddField(field);
			mainWindow->UpdateActorTypeContents(actorType);
		}
	);
}


void ActorTypeEditorWidget::OnBoundsChanged(int state)
{
	bool oldBounds = m_actorType->HasBounds();
	bool newBounds = state == Qt::Checked;
	if (oldBounds == newBounds)
		return;

	m_actorType->SetHasBounds(newBounds);
	m_mainWindow->UpdateActorTypeContents(m_actorType);

	MainWindow* mainWindow = m_mainWindow;
	shared_ptr<ActorType> actorType = m_actorType;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			actorType->SetHasBounds(oldBounds);
			mainWindow->UpdateActorTypeContents(actorType);
		},
		[=]() { // Redo
			actorType->SetHasBounds(newBounds);
			mainWindow->UpdateActorTypeContents(actorType);
		}
	);
}


void ActorTypeEditorWidget::RenameField(size_t i)
{
	bool ok;
	ActorField oldField = m_actorType->GetField(i);
	string newName = QInputDialog::getText(this, "Rename Field", "New Name:", QLineEdit::Normal,
		QString::fromStdString(oldField.name), &ok).toStdString();
	if (!ok)
		return;

	ActorField newField = oldField;
	newField.name = newName;
	m_actorType->SetField(i, newField);
	m_mainWindow->UpdateActorTypeContents(m_actorType);

	MainWindow* mainWindow = m_mainWindow;
	shared_ptr<ActorType> actorType = m_actorType;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			actorType->SetField(i, oldField);
			mainWindow->UpdateActorTypeContents(actorType);
		},
		[=]() { // Redo
			actorType->SetField(i, newField);
			mainWindow->UpdateActorTypeContents(actorType);
		}
	);
}


void ActorTypeEditorWidget::RemoveField(size_t i)
{
	ActorField field = m_actorType->GetField(i);
	if (QMessageBox::question(this, "Delete Field", QString("Are you sure you want to remove the field '") +
		QString::fromStdString(field.name) + QString("'?"), QMessageBox::Yes,
		QMessageBox::No | QMessageBox::Default | QMessageBox::Escape, QMessageBox::NoButton) !=
		QMessageBox::Yes)
		return;

	m_actorType->RemoveField(i);
	m_mainWindow->UpdateActorTypeContents(m_actorType);

	MainWindow* mainWindow = m_mainWindow;
	shared_ptr<ActorType> actorType = m_actorType;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			actorType->InsertField(i, field);
			mainWindow->UpdateActorTypeContents(actorType);
		},
		[=]() { // Redo
			actorType->RemoveField(i);
			mainWindow->UpdateActorTypeContents(actorType);
		}
	);
}


void ActorTypeEditorWidget::OnSpriteChanged(shared_ptr<Sprite> oldSprite, shared_ptr<Sprite> newSprite)
{
	m_actorType->SetEditorSprite(newSprite);
	m_mainWindow->UpdateActorTypeContents(m_actorType);

	MainWindow* mainWindow = m_mainWindow;
	shared_ptr<ActorType> actorType = m_actorType;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			actorType->SetEditorSprite(oldSprite);
			mainWindow->UpdateActorTypeContents(actorType);
		},
		[=]() { // Redo
			actorType->SetEditorSprite(newSprite);
			mainWindow->UpdateActorTypeContents(actorType);
		}
	);
}


void ActorTypeEditorWidget::UpdateView()
{
	m_sprite->SetSprite(m_actorType->GetEditorSprite());
	m_name->setText(QString("Actor Type - ") + QString::fromStdString(m_actorType->GetName()));
	m_bounds->setChecked(m_actorType->HasBounds());

	for (auto i : m_fields)
	{
		m_fieldLayout->removeWidget(i);
		i->deleteLater();
	}
	m_fields.clear();

	for (size_t i = 0; i < m_actorType->GetFieldCount(); i++)
	{
		const ActorField& field = m_actorType->GetField(i);

		QLabel* name = new QLabel(QString::fromStdString(field.name));
		m_fieldLayout->addWidget(name, (int)i, 0);
		m_fields.push_back(name);

		QLabel* type = new QLabel(QString::fromStdString(field.type->GetDescription()));
		QFont typeFont = QGuiApplication::font();
		typeFont.setPointSize(typeFont.pointSize() * 3 / 4);
		type->setFont(typeFont);
		QPalette style(type->palette());
		style.setColor(QPalette::WindowText, Theme::disabled);
		type->setPalette(style);

		m_fieldLayout->addWidget(type, (int)i, 1);
		m_fields.push_back(type);

		MainWindow* mainWindow = m_mainWindow;
		shared_ptr<ActorType> actorType = m_actorType;
		shared_ptr<ActorFieldValue> fieldValue = make_shared<ActorFieldValue>(field.params,
			[=](const Json::Value&, const Json::Value& newValue) {
				ActorField oldField = actorType->GetField(i);
				ActorField newField = oldField;
				newField.params = newValue;
				actorType->SetField(i, newField);

				mainWindow->AddUndoAction(
					[=]() { // Undo
						actorType->SetField(i, oldField);
						mainWindow->UpdateActorTypeContents(actorType);
					},
					[=]() { // Redo
						actorType->SetField(i, newField);
						mainWindow->UpdateActorTypeContents(actorType);
					}
				);
			});

		QWidget* value = field.type->CreateParameterEditor(m_mainWindow, m_project, fieldValue);
		if (value)
		{
			m_fieldLayout->addWidget(value, (int)i, 2);
			m_fields.push_back(value);
		}
		else
		{
			QLabel* noDefault = new QLabel("Default value cannot be provided for this type");
			QFont noDefaultFont = QGuiApplication::font();
			noDefaultFont.setItalic(true);
			noDefault->setFont(noDefaultFont);
			m_fieldLayout->addWidget(noDefault, (int)i, 2);
			m_fields.push_back(noDefault);
		}

		QPushButton* renameButton = new QPushButton("Rename...");
		m_fieldLayout->addWidget(renameButton, (int)i, 3);
		m_fields.push_back(renameButton);
		connect(renameButton, &QPushButton::clicked, [=]() { RenameField(i); });

		QPushButton* removeButton = new QPushButton("Remove");
		m_fieldLayout->addWidget(removeButton, (int)i, 4);
		m_fields.push_back(removeButton);
		connect(removeButton, &QPushButton::clicked, [=]() { RemoveField(i); });
	}
}


ActorTypeView::ActorTypeView(MainWindow* parent, shared_ptr<Project> project,
	shared_ptr<ActorType> actorType): EditorView(parent)
{
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);
	QScrollArea* scrollArea = new QScrollArea(this);
	m_widget = new ActorTypeEditorWidget(this, parent, project, actorType);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setWidgetResizable(true);
	scrollArea->setAlignment(Qt::AlignTop);
	scrollArea->horizontalScrollBar()->setEnabled(false);
	scrollArea->setWidget(m_widget);
	layout->addWidget(scrollArea, 1);
	setLayout(layout);
}


void ActorTypeView::UpdateView()
{
	m_widget->UpdateView();
}


shared_ptr<ActorType> ActorTypeView::GetActorType() const
{
	return m_widget->GetActorType();
}
