#pragma once

#include <QWidget>
#include "project.h"
#include "sprite.h"

class MainWindow;
class SpriteView;

class SpriteSelectWidget: public QWidget
{
	Q_OBJECT

	std::shared_ptr<Project> m_project;
	std::shared_ptr<Sprite> m_sprite;
	std::function<void(std::shared_ptr<Sprite> oldSprite, std::shared_ptr<Sprite> newSprite)> m_updateFunc;

	QImage* m_image;
	int m_width, m_height;

	void UpdateImageData();

public:
	SpriteSelectWidget(QWidget* parent, std::shared_ptr<Project> project, std::shared_ptr<Sprite> sprite,
		const std::function<void(std::shared_ptr<Sprite> oldSprite, std::shared_ptr<Sprite> newSprite)>& updateFunc);
	~SpriteSelectWidget();

	std::shared_ptr<Sprite> GetSprite() { return m_sprite; }
	void SetSprite(const std::shared_ptr<Sprite>& sprite);

	void UpdateView();

protected:
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void paintEvent(QPaintEvent* event) override;
};
