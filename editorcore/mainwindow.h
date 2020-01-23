#pragma once

#include <QMainWindow>
#include <QTimer>
#include <QKeyEvent>
#include <QTabWidget>
#include <QProcess>
#include <functional>
#include <stack>
#include "project.h"
#include "projectview.h"

class UndoAction
{
	std::function<void()> m_undo, m_redo;

public:
	UndoAction(const std::function<void()>& undoAction, const std::function<void()>& redoAction);
	void Undo();
	void Redo();
};

class PaletteView;
class TileSetView;
class EffectLayerView;
class MapView;
class SpriteView;
class ActorTypeView;

class MainWindow: public QMainWindow
{
	Q_OBJECT

	QAction* m_newAction;
	QAction* m_openAction;
	QAction* m_saveAction;
	QAction* m_saveAsAction;

	QAction* m_undoAction;
	QAction* m_redoAction;
	QAction* m_cutAction;
	QAction* m_copyAction;
	QAction* m_pasteAction;
	QAction* m_selectAllAction;

	QAction* m_importNESAction;

	QAction* m_exportPNGAction;

	QAction* m_runAction;

	QTabWidget* m_tabs;
	ProjectView* m_projectView;

	std::shared_ptr<Project> m_project;
	QString m_projectPath;
	bool m_modified;

	std::stack<std::shared_ptr<UndoAction>> m_undoStack, m_redoStack;

	std::map<std::shared_ptr<Palette>, PaletteView*> m_openPalettes;
	std::map<std::shared_ptr<TileSet>, TileSetView*> m_openTileSets;
	std::map<std::shared_ptr<MapLayer>, EffectLayerView*> m_openEffectLayers;
	std::map<std::shared_ptr<Map>, MapView*> m_openMaps;
	std::map<std::shared_ptr<Sprite>, SpriteView*> m_openSprites;
	std::map<std::shared_ptr<ActorType>, ActorTypeView*> m_openActorTypes;

	QString m_basePath;
	QProcess* m_buildProcess = nullptr;
	QProcess* m_runProcess = nullptr;

	void CloseProject();
	void NewProject(const QString& path);
	bool OpenProject(const QString& path);
	bool PromptToSaveIfRequired();
	bool SaveProject(const QString& path);
	bool AttemptSave();

public:
	MainWindow(const QString& title, const QString& basePath, const QString& assetPath, QWidget* parent = nullptr);
	~MainWindow();

	void OpenPalette(std::shared_ptr<Palette> palette);
	void ClosePalette(std::shared_ptr<Palette> palette);
	void UpdatePaletteName(std::shared_ptr<Palette> palette);
	void UpdatePaletteContents(std::shared_ptr<Palette> palette);

	void OpenTileSet(std::shared_ptr<TileSet> tileSet);
	void CloseTileSet(std::shared_ptr<TileSet> tileSet);
	void UpdateTileSetName(std::shared_ptr<TileSet> tileSet);
	void UpdateTileSetContents(std::shared_ptr<TileSet> tileSet);
	TileSetView* GetTileSetView(std::shared_ptr<TileSet> tileSet);

	void OpenEffectLayer(std::shared_ptr<MapLayer> layer);
	void CloseEffectLayer(std::shared_ptr<MapLayer> layer);
	void UpdateEffectLayerName(std::shared_ptr<MapLayer> layer);
	void UpdateEffectLayerContents(std::shared_ptr<MapLayer> layer);
	EffectLayerView* GetEffectLayerView(std::shared_ptr<MapLayer> layer);

	void OpenMap(std::shared_ptr<Map> map);
	void CloseMap(std::shared_ptr<Map> map);
	void UpdateMapName(std::shared_ptr<Map> map);
	void UpdateMapContents(std::shared_ptr<Map> map);
	MapView* GetMapView(std::shared_ptr<Map> map);

	void OpenSprite(std::shared_ptr<Sprite> sprite);
	void CloseSprite(std::shared_ptr<Sprite> sprite);
	void UpdateSpriteName(std::shared_ptr<Sprite> sprite);
	void UpdateSpriteContents(std::shared_ptr<Sprite> sprite);
	SpriteView* GetSpriteView(std::shared_ptr<Sprite> sprite);

	void OpenActorType(std::shared_ptr<ActorType> actorType);
	void CloseActorType(std::shared_ptr<ActorType> actorType);
	void UpdateActorTypeName(std::shared_ptr<ActorType> actorType);
	void UpdateActorTypeContents(std::shared_ptr<ActorType> actorType);
	ActorTypeView* GetActorTypeView(std::shared_ptr<ActorType> actorType);

	void AddUndoAction(const std::function<void()>& undoAction, const std::function<void()>& redoAction);

protected:
	virtual void closeEvent(QCloseEvent* event) override;

protected slots:
	void OnNew();
	void OnOpen();
	void OnSave();
	void OnUndo();
	void OnRedo();
	void OnCut();
	void OnCopy();
	void OnPaste();
	void OnSelectAll();
	void OnRun();
	void OnBuildFinished(int exitCode, QProcess::ExitStatus exitStatus);
	void OnRunFinished(int exitCode, QProcess::ExitStatus exitStatus);
	void TabClose(int i);
	void OnImportNES();
	void OnExportPNG();
};
