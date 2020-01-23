#pragma once

#include <QWidget>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include "project.h"
#include "tileset.h"
#include "tileseteditorwidget.h"
#include "tilesetpreviewwidget.h"
#include "tilesetanimationwidget.h"
#include "tilesetpalettewidget.h"
#include "tilesetassociatedsetswidget.h"
#include "toolwidget.h"
#include "editorview.h"

class MainWindow;

class TileSetView: public EditorView
{
	Q_OBJECT

	TileSetEditorWidget* m_editor;
	TileSetPreviewWidget* m_preview;
	TileSetAnimationWidget* m_anim;
	TileSetPaletteWidget* m_palettes;
	TileSetPaletteWidget* m_activePalette;
	TileSetAssociatedSetsWidget* m_associatedSets;
	QLabel* m_colCount;
	QLabel* m_tileCount;
	QCheckBox* m_previewAnim;

	MainWindow* m_mainWindow;
	std::shared_ptr<Project> m_project;
	std::shared_ptr<TileSet> m_tileSet;

	ToolWidget* m_selectMode;
	ToolWidget* m_penMode;
	ToolWidget* m_rectMode;
	ToolWidget* m_circleMode;
	ToolWidget* m_fillRectMode;
	ToolWidget* m_lineMode;
	ToolWidget* m_fillMode;
	ToolWidget* m_flipHorizontalMode;
	ToolWidget* m_flipVerticalMode;
	ToolWidget* m_rotateMode;
	ToolWidget* m_zoomInMode;
	ToolWidget* m_zoomOutMode;
	ToolWidget* m_collisionMode;
	ToolWidget* m_removeCollisions;
	ToolWidget* m_collideWithAll;

	QComboBox* m_collisionChannel;
	std::vector<uint32_t> m_collisionChannelIndicies;

	QTimer* m_deferredUpdateTimer;
	std::chrono::steady_clock::time_point m_lastUpdate;
	bool m_firstUpdate;

public:
	TileSetView(MainWindow* parent, std::shared_ptr<Project> project,
		std::shared_ptr<TileSet> tileSet);

	void UpdateView();
	void UpdateToolState();

	std::shared_ptr<TileSet> GetTileSet() const { return m_tileSet; }
	TileSetEditorWidget* GetEditor() { return m_editor; }

	std::shared_ptr<Palette> GetSelectedPalette() const;
	size_t GetSelectedLeftPaletteEntry() const;
	size_t GetSelectedRightPaletteEntry() const;
	void SetSelectedLeftPaletteEntry(std::shared_ptr<Palette> palette, size_t entry);
	void SetSelectedRightPaletteEntry(std::shared_ptr<Palette> palette, size_t entry);

	virtual void Cut() override;
	virtual void Copy() override;
	virtual void Paste() override;
	virtual void SelectAll() override;

	virtual void ExportPNG() override;

private slots:
	void ChangeColumnCount();
	void ResizeTileSet();
	void SetPreviewAnimation(int state);
	void OnDeferredUpdateTimer();
	void OnCollisionChannelChanged(int layer);
};
