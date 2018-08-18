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

	m_mapSize = new QLabel();
	headerLayout->addStretch(1);
	headerLayout->addWidget(m_mapSize);

	layout->addLayout(headerLayout);

	QScrollArea* layerAndTileScrollArea = new QScrollArea(this);
	QWidget* layerAndTiles = new QWidget(this);
	QVBoxLayout* layerAndTileLayout = new QVBoxLayout();
	layerAndTileLayout->setContentsMargins(8, 8, 8, 8);
	m_layers = new MapLayerWidget(layerAndTiles, m_editor, parent, project, map);
	m_editor->SetLayerWidget(m_layers);
	layerAndTileLayout->addWidget(m_layers);
	layerAndTileLayout->addSpacing(16);
	m_tiles = new MapTileWidget(layerAndTiles, m_editor, parent, project, map);
	m_editor->SetTileWidget(m_tiles);
	layerAndTileLayout->addWidget(m_tiles);
	layerAndTileLayout->addStretch(1);
	layerAndTiles->setLayout(layerAndTileLayout);
	layerAndTileScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	layerAndTileScrollArea->setWidgetResizable(true);
	layerAndTileScrollArea->setAlignment(Qt::AlignTop);
	layerAndTileScrollArea->setWidget(layerAndTiles);
	layerAndTileScrollArea->horizontalScrollBar()->setEnabled(false);
	contentLayout->addWidget(layerAndTileScrollArea);

	layout->addLayout(contentLayout, 1);
	setLayout(layout);

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
}


void MapView::UpdateView()
{
	m_mapSize->setText(QString::asprintf("%u x %u map", (unsigned)m_map->GetMainLayer()->GetWidth(),
		(unsigned)m_map->GetMainLayer()->GetHeight()));
	m_editor->UpdateView();
	m_layers->UpdateView();
	m_tiles->UpdateView();
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
