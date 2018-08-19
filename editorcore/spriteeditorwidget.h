#pragma once

#include <QWidget>
#include <QScrollArea>
#include "project.h"
#include "sprite.h"
#include "tilesetfloatinglayer.h"
#include "tileseteditorwidget.h"

class MainWindow;
class SpriteView;
class SpriteAnimationWidget;
class SpritePreviewWidget;

class SpriteEditorWidget: public QWidget
{
	Q_OBJECT

	struct EditAction
	{
		std::shared_ptr<SpriteAnimation> anim;
		size_t offset;
		uint8_t oldData, newData;
		std::shared_ptr<Palette> oldPalette, newPalette;
		uint8_t oldPaletteOffset, newPaletteOffset;
	};

	struct SelectAction
	{
		std::shared_ptr<TileSetFloatingLayer> oldSelectionContents;
		std::shared_ptr<TileSetFloatingLayer> oldUnderSelection;
		std::shared_ptr<TileSetFloatingLayer> newSelectionContents;
		std::shared_ptr<TileSetFloatingLayer> newUnderSelection;
	};

	struct SetDisplayColumnsAction
	{
		size_t oldCols;
		size_t newCols;
	};

	MainWindow* m_mainWindow;
	QScrollArea* m_scrollArea;
	std::shared_ptr<Project> m_project;
	std::shared_ptr<Sprite> m_sprite;
	std::shared_ptr<SpriteAnimation> m_animation;
	uint16_t m_frame;

	std::shared_ptr<Palette> m_palette;
	size_t m_leftPaletteEntry, m_rightPaletteEntry;

	int m_zoom;

	bool m_showHover;
	int m_hoverX, m_hoverY;
	int m_startX, m_startY;
	int m_waitForSelection;

	bool m_mouseDown;
	size_t m_mouseDownPaletteEntry;
	bool m_moveSelection;

	std::vector<EditAction> m_pendingActions;
	std::vector<SelectAction> m_pendingSelections;

	EditorTool m_tool;
	std::shared_ptr<TileSetFloatingLayer> m_floatingLayer;
	std::shared_ptr<TileSetFloatingLayer> m_selectionContents;
	std::shared_ptr<TileSetFloatingLayer> m_underSelection;

	SpriteAnimationWidget* m_animWidget;
	SpritePreviewWidget* m_previewWidget;

	void ResetPalette();

	TileSetFloatingLayerPixel GetPixel(int x, int y);
	void SetPixel(int x, int y, std::shared_ptr<Palette> palette, uint8_t entry);
	void SetPixelForMouseEvent(QMouseEvent* event);
	void CaptureLayer(std::shared_ptr<TileSetFloatingLayer> layer);
	void ApplyLayer(std::shared_ptr<TileSetFloatingLayer> layer);
	void UpdateSelectionLayer(QMouseEvent* event);
	bool IsMouseInSelection(QMouseEvent* event);
	void BeginMoveSelectionLayer();
	void MoveSelectionLayer(QMouseEvent* event);
	void FinishMoveSelectionLayer();
	void UpdateRectangleLayer(QMouseEvent* event);
	void UpdateFilledRectangleLayer(QMouseEvent* event);
	void UpdateLineLayer(QMouseEvent* event);
	void Fill(QMouseEvent* event);

	void CommitPendingActions();

public:
	SpriteEditorWidget(QWidget* parent, MainWindow* mainWindow, QScrollArea* scrollArea,
		std::shared_ptr<Project> project, std::shared_ptr<Sprite> sprite);

	void UpdateView();

	void SetAnimationWidget(SpriteAnimationWidget* widget) { m_animWidget = widget; }
	void SetPreviewWidget(SpritePreviewWidget* widget) { m_previewWidget = widget; }

	std::shared_ptr<Palette> GetSelectedPalette() const;
	size_t GetSelectedLeftPaletteEntry() const;
	size_t GetSelectedRightPaletteEntry() const;
	void SetSelectedLeftPaletteEntry(std::shared_ptr<Palette> palette, size_t entry);
	void SetSelectedRightPaletteEntry(std::shared_ptr<Palette> palette, size_t entry);

	EditorTool GetTool() const { return m_tool; }
	void SetTool(EditorTool tool);

	std::shared_ptr<SpriteAnimation> GetAnimation() { return m_animation; }
	uint16_t GetFrame() const { return m_frame; }
	void SetFrame(const std::shared_ptr<SpriteAnimation>& anim, uint16_t frame);

	void FlipHorizontal();
	void FlipVertical();
	void Rotate();

	void ZoomIn();
	void ZoomOut();

	bool Cut();
	bool Copy();
	bool Paste();
	void SelectAll();

protected:
	virtual void paintEvent(QPaintEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void leaveEvent(QEvent* event) override;
};
