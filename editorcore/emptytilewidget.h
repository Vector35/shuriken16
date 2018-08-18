#pragma once

#include <QWidget>
#include "project.h"

class MainWindow;
class MapEditorWidget;

class EmptyTileWidget: public QWidget
{
	Q_OBJECT

	MainWindow* m_mainWindow;
	MapEditorWidget* m_editor;
	std::shared_ptr<Project> m_project;
	int m_width, m_height;
	QSize m_renderSize;
	int m_tileWidth, m_tileHeight;
	bool m_showHover;

public:
	EmptyTileWidget(QWidget* parent, MapEditorWidget* editor, MainWindow* mainWindow,
		std::shared_ptr<Project> project, int width, int height);

protected:
	virtual void paintEvent(QPaintEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void leaveEvent(QEvent* event) override;
};
