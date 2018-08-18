#pragma once

#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include "project.h"
#include "palette.h"
#include "editorview.h"

class MainWindow;

class PaletteEntryWidget: public QLabel
{
	Q_OBJECT

	std::function<void()> m_leftFunc;
	std::function<void()> m_rightFunc;
	std::function<void()> m_doubleClickFunc;

public:
	PaletteEntryWidget(QWidget* parent, const QColor& color, int size, int selection,
		const std::function<void()>& leftFunc, const std::function<void()>& rightFunc,
		const std::function<void()>& doubleClickFunc);

protected:
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
	virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
};

class PaletteEditorWidget: public QWidget
{
	Q_OBJECT

	MainWindow* m_mainWindow;
	std::shared_ptr<Project> m_project;
	std::shared_ptr<Palette> m_palette;

	QLabel* m_name;
	QGridLayout* m_entryLayout;

	std::vector<QWidget*> m_entries;

	void EditPaletteEntry(size_t i);

public:
	PaletteEditorWidget(QWidget* parent, MainWindow* mainWindow, std::shared_ptr<Project> project,
		std::shared_ptr<Palette> palette);

	void UpdateView();

	std::shared_ptr<Palette> GetPalette() const { return m_palette; }

private slots:
	void ResizePalette();
};

class PaletteView: public EditorView
{
	Q_OBJECT

	PaletteEditorWidget* m_widget;

public:
	PaletteView(MainWindow* parent, std::shared_ptr<Project> project,
		std::shared_ptr<Palette> palette);

	void UpdateView();

	std::shared_ptr<Palette> GetPalette() const;
};
