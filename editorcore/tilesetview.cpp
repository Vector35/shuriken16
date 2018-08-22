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
#include "tilesetview.h"
#include "paletteview.h"
#include "theme.h"
#include "mainwindow.h"

using namespace std;
	

TileSetView::TileSetView(MainWindow* parent, shared_ptr<Project> project, shared_ptr<TileSet> tileSet):
	EditorView(parent), m_mainWindow(parent), m_project(project), m_tileSet(tileSet)
{
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);

	QHBoxLayout* headerLayout = new QHBoxLayout();
	headerLayout->setContentsMargins(8, 8, 8, 4);

	QScrollArea* editorScrollArea = new QScrollArea(this);
	m_editor = new TileSetEditorWidget(this, parent, editorScrollArea, project, tileSet);

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
	m_flipHorizontalMode = new ToolWidget("↔", "Flip Horizontal",
		[=]() { return false; },
		[=]() { m_editor->FlipHorizontal(); });
	headerLayout->addWidget(m_flipHorizontalMode);
	m_flipVerticalMode = new ToolWidget("↕", "Flip Vertical",
		[=]() { return false; },
		[=]() { m_editor->FlipVertical(); });
	headerLayout->addWidget(m_flipVerticalMode);
	m_rotateMode = new ToolWidget("↻", "Rotate",
		[=]() { return false; },
		[=]() { m_editor->Rotate(); });
	headerLayout->addWidget(m_rotateMode);
	m_zoomInMode = new ToolWidget("⊕", "Zoom In",
		[=]() { return false; },
		[=]() { m_editor->ZoomIn(); });
	headerLayout->addWidget(m_zoomInMode);
	m_zoomOutMode = new ToolWidget("⊖", "Zoom Out",
		[=]() { return false; },
		[=]() { m_editor->ZoomOut(); });
	headerLayout->addWidget(m_zoomOutMode);

	headerLayout->addSpacing(16);

	m_collisionMode = new ToolWidget("⎄", "Collision Editor",
		[=]() { return m_editor->GetTool() == CollisionTool; },
		[=]() { m_editor->SetTool(CollisionTool); UpdateToolState(); });
	headerLayout->addWidget(m_collisionMode);
	m_removeCollisions = new ToolWidget("✖", "Remove All Collisions",
		[=]() { return false; },
		[=]() { m_editor->RemoveCollisions(); UpdateToolState(); });
	headerLayout->addWidget(m_removeCollisions);
	m_collideWithAll = new ToolWidget("✚", "Collide with All",
		[=]() { return false; },
		[=]() { m_editor->CollideWithAll(); UpdateToolState(); });
	headerLayout->addWidget(m_collideWithAll);

	m_colCount = new QLabel();
	headerLayout->addStretch(1);
	headerLayout->addWidget(m_colCount);

	if (!m_tileSet->IsSmartTileSet())
	{
		QPushButton* changeColsButton = new QPushButton("Set Width...");
		connect(changeColsButton, &QPushButton::clicked, this, &TileSetView::ChangeColumnCount);
		headerLayout->addWidget(changeColsButton);
	}

	m_tileCount = new QLabel();
	headerLayout->addSpacing(16);
	headerLayout->addWidget(m_tileCount);

	if (!m_tileSet->IsSmartTileSet())
	{
		QPushButton* resizeButton = new QPushButton("Resize...");
		connect(resizeButton, &QPushButton::clicked, this, &TileSetView::ResizeTileSet);
		headerLayout->addWidget(resizeButton);
	}

	layout->addLayout(headerLayout);

	QHBoxLayout* contentLayout = new QHBoxLayout();

	QPalette style(this->palette());
	style.setColor(QPalette::Window, Theme::background);
	editorScrollArea->setPalette(style);
	editorScrollArea->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	editorScrollArea->setWidget(m_editor);
	contentLayout->addWidget(editorScrollArea, 1);

	QVBoxLayout* rightLayout = new QVBoxLayout();
	QHBoxLayout* previewLayout = new QHBoxLayout();
	m_preview = new TileSetPreviewWidget(this, this, parent, project, tileSet);
	m_editor->SetPreviewWidget(m_preview);
	previewLayout->addWidget(m_preview);
	previewLayout->addStretch(1);
	rightLayout->addLayout(previewLayout);
	m_previewAnim = new QCheckBox("Preview animation");
	connect(m_previewAnim, &QCheckBox::stateChanged, this, &TileSetView::SetPreviewAnimation);
	rightLayout->addWidget(m_previewAnim);
	m_activePalette = new TileSetPaletteWidget(this, this, parent, project, tileSet, true);
	rightLayout->addWidget(m_activePalette);

	QScrollArea* animAndPaletteScrollArea = new QScrollArea(this);
	QWidget* animAndPalette = new QWidget(this);
	QVBoxLayout* animAndPaletteLayout = new QVBoxLayout();
	animAndPaletteLayout->setContentsMargins(8, 8, 0, 8);
	m_anim = new TileSetAnimationWidget(animAndPalette, m_editor, parent, tileSet);
	m_editor->SetAnimationWidget(m_anim);
	animAndPaletteLayout->addWidget(m_anim);
	m_palettes = new TileSetPaletteWidget(animAndPalette, this, parent, project, tileSet, false);
	animAndPaletteLayout->addWidget(m_palettes);
	animAndPaletteLayout->addStretch(1);
	animAndPalette->setLayout(animAndPaletteLayout);
	animAndPaletteScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	animAndPaletteScrollArea->setWidgetResizable(true);
	animAndPaletteScrollArea->setAlignment(Qt::AlignTop);
	animAndPaletteScrollArea->setWidget(animAndPalette);
	animAndPaletteScrollArea->horizontalScrollBar()->setEnabled(false);
	rightLayout->addWidget(animAndPaletteScrollArea);
	contentLayout->addLayout(rightLayout);

	layout->addLayout(contentLayout, 1);
	setLayout(layout);

	m_deferredUpdateTimer = new QTimer(this);
	m_deferredUpdateTimer->setInterval(250);
	m_deferredUpdateTimer->setSingleShot(true);
	connect(m_deferredUpdateTimer, &QTimer::timeout, this, &TileSetView::OnDeferredUpdateTimer);
	m_lastUpdate = chrono::steady_clock::now();
	m_firstUpdate = true;

	UpdateView();
}


