#include <QLabel>
#include <QGuiApplication>
#include <QInputDialog>
#include <QMessageBox>
#include "spriteanimationwidget.h"
#include "animationframewidget.h"
#include "spriteeditorwidget.h"
#include "mainwindow.h"
#include "spriteview.h"
#include "spriteanimationheaderwidget.h"
#include "animationsettingsdialog.h"

using namespace std;


SpriteAnimationWidget::SpriteAnimationWidget(QWidget* parent, SpriteEditorWidget* editor,
	MainWindow* mainWindow, shared_ptr<Sprite> sprite): QWidget(parent),
	m_mainWindow(mainWindow), m_editor(editor), m_sprite(sprite)
{
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 8);

	QHBoxLayout* headerLayout = new QHBoxLayout();
	QLabel* layers = new QLabel("Animations");
	QFont headerFont = QGuiApplication::font();
	headerFont.setPointSize(headerFont.pointSize() * 5 / 4);
	layers->setFont(headerFont);
	headerLayout->addWidget(layers, 1);
	m_createButton = new QPushButton("New...");
	connect(m_createButton, &QPushButton::clicked, this, &SpriteAnimationWidget::OnCreateAnimation);
	headerLayout->addWidget(m_createButton);
	headerLayout->addSpacing(16);
	layout->addLayout(headerLayout);

	m_entryLayout = new QVBoxLayout();
	layout->addLayout(m_entryLayout);

	setLayout(layout);
}


void SpriteAnimationWidget::EditAnimation(const shared_ptr<SpriteAnimation>& anim)
{
	string oldName = anim->GetName();
	bool oldLoop = anim->IsLooping();

	AnimationSettingsDialog dialog(this, anim);
	if (dialog.exec() != QDialog::Accepted)
		return;

	string newName = dialog.GetName();
	bool newLoop = dialog.IsLooping();

	if (newName.size() == 0)
	{
		QMessageBox::critical(this, "Error", "New animation name is invalid.");
		return;
	}

	anim->SetName(newName);
	anim->SetLooping(newLoop);
	m_mainWindow->UpdateSpriteContents(m_sprite);

	shared_ptr<Sprite> sprite = m_sprite;
	MainWindow* mainWindow = m_mainWindow;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			anim->SetName(oldName);
			anim->SetLooping(oldLoop);
			mainWindow->UpdateSpriteContents(sprite);
		},
		[=]() { // Redo
			anim->SetName(newName);
			anim->SetLooping(newLoop);
			mainWindow->UpdateSpriteContents(sprite);
		}
	);
}


void SpriteAnimationWidget::DuplicateAnimation(const shared_ptr<SpriteAnimation>& anim)
{
	string oldName = anim->GetName();
	bool ok;
	string newName = QInputDialog::getText(this, "Dupliciate", "New name:", QLineEdit::Normal,
		QString::fromStdString(oldName), &ok).toStdString();
	if (!ok)
		return;

	if (newName.size() == 0)
	{
		QMessageBox::critical(this, "Error", "New animation name is invalid.");
		return;
	}

	shared_ptr<SpriteAnimation> animCopy = make_shared<SpriteAnimation>(*anim);
	animCopy->SetName(newName);
	size_t i = m_sprite->GetAnimationCount();
	m_sprite->AddAnimation(animCopy);
	m_mainWindow->UpdateSpriteContents(m_sprite);
	m_editor->SetFrame(animCopy, 0);

	shared_ptr<Sprite> sprite = m_sprite;
	MainWindow* mainWindow = m_mainWindow;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			m_sprite->RemoveAnimation(i);
			mainWindow->UpdateSpriteContents(sprite);
			SpriteView* view = mainWindow->GetSpriteView(sprite);
			if (view)
				view->GetEditor()->SetFrame(anim, 0);
		},
		[=]() { // Redo
			m_sprite->AddAnimation(animCopy);
			mainWindow->UpdateSpriteContents(sprite);
			SpriteView* view = mainWindow->GetSpriteView(sprite);
			if (view)
				view->GetEditor()->SetFrame(animCopy, 0);
		}
	);
}


