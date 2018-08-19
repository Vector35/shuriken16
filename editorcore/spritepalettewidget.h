#pragma once

#include <QWidget>
#include <QGridLayout>
#include "project.h"
#include "sprite.h"
#include "palette.h"

class MainWindow;
class SpriteView;

class SpritePaletteWidget: public QWidget
{
	Q_OBJECT

	MainWindow* m_mainWindow;
	SpriteView* m_view;
	std::shared_ptr<Project> m_project;
	std::shared_ptr<Sprite> m_sprite;
	bool m_activeOnly;

	QGridLayout* m_entryLayout;
	std::vector<QWidget*> m_entries;

	void EditPaletteEntry(std::shared_ptr<Palette> palette, size_t i);
	void AddPaletteWidgets(std::shared_ptr<Palette> palette, int& row);

public:
	SpritePaletteWidget(QWidget* parent, SpriteView* view, MainWindow* mainWindow,
		std::shared_ptr<Project> project, std::shared_ptr<Sprite> sprite, bool activeOnly);

	void UpdateView();
};
