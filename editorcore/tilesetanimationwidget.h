#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include "tileset.h"

class MainWindow;
class TileSetEditorWidget;

class TileSetAnimationWidget: public QWidget
{
	Q_OBJECT

	struct RemovedFrame
	{
		uint16_t length;
		std::vector<std::shared_ptr<Tile>> data;
	};

	MainWindow* m_mainWindow;
	TileSetEditorWidget* m_editor;
	std::shared_ptr<TileSet> m_tileSet;

	QVBoxLayout* m_entryLayout;
	std::vector<QWidget*> m_entries;
	QPushButton* m_createButton;

	void EditFrame(uint16_t frame);
	void DuplicateFrame(uint16_t frame);
	void MoveFrameUp(uint16_t frame);
	void MoveFrameDown(uint16_t frame);
	void RemoveFrame(uint16_t frame);

public:
	TileSetAnimationWidget(QWidget* parent, TileSetEditorWidget* editor, MainWindow* mainWindow,
		std::shared_ptr<TileSet> tileSet);

	void UpdateView();

private slots:
	void OnCreateAnimation();
};
