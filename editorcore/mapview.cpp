#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QGuiApplication>
#include <QPicture>
#include <QPainter>
#include <QColorDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QScrollBar>
#include "mapview.h"
#include "theme.h"
#include "mainwindow.h"
#include "mapactorwidget.h"

using namespace std;


MapView::MapView(MainWindow* parent, shared_ptr<Project> project, shared_ptr<Map> map):
	EditorView(parent), m_mainWindow(parent), m_project(project), m_map(map)
{
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);

	QHBoxLayout* headerLayout = new QHBoxLayout();
	headerLayout->setContentsMargins(8, 8, 8, 4);

	QHBoxLayout* contentLayout = new QHBoxLayout();

	m_editor = new MapEditorWidget(this, parent, project, map, false);
	contentLayout->addWidget(m_editor, 1);

	m_selectMode = new ToolWidget("⬚", "Select",
		[=]() { return m_editor->GetTool() == SelectTool; },
		[=]() { m_editor->SetTool(SelectTool); UpdateToolState(); });
	headerLayout->addWidget(m_selectMode);
	m_penMode = new ToolWidget("✎", "Pen",
		[=]() { return m_editor->GetTool() == PenTool; },
		[=]() { m_editor->SetTool(PenTool); UpdateToolState(); });
	headerLayout->addWidget(m_penMode);
	m_rectMode = new ToolWidget("□", "Rectangle",
		[=]() { return m_editor->GetTool() == RectangleTool; },
		[=]() { m_editor->SetTool(RectangleTool); UpdateToolState(); });
	headerLayout->addWidget(m_rectMode);
	m_fillRectMode = new ToolWidget("■", "Filled Rectangle",
		[=]() { return m_editor->GetTool() == FilledRectangleTool; },
		[=]() { m_editor->SetTool(FilledRectangleTool); UpdateToolState(); });
	m_circleMode = new ToolWidget("◯", "Circle",
		[=]() { return m_editor->GetTool() == CircleTool; },
		[=]() { m_editor->SetTool(CircleTool); UpdateToolState(); });
	headerLayout->addWidget(m_circleMode);
	headerLayout->addWidget(m_fillRectMode);
	m_lineMode = new ToolWidget("╱", "Line",
		[=]() { return m_editor->GetTool() == LineTool; },
		[=]() { m_editor->SetTool(LineTool); UpdateToolState(); });
	headerLayout->addWidget(m_lineMode);
	m_fillMode = new ToolWidget("▨", "Fill",
		[=]() { return m_editor->GetTool() == FillTool; },
		[=]() { m_editor->SetTool(FillTool); UpdateToolState(); });
	headerLayout->addWidget(m_fillMode);
	m_zoomInMode = new ToolWidget("⊕", "Zoom In",
		[=]() { return false; },
		[=]() { m_editor->ZoomIn(); });
	headerLayout->addWidget(m_zoomInMode);
	m_zoomOutMode = new ToolWidget("⊖", "Zoom Out",
		[=]() { return false; },
		[=]() { m_editor->ZoomOut(); });
	headerLayout->addWidget(m_zoomOutMode);

	headerLayout->addSpacing(16);

	m_actorMode = new ToolWidget("A", "Actor Editor",
		[=]() { return m_editor->GetTool() == ActorTool; },
		[=]() { m_editor->SetTool(ActorTool); UpdateToolState(); });
	headerLayout->addWidget(m_actorMode);

	m_mapSize = new QLabel();
	headerLayout->addStretch(1);
	headerLayout->addWidget(m_mapSize);

	layout->addLayout(headerLayout);

	QVBoxLayout* rightLayout = new QVBoxLayout();
	m_layers = new MapLayerWidget(this, m_editor, parent, project, map);
	m_editor->SetLayerWidget(m_layers);
	rightLayout->addWidget(m_layers);
	rightLayout->addSpacing(16);

	QScrollArea* tileScrollArea = new QScrollArea(this);
	QWidget* tiles = new QWidget(this);
	QVBoxLayout* tileLayout = new QVBoxLayout();
	tileLayout->setContentsMargins(8, 8, 8, 8);
	m_tiles = new MapTileWidget(tiles, m_editor, parent, project, map);
	m_editor->SetTileWidget(m_tiles);
	tileLayout->addWidget(m_tiles);
	m_actors = new MapActorWidget(tiles, m_editor, parent, project, map);
	m_actors->setVisible(false);
	m_editor->SetActorWidget(m_actors);
	tileLayout->addWidget(m_actors);
	tileLayout->addStretch(1);
	tiles->setLayout(tileLayout);
	tileScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	tileScrollArea->setWidgetResizable(true);
	tileScrollArea->setAlignment(Qt::AlignTop);
	tileScrollArea->setWidget(tiles);
	tileScrollArea->horizontalScrollBar()->setEnabled(false);
	rightLayout->addWidget(tileScrollArea);
	contentLayout->addLayout(rightLayout);

	layout->addLayout(contentLayout, 1);
	setLayout(layout);

	m_deferredUpdateTimer = new QTimer(this);
	m_deferredUpdateTimer->setInterval(250);
	m_deferredUpdateTimer->setSingleShot(true);
	connect(m_deferredUpdateTimer, &QTimer::timeout, this, &MapView::OnDeferredUpdateTimer);
	m_lastUpdate = chrono::steady_clock::now();
	m_firstUpdate = true;

	UpdateView();
}


void MapView::UpdateToolState()
{
	m_selectMode->UpdateState();
	m_penMode->UpdateState();
	m_rectMode->UpdateState();
	m_fillRectMode->UpdateState();
	m_lineMode->UpdateState();
	m_fillMode->UpdateState();
	m_actorMode->UpdateState();
}


void MapView::UpdateView()
{
	m_editor->UpdateView();

	auto sinceLastUpdate = chrono::steady_clock::now() - m_lastUpdate;
	double t = chrono::duration_cast<chrono::milliseconds>(sinceLastUpdate).count();
	if (m_firstUpdate || (t > 250))
	{
		m_mapSize->setText(QString::asprintf("%u x %u map", (unsigned)m_map->GetMainLayer()->GetWidth(),
			(unsigned)m_map->GetMainLayer()->GetHeight()));
		m_layers->UpdateView();
		m_tiles->UpdateView();
		m_actors->UpdateView();
	}
	else
	{
		m_deferredUpdateTimer->stop();
		m_deferredUpdateTimer->start();
	}

	m_lastUpdate = chrono::steady_clock::now();
	m_firstUpdate = false;
}


void MapView::OnDeferredUpdateTimer()
{
	m_mapSize->setText(QString::asprintf("%u x %u map", (unsigned)m_map->GetMainLayer()->GetWidth(),
		(unsigned)m_map->GetMainLayer()->GetHeight()));
	m_layers->UpdateView();
	m_tiles->UpdateView();
	m_actors->UpdateView();
	m_lastUpdate = chrono::steady_clock::now();
}


void MapView::Cut()
{
	m_editor->Cut();
}


void MapView::Copy()
{
	m_editor->Copy();
}


void MapView::Paste()
{
	m_editor->Paste();
	UpdateToolState();
}


void MapView::SelectAll()
{
	m_editor->SelectAll();
	UpdateToolState();
}
