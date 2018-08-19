#pragma once

#include <QWidget>
#include "project.h"
#include "sprite.h"

class MainWindow;
class SpriteView;

class SpritePreviewWidget: public QWidget
{
	Q_OBJECT

	MainWindow* m_mainWindow;
	SpriteView* m_view;
	std::shared_ptr<Project> m_project;
	std::shared_ptr<Sprite> m_sprite;
	std::shared_ptr<SpriteAnimation> m_animation;

	QImage* m_image;
	int m_width, m_height;

	bool m_animate;
	uint32_t m_animFrame;
	QTimer* m_animTimer;
	uint16_t m_activeFrame;

	void UpdateImageData();

public:
	SpritePreviewWidget(QWidget* parent, SpriteView* view, MainWindow* mainWindow,
		std::shared_ptr<Project> project, std::shared_ptr<Sprite> sprite);
	~SpritePreviewWidget();

	void UpdateView();

	void SetPreviewAnimation(bool anim);
	void SetActiveFrame(const std::shared_ptr<SpriteAnimation>& anim, uint16_t frame);

protected:
	virtual void paintEvent(QPaintEvent* event) override;

private slots:
	void OnAnimationTimer();
};