void TileSetView::UpdateToolState()
{
	m_selectMode->UpdateState();
	m_penMode->UpdateState();
	m_rectMode->UpdateState();
	m_fillRectMode->UpdateState();
	m_lineMode->UpdateState();
	m_fillMode->UpdateState();
	m_collisionMode->UpdateState();
}


void TileSetView::UpdateView()
{
	m_editor->UpdateView();
	m_preview->UpdateView();

	auto sinceLastUpdate = chrono::steady_clock::now() - m_lastUpdate;
	double t = chrono::duration_cast<chrono::milliseconds>(sinceLastUpdate).count();
	if (m_firstUpdate || (t > 250))
	{
		m_colCount->setText(QString::asprintf("%u", (unsigned int)m_tileSet->GetDisplayColumns()) +
			QString((m_tileSet->GetDisplayColumns() == 1) ? " tile per row" : " tiles per row"));
		m_tileCount->setText(QString::asprintf("%u", (unsigned int)m_tileSet->GetTileCount()) +
			QString((m_tileSet->GetTileCount() == 1) ? " tile in set" : " tiles in set"));

		m_anim->UpdateView();
		m_palettes->UpdateView();
		m_activePalette->UpdateView();

		if (m_tileSet->GetAnimation())
			m_previewAnim->show();
		else
			m_previewAnim->hide();
	}
	else
	{
		m_deferredUpdateTimer->stop();
		m_deferredUpdateTimer->start();
	}

	m_lastUpdate = chrono::steady_clock::now();
	m_firstUpdate = false;
}


void TileSetView::OnDeferredUpdateTimer()
{
	m_colCount->setText(QString::asprintf("%u", (unsigned int)m_tileSet->GetDisplayColumns()) +
		QString((m_tileSet->GetDisplayColumns() == 1) ? " tile per row" : " tiles per row"));
	m_tileCount->setText(QString::asprintf("%u", (unsigned int)m_tileSet->GetTileCount()) +
		QString((m_tileSet->GetTileCount() == 1) ? " tile in set" : " tiles in set"));

	m_anim->UpdateView();
	m_palettes->UpdateView();
	m_activePalette->UpdateView();

	if (m_tileSet->GetAnimation())
		m_previewAnim->show();
	else
		m_previewAnim->hide();

	m_lastUpdate = chrono::steady_clock::now();
}


