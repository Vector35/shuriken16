#pragma once

#include <QWidget>
#include "project.h"
#include "tileset.h"

class MainWindow;
class MapEditorWidget;

class TileSelectWidget: public QWidget
{
	Q_OBJECT

	MainWindow* m_mainWindow;
	MapEditorWidget* m_editor;
	std::shared_ptr<Project> m_project;
	std::shared_ptr<TileSet> m_tileSet;

	QImage* m_image;
	int m_width, m_height;
	QSize m_renderSize;

	int m_cols, m_rows;
	int m_tileWidth, m_tileHeight;

	bool m_showHover;
	int m_hoverX, m_hoverY;

public:
	TileSelectWidget(QWidget* parent, MapEditorWidget* editor, MainWindow* mainWindow,
		std::shared_ptr<Project> project, std::shared_ptr<TileSet> tileSet);
	~TileSelectWidget();

	void UpdateView();

protected:
	virtual void paintEvent(QPaintEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void leaveEvent(QEvent* event) override;
};
