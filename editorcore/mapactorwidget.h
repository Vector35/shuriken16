#pragma once

#include <QWidget>
#include <QGridLayout>
#include "project.h"
#include "map.h"
#include "actor.h"

class MainWindow;
class MapEditorWidget;

class MapActorWidget: public QWidget
{
	Q_OBJECT

	MainWindow* m_mainWindow;
	MapEditorWidget* m_editor;
	std::shared_ptr<Project> m_project;
	std::shared_ptr<Map> m_map;

	QGridLayout* m_entryLayout;
	std::vector<QWidget*> m_entries;

	bool m_positionValid;
	size_t m_x, m_y, m_width, m_height;

	std::shared_ptr<Actor> m_selectedActor;
	std::shared_ptr<ActorType> m_lastActorType;

	void RemoveActor(std::shared_ptr<Actor> actor);

public:
	MapActorWidget(QWidget* parent, MapEditorWidget* editor, MainWindow* mainWindow,
		std::shared_ptr<Project> project, std::shared_ptr<Map> map);

	std::shared_ptr<Actor> GetSelectedActor() const { return m_selectedActor; }

	void UpdateView();

	void ClearSelection();
	void SetSelection(size_t x, size_t y, size_t width, size_t height);

private slots:
	void OnCreateActor();
};
