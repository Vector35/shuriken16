#pragma once

#include <QWidget>
#include <QLabel>
#include <QCheckBox>
#include <chrono>
#include "project.h"
#include "sprite.h"
#include "spriteeditorwidget.h"
#include "spritepreviewwidget.h"
#include "spriteanimationwidget.h"
#include "spritepalettewidget.h"
#include "toolwidget.h"
#include "editorview.h"

class MainWindow;

class SpriteView: public EditorView
{
	Q_OBJECT

	SpriteEditorWidget* m_editor;
	SpritePreviewWidget* m_preview;
	SpriteAnimationWidget* m_anim;
	SpritePaletteWidget* m_palettes;
	SpritePaletteWidget* m_activePalette;
	QCheckBox* m_previewAnim;

	MainWindow* m_mainWindow;
	std::shared_ptr<Project> m_project;
	std::shared_ptr<Sprite> m_sprite;

	ToolWidget* m_selectMode;
	ToolWidget* m_penMode;
	ToolWidget* m_rectMode;
	ToolWidget* m_fillRectMode;
	ToolWidget* m_circleMode;
	ToolWidget* m_lineMode;
	ToolWidget* m_fillMode;
	ToolWidget* m_flipHorizontalMode;
	ToolWidget* m_flipVerticalMode;
	ToolWidget* m_rotateMode;
	ToolWidget* m_zoomInMode;
	ToolWidget* m_zoomOutMode;

	QTimer* m_deferredUpdateTimer;
	std::chrono::steady_clock::time_point m_lastUpdate;
	bool m_firstUpdate;

public:
	SpriteView(MainWindow* parent, std::shared_ptr<Project> project,
		std::shared_ptr<Sprite> sprite);

	void UpdateView();
	void UpdateToolState();

	std::shared_ptr<Sprite> GetSprite() const { return m_sprite; }
	SpriteEditorWidget* GetEditor() { return m_editor; }

	std::shared_ptr<Palette> GetSelectedPalette() const;
	size_t GetSelectedLeftPaletteEntry() const;
	size_t GetSelectedRightPaletteEntry() const;
	void SetSelectedLeftPaletteEntry(std::shared_ptr<Palette> palette, size_t entry);
	void SetSelectedRightPaletteEntry(std::shared_ptr<Palette> palette, size_t entry);

	virtual void Cut() override;
	virtual void Copy() override;
	virtual void Paste() override;
	virtual void SelectAll() override;

private slots:
	void SetPreviewAnimation(int state);
	void OnDeferredUpdateTimer();
};
