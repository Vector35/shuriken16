#include <QLabel>
#include <QGuiApplication>
#include <QInputDialog>
#include "tilesetanimationwidget.h"
#include "animationframewidget.h"
#include "tileseteditorwidget.h"
#include "mainwindow.h"
#include "tilesetview.h"

using namespace std;


TileSetAnimationWidget::TileSetAnimationWidget(QWidget* parent, TileSetEditorWidget* editor,
	MainWindow* mainWindow, shared_ptr<TileSet> tileSet): QWidget(parent),
	m_mainWindow(mainWindow), m_editor(editor), m_tileSet(tileSet)
{
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 8);

	QHBoxLayout* headerLayout = new QHBoxLayout();
	QLabel* layers = new QLabel("Animation");
	QFont headerFont = QGuiApplication::font();
	headerFont.setPointSize(headerFont.pointSize() * 5 / 4);
	layers->setFont(headerFont);
	headerLayout->addWidget(layers, 1);
	m_createButton = new QPushButton("Create");
	connect(m_createButton, &QPushButton::clicked, this, &TileSetAnimationWidget::OnCreateAnimation);
	headerLayout->addWidget(m_createButton);
	headerLayout->addSpacing(8);
	layout->addLayout(headerLayout);

	m_entryLayout = new QVBoxLayout();
	layout->addLayout(m_entryLayout);

	setLayout(layout);
}


void TileSetAnimationWidget::EditFrame(uint16_t frame)
{
	shared_ptr<Animation> anim = m_tileSet->GetAnimation();
	if (!anim)
		return;

	bool ok;
	uint16_t oldLength = anim->GetFrameLength(frame);
	uint16_t newLength = (uint16_t)QInputDialog::getInt(this, "Animation Frame Length",
		"Length of animation frame (in 1/60 sec):", oldLength, 1, 65535, 1, &ok);
	if (!ok)
		return;

	anim->SetFrameLength(frame, newLength);
	m_mainWindow->UpdateTileSetContents(m_tileSet);

	shared_ptr<TileSet> tileSet = m_tileSet;
	MainWindow* mainWindow = m_mainWindow;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			anim->SetFrameLength(frame, oldLength);
			mainWindow->UpdateTileSetContents(tileSet);
		},
		[=]() { // Redo
			anim->SetFrameLength(frame, newLength);
			mainWindow->UpdateTileSetContents(tileSet);
		}
	);
}


void TileSetAnimationWidget::DuplicateFrame(uint16_t frame)
{
	m_tileSet->DuplicateFrame(frame);
	m_mainWindow->UpdateTileSetContents(m_tileSet);

	shared_ptr<TileSet> tileSet = m_tileSet;
	MainWindow* mainWindow = m_mainWindow;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			tileSet->RemoveFrame(frame);
			mainWindow->UpdateTileSetContents(tileSet);
		},
		[=]() { // Redo
			tileSet->DuplicateFrame(frame);
			mainWindow->UpdateTileSetContents(tileSet);
		}
	);
}


void TileSetAnimationWidget::MoveFrameUp(uint16_t frame)
{
	m_tileSet->SwapFrames(frame - 1, frame);
	m_mainWindow->UpdateTileSetContents(m_tileSet);
	m_editor->SetFrame(frame - 1);

	shared_ptr<TileSet> tileSet = m_tileSet;
	MainWindow* mainWindow = m_mainWindow;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			m_tileSet->SwapFrames(frame - 1, frame);
			mainWindow->UpdateTileSetContents(tileSet);
			TileSetView* view = mainWindow->GetTileSetView(tileSet);
			if (view)
				view->GetEditor()->SetFrame(frame);
		},
		[=]() { // Redo
			m_tileSet->SwapFrames(frame - 1, frame);
			mainWindow->UpdateTileSetContents(tileSet);
			TileSetView* view = mainWindow->GetTileSetView(tileSet);
			if (view)
				view->GetEditor()->SetFrame(frame - 1);
		}
	);
}


void TileSetAnimationWidget::MoveFrameDown(uint16_t frame)
{
	m_tileSet->SwapFrames(frame, frame + 1);
	m_mainWindow->UpdateTileSetContents(m_tileSet);
	m_editor->SetFrame(frame + 1);

	shared_ptr<TileSet> tileSet = m_tileSet;
	MainWindow* mainWindow = m_mainWindow;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			m_tileSet->SwapFrames(frame, frame + 1);
			mainWindow->UpdateTileSetContents(tileSet);
			TileSetView* view = mainWindow->GetTileSetView(tileSet);
			if (view)
				view->GetEditor()->SetFrame(frame);
		},
		[=]() { // Redo
			m_tileSet->SwapFrames(frame, frame + 1);
			mainWindow->UpdateTileSetContents(tileSet);
			TileSetView* view = mainWindow->GetTileSetView(tileSet);
			if (view)
				view->GetEditor()->SetFrame(frame + 1);
		}
	);
}


