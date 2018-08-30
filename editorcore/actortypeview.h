#pragma once

#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QCheckBox>
#include "project.h"
#include "actortype.h"
#include "editorview.h"
#include "spriteselectwidget.h"

class MainWindow;

class ActorTypeEditorWidget: public QWidget
{
	Q_OBJECT

	MainWindow* m_mainWindow;
	std::shared_ptr<Project> m_project;
	std::shared_ptr<ActorType> m_actorType;

	QLabel* m_name;
	QGridLayout* m_fieldLayout;
	QCheckBox* m_bounds;
	SpriteSelectWidget* m_sprite;

	std::vector<QWidget*> m_fields;

	void RenameField(size_t i);
	void RemoveField(size_t i);
	void OnSpriteChanged(std::shared_ptr<Sprite> oldSprite, std::shared_ptr<Sprite> newSprite);

public:
	ActorTypeEditorWidget(QWidget* parent, MainWindow* mainWindow, std::shared_ptr<Project> project,
		std::shared_ptr<ActorType> actorType);

	void UpdateView();

	std::shared_ptr<ActorType> GetActorType() const { return m_actorType; }

private slots:
	void OnAddField();
	void OnBoundsChanged(int state);
};

class ActorTypeView: public EditorView
{
	Q_OBJECT

	ActorTypeEditorWidget* m_widget;

public:
	ActorTypeView(MainWindow* parent, std::shared_ptr<Project> project,
		std::shared_ptr<ActorType> actorType);

	void UpdateView();

	std::shared_ptr<ActorType> GetActorType() const;
};
