#pragma once

#include <QWidget>
#include <QScrollArea>
#include "project.h"
#include "tileset.h"
#include "tilesetfloatinglayer.h"

class MainWindow;
class TileSetView;

enum EditorTool
{
	SelectTool,
	PenTool,
	RectangleTool,
	FilledRectangleTool,
	CircleTool,
	LineTool,
	FillTool,
	CollisionTool,
	ActorTool
};

class TileSetAnimationWidget;
class TileSetPreviewWidget;

class TileSetEditorWidget: public QWidget
{
	Q_OBJECT

	struct EditAction
	{
		size_t tileIndex, offset;
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

	struct CollisionUpdateAction
	{
		size_t tileIndex;
		uint32_t channel;
		std::vector<BoundingRect> oldCollision, newCollision;
	};

	MainWindow* m_mainWindow;
	QScrollArea* m_scrollArea;
	std::shared_ptr<Project> m_project;
	std::shared_ptr<TileSet> m_tileSet;
	uint16_t m_frame;
	uint32_t m_collisionChannel;

	std::shared_ptr<Palette> m_palette;
	size_t m_leftPaletteEntry, m_rightPaletteEntry;

	size_t m_columns, m_rows;
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
	std::vector<SetDisplayColumnsAction> m_pendingSetColumns;

	EditorTool m_tool;
	std::shared_ptr<TileSetFloatingLayer> m_floatingLayer;
	std::shared_ptr<TileSetFloatingLayer> m_selectionContents;
	std::shared_ptr<TileSetFloatingLayer> m_underSelection;

	TileSetAnimationWidget* m_animWidget;
	TileSetPreviewWidget* m_previewWidget;

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
	void UpdateCircleLayer(QMouseEvent* event);
	void UpdateLineLayer(QMouseEvent* event);
	void Fill(QMouseEvent* event);
	void AddSelectionAsCollision();
	void SetSelectionAsCollision();
	void RemoveSingleCollision();

	void CommitPendingActions();

public:
	TileSetEditorWidget(QWidget* parent, MainWindow* mainWindow, QScrollArea* scrollArea,
		std::shared_ptr<Project> project, std::shared_ptr<TileSet> tileSet);

	void UpdateView();

	void SetAnimationWidget(TileSetAnimationWidget* widget) { m_animWidget = widget; }
	void SetPreviewWidget(TileSetPreviewWidget* widget) { m_previewWidget = widget; }

	std::shared_ptr<Palette> GetSelectedPalette() const;
	size_t GetSelectedLeftPaletteEntry() const;
	size_t GetSelectedRightPaletteEntry() const;
	void SetSelectedLeftPaletteEntry(std::shared_ptr<Palette> palette, size_t entry);
	void SetSelectedRightPaletteEntry(std::shared_ptr<Palette> palette, size_t entry);

	EditorTool GetTool() const { return m_tool; }
	void SetTool(EditorTool tool);

	uint16_t GetFrame() const { return m_frame; }
	void SetFrame(uint16_t frame);

	void FlipHorizontal();
	void FlipVertical();
	void Rotate();

	void ZoomIn();
	void ZoomOut();

	bool Cut();
	bool Copy();
	bool Paste();
	void SelectAll();

	void RemoveCollisions();
	void CollideWithAll();

	void SetCollisionChannel(uint32_t channel);

	void ExportPNG();

protected:
	virtual void paintEvent(QPaintEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void leaveEvent(QEvent* event) override;
};
