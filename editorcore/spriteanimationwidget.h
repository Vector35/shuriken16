#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include "sprite.h"

class MainWindow;
class SpriteEditorWidget;

class SpriteAnimationWidget: public QWidget
{
	Q_OBJECT

	struct RemovedFrame
	{
		uint16_t length;
		std::shared_ptr<Tile> data;
	};

	MainWindow* m_mainWindow;
	SpriteEditorWidget* m_editor;
	std::shared_ptr<Sprite> m_sprite;

	QVBoxLayout* m_entryLayout;
	std::vector<QWidget*> m_entries;
	QPushButton* m_createButton;

	void EditAnimation(const std::shared_ptr<SpriteAnimation>& anim);
	void DuplicateAnimation(const std::shared_ptr<SpriteAnimation>& anim);
	void RemoveAnimation(const std::shared_ptr<SpriteAnimation>& anim);
	void EditFrame(const std::shared_ptr<SpriteAnimation>& anim, uint16_t frame);
	void DuplicateFrame(const std::shared_ptr<SpriteAnimation>& anim, uint16_t frame);
	void MoveFrameUp(const std::shared_ptr<SpriteAnimation>& anim, uint16_t frame);
	void MoveFrameDown(const std::shared_ptr<SpriteAnimation>& anim, uint16_t frame);
	void RemoveFrame(const std::shared_ptr<SpriteAnimation>& anim, uint16_t frame);

public:
	SpriteAnimationWidget(QWidget* parent, SpriteEditorWidget* editor, MainWindow* mainWindow,
		std::shared_ptr<Sprite> sprite);

	void UpdateView();

private slots:
	void OnCreateAnimation();
};
