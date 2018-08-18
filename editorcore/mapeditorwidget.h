#pragma once

#include <QAbstractScrollArea>
#include <QTimer>
#include "project.h"
#include "map.h"
#include "tileseteditorwidget.h"
#include "mapfloatinglayer.h"
#include "renderer.h"

class MainWindow;
class MapLayerWidget;
class MapTileWidget;

class MapEditorWidget: public QAbstractScrollArea
{
	Q_OBJECT

	struct EditAction
	{
		std::shared_ptr<MapLayer> layer;
		size_t x, y;
		std::shared_ptr<TileSet> oldTileSet, newTileSet;
		size_t oldTileIndex, newTileIndex;
	};

	struct SelectAction
	{
		std::shared_ptr<MapFloatingLayer> oldSelectionContents;
		std::shared_ptr<MapFloatingLayer> oldUnderSelection;
		std::shared_ptr<MapFloatingLayer> newSelectionContents;
		std::shared_ptr<MapFloatingLayer> newUnderSelection;
	};

	MainWindow* m_mainWindow;
	MapLayerWidget* m_layerWidget;
	MapTileWidget* m_tileWidget;
	bool m_effectLayerEditor;
	std::shared_ptr<Project> m_project;
	std::shared_ptr<Map> m_map;
	std::shared_ptr<MapLayer> m_layer;

	std::shared_ptr<TileSet> m_leftTileSet, m_rightTileSet;
	size_t m_leftTileIndex, m_rightTileIndex;

	int m_zoom;
	bool m_fadeOtherLayers;

	bool m_animate;
	QTimer* m_animTimer;

	bool m_showHover;
	int m_hoverX, m_hoverY;

	bool m_mouseDown;
	std::shared_ptr<TileSet> m_mouseDownTileSet;
	size_t m_mouseDownTileIndex;
	bool m_moveSelection;

	std::vector<EditAction> m_pendingActions;
	std::vector<SelectAction> m_pendingSelections;

	QImage* m_image;
	int m_renderWidth, m_renderHeight;
	std::shared_ptr<Renderer> m_renderer;
	std::map<std::shared_ptr<MapLayer>, bool> m_visibility;

	EditorTool m_tool;
	int m_startX, m_startY;
	int m_waitForSelection;
	std::shared_ptr<MapFloatingLayer> m_floatingLayer;
	std::shared_ptr<MapFloatingLayer> m_selectionContents;
	std::shared_ptr<MapFloatingLayer> m_underSelection;

	MapFloatingLayerTile GetTile(int x, int y);
	bool SetTile(int x, int y, std::shared_ptr<TileSet> tileSet, uint16_t index);
	void SetTileForMouseEvent(QMouseEvent* event);
	void CaptureLayer(std::shared_ptr<MapFloatingLayer> layer);
	void ApplyLayer(std::shared_ptr<MapFloatingLayer> layer);
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
	MapEditorWidget(QWidget* parent, MainWindow* mainWindow, std::shared_ptr<Project> project,
		std::shared_ptr<Map> map, bool effectLayer);

	void UpdateView();

	std::shared_ptr<MapLayer> GetActiveLayer() const;
	void SetActiveLayer(std::shared_ptr<MapLayer> layer);

	std::shared_ptr<TileSet> GetSelectedLeftTileSet() const;
	size_t GetSelectedLeftTileIndex() const;
	std::shared_ptr<TileSet> GetSelectedRightTileSet() const;
	size_t GetSelectedRightTileIndex() const;
	void SetSelectedLeftTile(std::shared_ptr<TileSet> tileSet, size_t entry);
	void SetSelectedRightTile(std::shared_ptr<TileSet> tileSet, size_t entry);

	void SetLayerWidget(MapLayerWidget* widget) { m_layerWidget = widget; }
	void SetTileWidget(MapTileWidget* widget) { m_tileWidget = widget; }

	bool IsFadeOtherLayersEnabled() const { return m_fadeOtherLayers; }
	void SetFadeOtherLayersEnabled(bool enabled) { m_fadeOtherLayers = enabled; }

	bool IsAnimationEnabled() const { return m_animate; }
	void SetAnimationEnabled(bool enabled);

	bool IsLayerVisible(std::shared_ptr<MapLayer> layer);
	void SetLayerVisibility(std::shared_ptr<MapLayer> layer, bool visible);

	EditorTool GetTool() const { return m_tool; }
	void SetTool(EditorTool tool);

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
	virtual void resizeEvent(QResizeEvent* event) override;

private slots:
	void OnAnimationTimer();
};