void SpriteAnimationWidget::RemoveAnimation(const shared_ptr<SpriteAnimation>& anim)
{
	if (m_sprite->GetAnimationCount() <= 1)
	{
		QMessageBox::critical(this, "Error", "Cannot remove the last animation.");
		return;
	}

	if (QMessageBox::question(this, "Delete Animation", QString("Are you sure you want to remove the animation '") +
		QString::fromStdString(anim->GetName()) + QString("'?"), QMessageBox::Yes,
		QMessageBox::No | QMessageBox::Default | QMessageBox::Escape, QMessageBox::NoButton) !=
		QMessageBox::Yes)
		return;

	size_t i;
	for (i = 0; i < m_sprite->GetAnimationCount(); i++)
	{
		if (m_sprite->GetAnimation(i) == anim)
			break;
	}
	if (i >= m_sprite->GetAnimationCount())
	{
		QMessageBox::critical(this, "Error", "Cannot find the selected animation.");
		return;
	}

	m_sprite->RemoveAnimation(i);
	m_editor->SetFrame(m_sprite->GetAnimation(0), 0);
	m_mainWindow->UpdateSpriteContents(m_sprite);

	shared_ptr<Sprite> sprite = m_sprite;
	MainWindow* mainWindow = m_mainWindow;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			m_sprite->InsertAnimation(i, anim);
			mainWindow->UpdateSpriteContents(sprite);
			SpriteView* view = mainWindow->GetSpriteView(sprite);
			if (view)
				view->GetEditor()->SetFrame(anim, 0);
		},
		[=]() { // Redo
			m_sprite->RemoveAnimation(i);
			mainWindow->UpdateSpriteContents(sprite);
			SpriteView* view = mainWindow->GetSpriteView(sprite);
			if (view)
				view->GetEditor()->SetFrame(m_sprite->GetAnimation(0), 0);
		}
	);
}


void SpriteAnimationWidget::EditFrame(const shared_ptr<SpriteAnimation>& spriteAnim, uint16_t frame)
{
	shared_ptr<Animation> anim = spriteAnim->GetAnimation();
	if (!anim)
		return;

	bool ok;
	uint16_t oldLength = anim->GetFrameLength(frame);
	uint16_t newLength = (uint16_t)QInputDialog::getInt(this, "Animation Frame Length",
		"Length of animation frame (in 1/60 sec):", oldLength, 1, 65535, 1, &ok);
	if (!ok)
		return;

	anim->SetFrameLength(frame, newLength);
	m_mainWindow->UpdateSpriteContents(m_sprite);

	shared_ptr<Sprite> sprite = m_sprite;
	MainWindow* mainWindow = m_mainWindow;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			anim->SetFrameLength(frame, oldLength);
			mainWindow->UpdateSpriteContents(sprite);
		},
		[=]() { // Redo
			anim->SetFrameLength(frame, newLength);
			mainWindow->UpdateSpriteContents(sprite);
		}
	);
}


void SpriteAnimationWidget::DuplicateFrame(const shared_ptr<SpriteAnimation>& spriteAnim, uint16_t frame)
{
	spriteAnim->DuplicateFrame(frame);
	m_mainWindow->UpdateSpriteContents(m_sprite);

	shared_ptr<Sprite> sprite = m_sprite;
	MainWindow* mainWindow = m_mainWindow;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			spriteAnim->RemoveFrame(frame);
			mainWindow->UpdateSpriteContents(sprite);
		},
		[=]() { // Redo
			spriteAnim->DuplicateFrame(frame);
			mainWindow->UpdateSpriteContents(sprite);
		}
	);
}


void SpriteAnimationWidget::MoveFrameUp(const shared_ptr<SpriteAnimation>& spriteAnim, uint16_t frame)
{
	spriteAnim->SwapFrames(frame - 1, frame);
	m_mainWindow->UpdateSpriteContents(m_sprite);
	m_editor->SetFrame(spriteAnim, frame - 1);

	shared_ptr<Sprite> sprite = m_sprite;
	MainWindow* mainWindow = m_mainWindow;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			spriteAnim->SwapFrames(frame - 1, frame);
			mainWindow->UpdateSpriteContents(sprite);
			SpriteView* view = mainWindow->GetSpriteView(sprite);
			if (view)
				view->GetEditor()->SetFrame(spriteAnim, frame);
		},
		[=]() { // Redo
			spriteAnim->SwapFrames(frame - 1, frame);
			mainWindow->UpdateSpriteContents(sprite);
			SpriteView* view = mainWindow->GetSpriteView(sprite);
			if (view)
				view->GetEditor()->SetFrame(spriteAnim, frame - 1);
		}
	);
}


void SpriteAnimationWidget::MoveFrameDown(const shared_ptr<SpriteAnimation>& spriteAnim, uint16_t frame)
{
	spriteAnim->SwapFrames(frame, frame + 1);
	m_mainWindow->UpdateSpriteContents(m_sprite);
	m_editor->SetFrame(spriteAnim, frame + 1);

	shared_ptr<Sprite> sprite = m_sprite;
	MainWindow* mainWindow = m_mainWindow;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			spriteAnim->SwapFrames(frame, frame + 1);
			mainWindow->UpdateSpriteContents(sprite);
			SpriteView* view = mainWindow->GetSpriteView(sprite);
			if (view)
				view->GetEditor()->SetFrame(spriteAnim, frame);
		},
		[=]() { // Redo
			spriteAnim->SwapFrames(frame, frame + 1);
			mainWindow->UpdateSpriteContents(sprite);
			SpriteView* view = mainWindow->GetSpriteView(sprite);
			if (view)
				view->GetEditor()->SetFrame(spriteAnim, frame + 1);
		}
	);
}


