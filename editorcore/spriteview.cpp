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
#include "spriteview.h"
#include "paletteview.h"
#include "theme.h"
#include "mainwindow.h"

using namespace std;
	

SpriteView::SpriteView(MainWindow* parent, shared_ptr<Project> project, shared_ptr<Sprite> sprite):
	EditorView(parent), m_mainWindow(parent), m_project(project), m_sprite(sprite)
{
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);

	QHBoxLayout* headerLayout = new QHBoxLayout();
	headerLayout->setContentsMargins(8, 8, 8, 4);

	QScrollArea* editorScrollArea = new QScrollArea(this);
	m_editor = new SpriteEditorWidget(this, parent, editorScrollArea, project, sprite);

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

	headerLayout->addStretch(1);
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
	m_preview = new SpritePreviewWidget(this, this, parent, project, sprite);
	m_editor->SetPreviewWidget(m_preview);
	previewLayout->addWidget(m_preview);
	previewLayout->addStretch(1);
	rightLayout->addLayout(previewLayout);
	m_previewAnim = new QCheckBox("Preview animation");
	connect(m_previewAnim, &QCheckBox::stateChanged, this, &SpriteView::SetPreviewAnimation);
	rightLayout->addWidget(m_previewAnim);
	m_activePalette = new SpritePaletteWidget(this, this, parent, project, sprite, true);
	rightLayout->addWidget(m_activePalette);

	QScrollArea* animAndPaletteScrollArea = new QScrollArea(this);
	QWidget* animAndPalette = new QWidget(this);
	QVBoxLayout* animAndPaletteLayout = new QVBoxLayout();
	animAndPaletteLayout->setContentsMargins(8, 8, 0, 8);
	m_anim = new SpriteAnimationWidget(animAndPalette, m_editor, parent, sprite);
	m_editor->SetAnimationWidget(m_anim);
	animAndPaletteLayout->addWidget(m_anim);
	m_palettes = new SpritePaletteWidget(animAndPalette, this, parent, project, sprite, false);
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
	connect(m_deferredUpdateTimer, &QTimer::timeout, this, &SpriteView::OnDeferredUpdateTimer);
	m_lastUpdate = chrono::steady_clock::now();
	m_firstUpdate = true;

	UpdateView();
}


void SpriteView::UpdateToolState()
{
	m_selectMode->UpdateState();
	m_penMode->UpdateState();
	m_rectMode->UpdateState();
	m_fillRectMode->UpdateState();
	m_lineMode->UpdateState();
	m_fillMode->UpdateState();
}


void SpriteView::UpdateView()
{
	m_editor->UpdateView();
	m_preview->UpdateView();

	auto sinceLastUpdate = chrono::steady_clock::now() - m_lastUpdate;
	double t = chrono::duration_cast<chrono::milliseconds>(sinceLastUpdate).count();
	if (m_firstUpdate || (t > 250))
	{
		m_anim->UpdateView();
		m_palettes->UpdateView();
		m_activePalette->UpdateView();
	}
	else
	{
		m_deferredUpdateTimer->stop();
		m_deferredUpdateTimer->start();
	}

	m_lastUpdate = chrono::steady_clock::now();
	m_firstUpdate = false;
}


void SpriteView::OnDeferredUpdateTimer()
{
	m_anim->UpdateView();
	m_palettes->UpdateView();
	m_activePalette->UpdateView();
	m_lastUpdate = chrono::steady_clock::now();
}


shared_ptr<Palette> SpriteView::GetSelectedPalette() const
{
	return m_editor->GetSelectedPalette();
}


size_t SpriteView::GetSelectedLeftPaletteEntry() const
{
	return m_editor->GetSelectedLeftPaletteEntry();
}


size_t SpriteView::GetSelectedRightPaletteEntry() const
{
	return m_editor->GetSelectedRightPaletteEntry();
}


void SpriteView::SetSelectedLeftPaletteEntry(shared_ptr<Palette> palette, size_t entry)
{
	m_editor->SetSelectedLeftPaletteEntry(palette, entry);
	m_palettes->UpdateView();
	m_activePalette->UpdateView();
}


void SpriteView::SetSelectedRightPaletteEntry(shared_ptr<Palette> palette, size_t entry)
{
	m_editor->SetSelectedRightPaletteEntry(palette, entry);
	m_palettes->UpdateView();
	m_activePalette->UpdateView();
}


void SpriteView::Cut()
{
	m_editor->Cut();
}


void SpriteView::Copy()
{
	m_editor->Copy();
}


void SpriteView::Paste()
{
	m_editor->Paste();
	UpdateToolState();
}


void SpriteView::SelectAll()
{
	m_editor->SelectAll();
	UpdateToolState();
}


void SpriteView::SetPreviewAnimation(int state)
{
	m_preview->SetPreviewAnimation(state == Qt::Checked);
}
