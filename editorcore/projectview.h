#pragma once

#include <QWidget>
#include <QDockWidget>
#include <QVBoxLayout>
#include "project.h"

class MainWindow;

class ProjectViewWidget: public QWidget
{
	Q_OBJECT

	MainWindow* m_mainWindow;
	std::shared_ptr<Project> m_project;

	QVBoxLayout* m_paletteLayout;
	QVBoxLayout* m_tileSetLayout;
	QVBoxLayout* m_effectLayerLayout;
	QVBoxLayout* m_mapLayout;
	QVBoxLayout* m_spriteLayout;
	QVBoxLayout* m_actorTypeLayout;

	std::vector<QWidget*> m_paletteItems, m_tileSetItems, m_effectLayerItems, m_mapItems, m_spriteItems, m_actorTypeItems;

	bool ChooseName(const std::string& defaultName, std::string& chosenName,
		const QString& title, const QString& prompt);

	void RenamePalette(std::shared_ptr<Palette> palette);
	void DuplicatePalette(std::shared_ptr<Palette> palette);
	void RemovePalette(std::shared_ptr<Palette> palette);

	void RenameTileSet(std::shared_ptr<TileSet> tileSet);
	void DuplicateTileSet(std::shared_ptr<TileSet> tileSet);
	void RemoveTileSet(std::shared_ptr<TileSet> tileSet);

	void RenameEffectLayer(std::shared_ptr<MapLayer> layer);
	void DuplicateEffectLayer(std::shared_ptr<MapLayer> layer);
	void RemoveEffectLayer(std::shared_ptr<MapLayer> layer);

	void RenameMap(std::shared_ptr<Map> map);
	void DuplicateMap(std::shared_ptr<Map> map);
	void RemoveMap(std::shared_ptr<Map> map);

	void RenameSprite(std::shared_ptr<Sprite> sprite);
	void DuplicateSprite(std::shared_ptr<Sprite> sprite);
	void RemoveSprite(std::shared_ptr<Sprite> sprite);

	void RenameActorType(std::shared_ptr<ActorType> actorType);
	void DuplicateActorType(std::shared_ptr<ActorType> actorType);
	void RemoveActorType(std::shared_ptr<ActorType> actorType);

public:
	ProjectViewWidget(QWidget* parent, MainWindow* mainWindow);

	void SetProject(std::shared_ptr<Project> project);
	void UpdateList();

	virtual QSize sizeHint() const override;

private slots:
	void AddPalette();
	void AddTileset();
	void AddEffectLayer();
	void AddMap();
	void AddSprite();
	void AddActorType();
};

class ProjectView: public QDockWidget
{
	Q_OBJECT

	ProjectViewWidget* m_widget;

public:
	ProjectView(MainWindow* parent);

	void SetProject(std::shared_ptr<Project> project);
	void UpdateList();
};