void SpriteAnimationWidget::RemoveFrame(const shared_ptr<SpriteAnimation>& spriteAnim, uint16_t frame)
{
	shared_ptr<Animation> anim = spriteAnim->GetAnimation();
	if (!anim)
		return;
	if (frame >= anim->GetFrameCount())
		return;
	if (anim->GetFrameCount() <= 1)
		return;

	shared_ptr<RemovedFrame> frameData = make_shared<RemovedFrame>();
	frameData->length = anim->GetFrameLength(frame);
	frameData->data = spriteAnim->GetTileForSingleFrame(frame);
	spriteAnim->RemoveFrame(frame);

	m_mainWindow->UpdateSpriteContents(m_sprite);
	if (frame >= anim->GetFrameCount())
		m_editor->SetFrame(spriteAnim, anim->GetFrameCount() - 1);

	shared_ptr<Sprite> sprite = m_sprite;
	MainWindow* mainWindow = m_mainWindow;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			spriteAnim->InsertFrameFromTile(frame, frameData->data, frameData->length);
			mainWindow->UpdateSpriteContents(sprite);
			SpriteView* view = mainWindow->GetSpriteView(sprite);
			if (view)
				view->GetEditor()->SetFrame(spriteAnim, frame);
		},
		[=]() { // Redo
			spriteAnim->RemoveFrame(frame);
			mainWindow->UpdateSpriteContents(sprite);
			SpriteView* view = mainWindow->GetSpriteView(sprite);
			if (view && (frame >= anim->GetFrameCount()))
				view->GetEditor()->SetFrame(spriteAnim, anim->GetFrameCount() - 1);
		}
	);
}


void SpriteAnimationWidget::UpdateView()
{
	for (auto i : m_entries)
	{
		m_entryLayout->removeWidget(i);
		i->deleteLater();
	}
	m_entries.clear();

	for (auto& spriteAnim : m_sprite->GetAnimations())
	{
		SpriteAnimationHeaderWidget* header = new SpriteAnimationHeaderWidget(this, spriteAnim->GetName(),
			[=]() { EditAnimation(spriteAnim); },
			[=]() { DuplicateAnimation(spriteAnim); },
			[=]() { RemoveAnimation(spriteAnim); });
		m_entryLayout->addWidget(header);
		m_entries.push_back(header);

		shared_ptr<Animation> anim = spriteAnim->GetAnimation();
		uint16_t frame = 0;
		for (auto i : anim->GetFrameLengths())
		{
			char name[256];
			sprintf(name, "Frame %d, length %d/60 sec", frame, i);
			AnimationFrameWidget* widget = new AnimationFrameWidget(this, name,
				(m_editor->GetAnimation() == spriteAnim) && (m_editor->GetFrame() == frame),
				frame > 0, frame < (anim->GetFrameLengths().size() - 1),
				[=]() { m_editor->SetFrame(spriteAnim, frame); },
				[=]() { EditFrame(spriteAnim, frame); },
				[=]() { DuplicateFrame(spriteAnim, frame); },
				[=]() { RemoveFrame(spriteAnim, frame); },
				[=]() { MoveFrameUp(spriteAnim, frame); },
				[=]() { MoveFrameDown(spriteAnim, frame); });
			m_entryLayout->addWidget(widget);
			m_entries.push_back(widget);
			frame++;
		}
	}
}


void SpriteAnimationWidget::OnCreateAnimation()
{
	AnimationSettingsDialog dialog(this, shared_ptr<SpriteAnimation>());
	if (dialog.exec() != QDialog::Accepted)
		return;

	string name = dialog.GetName();
	bool loop = dialog.IsLooping();

	if (name.size() == 0)
	{
		QMessageBox::critical(this, "Error", "New animation name is invalid.");
		return;
	}

	shared_ptr<SpriteAnimation> anim = m_sprite->CreateAnimation(name);
	anim->SetLooping(loop);
	size_t i = m_sprite->GetAnimationCount();
	m_sprite->AddAnimation(anim);
	m_mainWindow->UpdateSpriteContents(m_sprite);

	shared_ptr<Sprite> sprite = m_sprite;
	MainWindow* mainWindow = m_mainWindow;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			sprite->RemoveAnimation(i);
			mainWindow->UpdateSpriteContents(sprite);
		},
		[=]() { // Redo
			sprite->AddAnimation(anim);
			mainWindow->UpdateSpriteContents(sprite);
		}
	);
}