void TileSetView::ChangeColumnCount()
{
	bool ok;
	size_t existingCols = m_tileSet->GetDisplayColumns();
	size_t cols = (size_t)QInputDialog::getInt(this, "Set Display Width", "Number of tiles to display per row:",
		(int)existingCols, 1, 64, 1, &ok);
	if (!ok)
		return;

	if ((cols < 1) || (cols > 64))
	{
		QMessageBox::critical(this, "Error", "Width must be between 1 and 64.");
		return;
	}

	m_tileSet->SetDisplayColumns(cols);
	m_mainWindow->UpdateTileSetContents(m_tileSet);

	shared_ptr<TileSet> tileSet = m_tileSet;
	MainWindow* mainWindow = m_mainWindow;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			tileSet->SetDisplayColumns(existingCols);
			mainWindow->UpdateTileSetContents(tileSet);
		},
		[=]() { // Redo
			tileSet->SetDisplayColumns(cols);
			mainWindow->UpdateTileSetContents(tileSet);
		}
	);
}


void TileSetView::ResizeTileSet()
{
	bool ok;
	size_t existingCount = m_tileSet->GetTileCount();
	size_t count = (size_t)QInputDialog::getInt(this, "Resize Tile Set", "Number of tiles:",
		(int)existingCount, 1, 1024, 1, &ok);
	if (!ok)
		return;

	if ((count < 1) || (count > 1024))
	{
		QMessageBox::critical(this, "Error", "Tile set size must be between 1 and 1024.");
		return;
	}

	if (count < existingCount)
	{
		std::vector<std::shared_ptr<Tile>> deletedEntries;
		deletedEntries.insert(deletedEntries.end(), m_tileSet->GetTiles().begin() + count,
			m_tileSet->GetTiles().end());
		if (deletedEntries.size() != (existingCount - count))
		{
			QMessageBox::critical(this, "Error", "Internal error saving undo action for deletion of "
				"tile set entries.");
			return;
		}
		m_tileSet->SetTileCount(count);
		m_mainWindow->UpdateTileSetContents(m_tileSet);

		shared_ptr<TileSet> tileSet = m_tileSet;
		MainWindow* mainWindow = m_mainWindow;
		m_mainWindow->AddUndoAction(
			[=]() { // Undo
				tileSet->SetTileCount(existingCount);
				for (size_t i = 0; i < deletedEntries.size(); i++)
					tileSet->SetTile(count + i, deletedEntries[i]);
				mainWindow->UpdateTileSetContents(tileSet);
			},
			[=]() { // Redo
				tileSet->SetTileCount(count);
				mainWindow->UpdateTileSetContents(tileSet);
			}
		);
	}
	else
	{
		m_tileSet->SetTileCount(count);
		m_mainWindow->UpdateTileSetContents(m_tileSet);

		shared_ptr<TileSet> tileSet = m_tileSet;
		MainWindow* mainWindow = m_mainWindow;
		m_mainWindow->AddUndoAction(
			[=]() { // Undo
				tileSet->SetTileCount(existingCount);
				mainWindow->UpdateTileSetContents(tileSet);
			},
			[=]() { // Redo
				tileSet->SetTileCount(count);
				mainWindow->UpdateTileSetContents(tileSet);
			}
		);
	}
}


shared_ptr<Palette> TileSetView::GetSelectedPalette() const
{
	return m_editor->GetSelectedPalette();
}


size_t TileSetView::GetSelectedLeftPaletteEntry() const
{
	return m_editor->GetSelectedLeftPaletteEntry();
}


size_t TileSetView::GetSelectedRightPaletteEntry() const
{
	return m_editor->GetSelectedRightPaletteEntry();
}


void TileSetView::SetSelectedLeftPaletteEntry(shared_ptr<Palette> palette, size_t entry)
{
	m_editor->SetSelectedLeftPaletteEntry(palette, entry);
	m_palettes->UpdateView();
	m_activePalette->UpdateView();
}


void TileSetView::SetSelectedRightPaletteEntry(shared_ptr<Palette> palette, size_t entry)
{
	m_editor->SetSelectedRightPaletteEntry(palette, entry);
	m_palettes->UpdateView();
	m_activePalette->UpdateView();
}


void TileSetView::Cut()
{
	m_editor->Cut();
}


void TileSetView::Copy()
{
	m_editor->Copy();
}


void TileSetView::Paste()
{
	m_editor->Paste();
	UpdateToolState();
}


void TileSetView::SelectAll()
{
	m_editor->SelectAll();
	UpdateToolState();
}


void TileSetView::SetPreviewAnimation(int state)
{
	m_preview->SetPreviewAnimation(state == Qt::Checked);
}
