#pragma once

#include <QWidget>
#include "project.h"
#include "tileset.h"

class MainWindow;
class TileSetView;

class TileSetPreviewWidget: public QWidget
{
	Q_OBJECT

	MainWindow* m_mainWindow;
	TileSetView* m_view;
	std::shared_ptr<Project> m_project;
	std::shared_ptr<TileSet> m_tileSet;

	QImage* m_image;
	int m_width, m_height;

	bool m_animate;
	uint32_t m_animFrame;
	QTimer* m_animTimer;
	uint16_t m_activeFrame;

	void UpdateImageData(int rows, int cols);
	void UpdateImageData();

public:
	TileSetPreviewWidget(QWidget* parent, TileSetView* view, MainWindow* mainWindow,
		std::shared_ptr<Project> project, std::shared_ptr<TileSet> tileSet);
	~TileSetPreviewWidget();

	void UpdateView();

	void SetPreviewAnimation(bool anim);
	void SetActiveFrame(uint16_t frame);

protected:
	virtual void paintEvent(QPaintEvent* event) override;

private slots:
	void OnAnimationTimer();
};
