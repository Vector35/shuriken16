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
#include "effectlayerview.h"
#include "theme.h"
#include "mainwindow.h"

using namespace std;


EffectLayerView::EffectLayerView(MainWindow* parent, shared_ptr<Project> project, shared_ptr<MapLayer> layer):
	EditorView(parent), m_mainWindow(parent), m_project(project), m_layer(layer)
{
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);

	QHBoxLayout* headerLayout = new QHBoxLayout();
	headerLayout->setContentsMargins(8, 8, 8, 4);

	QHBoxLayout* contentLayout = new QHBoxLayout();

	shared_ptr<Map> map = make_shared<Map>();
	map->InsertLayer(0, layer);
	map->SetMainLayer(layer);
	m_editor = new MapEditorWidget(this, parent, project, map, true);
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
	m_circleMode = new ToolWidget("◯", "Circle",
		[=]() { return m_editor->GetTool() == CircleTool; },
		[=]() { m_editor->SetTool(CircleTool); UpdateToolState(); });
	headerLayout->addWidget(m_circleMode);
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
	layerAndTileLayout->setContentsMargins(8, 8, 0, 8);
	m_settings = new EffectLayerSettingsWidget(layerAndTiles, m_editor, parent, project, layer);
	layerAndTileLayout->addWidget(m_settings);
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

	m_deferredUpdateTimer = new QTimer(this);
	m_deferredUpdateTimer->setInterval(250);
	m_deferredUpdateTimer->setSingleShot(true);
	connect(m_deferredUpdateTimer, &QTimer::timeout, this, &EffectLayerView::OnDeferredUpdateTimer);
	m_lastUpdate = chrono::steady_clock::now();
	m_firstUpdate = true;

	UpdateView();
}


void EffectLayerView::UpdateView()
{
	m_editor->UpdateView();

	auto sinceLastUpdate = chrono::steady_clock::now() - m_lastUpdate;
	double t = chrono::duration_cast<chrono::milliseconds>(sinceLastUpdate).count();
	if (m_firstUpdate || (t > 250))
	{
		m_mapSize->setText(QString::asprintf("%u x %u layer", (unsigned)m_layer->GetWidth(),
			(unsigned)m_layer->GetHeight()));
		m_settings->UpdateView();
		m_tiles->UpdateView();
	}
	else
	{
		m_deferredUpdateTimer->stop();
		m_deferredUpdateTimer->start();
	}

	m_lastUpdate = chrono::steady_clock::now();
	m_firstUpdate = false;
}


void EffectLayerView::OnDeferredUpdateTimer()
{
	m_mapSize->setText(QString::asprintf("%u x %u layer", (unsigned)m_layer->GetWidth(),
		(unsigned)m_layer->GetHeight()));
	m_settings->UpdateView();
	m_tiles->UpdateView();
	m_lastUpdate = chrono::steady_clock::now();
}


void EffectLayerView::UpdateToolState()
{
	m_selectMode->UpdateState();
	m_penMode->UpdateState();
	m_rectMode->UpdateState();
	m_fillRectMode->UpdateState();
	m_lineMode->UpdateState();
	m_fillMode->UpdateState();
}


void EffectLayerView::Cut()
{
	m_editor->Cut();
}


void EffectLayerView::Copy()
{
	m_editor->Copy();
}


void EffectLayerView::Paste()
{
	m_editor->Paste();
	UpdateToolState();
}


void EffectLayerView::SelectAll()
{
	m_editor->SelectAll();
	UpdateToolState();
}