void TileSetAnimationWidget::RemoveFrame(uint16_t frame)
{
	shared_ptr<Animation> anim = m_tileSet->GetAnimation();
	if (!anim)
		return;
	if (frame >= anim->GetFrameCount())
		return;

	shared_ptr<RemovedFrame> frameData = make_shared<RemovedFrame>();
	frameData->length = anim->GetFrameLength(frame);
	frameData->data = m_tileSet->GetTilesForSingleFrame(frame);
	m_tileSet->RemoveFrame(frame);

	if (anim->GetFrameCount() <= 1)
	{
		m_tileSet->SetAnimation(shared_ptr<Animation>());
		m_mainWindow->UpdateTileSetContents(m_tileSet);
		m_editor->SetFrame(0);

		shared_ptr<TileSet> tileSet = m_tileSet;
		MainWindow* mainWindow = m_mainWindow;
		m_mainWindow->AddUndoAction(
			[=]() { // Undo
				m_tileSet->SetAnimation(anim);
				m_tileSet->InsertFrameFromTiles(frame, frameData->data, frameData->length);
				mainWindow->UpdateTileSetContents(tileSet);
				TileSetView* view = mainWindow->GetTileSetView(tileSet);
				if (view)
					view->GetEditor()->SetFrame(frame);
			},
			[=]() { // Redo
				m_tileSet->RemoveFrame(frame);
				m_tileSet->SetAnimation(shared_ptr<Animation>());
				mainWindow->UpdateTileSetContents(tileSet);
				TileSetView* view = mainWindow->GetTileSetView(tileSet);
				if (view)
					view->GetEditor()->SetFrame(0);
			}
		);
	}
	else
	{
		m_mainWindow->UpdateTileSetContents(m_tileSet);
		if (frame >= anim->GetFrameCount())
			m_editor->SetFrame(anim->GetFrameCount() - 1);

		shared_ptr<TileSet> tileSet = m_tileSet;
		MainWindow* mainWindow = m_mainWindow;
		m_mainWindow->AddUndoAction(
			[=]() { // Undo
				m_tileSet->InsertFrameFromTiles(frame, frameData->data, frameData->length);
				mainWindow->UpdateTileSetContents(tileSet);
				TileSetView* view = mainWindow->GetTileSetView(tileSet);
				if (view)
					view->GetEditor()->SetFrame(frame);
			},
			[=]() { // Redo
				m_tileSet->RemoveFrame(frame);
				mainWindow->UpdateTileSetContents(tileSet);
				TileSetView* view = mainWindow->GetTileSetView(tileSet);
				if (view && (frame >= anim->GetFrameCount()))
					view->GetEditor()->SetFrame(anim->GetFrameCount() - 1);
			}
		);
	}
}


void TileSetAnimationWidget::UpdateView()
{
	for (auto i : m_entries)
	{
		m_entryLayout->removeWidget(i);
		i->deleteLater();
	}
	m_entries.clear();

	if (!m_tileSet->GetAnimation())
	{
		QLabel* label = new QLabel("No animation created");
		QFont labelFont = QGuiApplication::font();
		labelFont.setItalic(true);
		label->setFont(labelFont);
		m_entryLayout->addWidget(label);
		m_entries.push_back(label);

		m_createButton->show();
		return;
	}

	shared_ptr<Animation> anim = m_tileSet->GetAnimation();
	uint16_t frame = 0;
	for (auto i : anim->GetFrameLengths())
	{
		char name[256];
		sprintf(name, "Frame %d, length %d/60 sec", frame, i);
		AnimationFrameWidget* widget = new AnimationFrameWidget(this, name,
			m_editor->GetFrame() == frame, frame > 0, frame < (anim->GetFrameLengths().size() - 1),
			[=]() { m_editor->SetFrame(frame); },
			[=]() { EditFrame(frame); },
			[=]() { DuplicateFrame(frame); },
			[=]() { RemoveFrame(frame); },
			[=]() { MoveFrameUp(frame); },
			[=]() { MoveFrameDown(frame); });
		m_entryLayout->addWidget(widget);
		m_entries.push_back(widget);
		frame++;
	}

	m_createButton->hide();
}


void TileSetAnimationWidget::OnCreateAnimation()
{
	shared_ptr<Animation> anim = make_shared<Animation>();
	anim->AddFrame(8);
	anim->AddFrame(8);
	m_tileSet->SetAnimation(anim);
	m_tileSet->CopyFrame(0, 1);
	m_mainWindow->UpdateTileSetContents(m_tileSet);

	shared_ptr<TileSet> tileSet = m_tileSet;
	MainWindow* mainWindow = m_mainWindow;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			tileSet->SetAnimation(shared_ptr<Animation>());
			mainWindow->UpdateTileSetContents(tileSet);
		},
		[=]() { // Redo
			tileSet->SetAnimation(anim);
			tileSet->CopyFrame(0, 1);
			mainWindow->UpdateTileSetContents(tileSet);
		}
	);
}
