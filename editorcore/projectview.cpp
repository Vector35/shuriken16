#include <QScrollArea>
#include <QHBoxLayout>
#include <QGuiApplication>
#include <QPushButton>
#include <QLabel>
#include <QInputDialog>
#include <QMessageBox>
#include "projectview.h"
#include "projectitemwidget.h"
#include "addpalettedialog.h"
#include "addtilesetdialog.h"
#include "addeffectlayerdialog.h"
#include "addmapdialog.h"
#include "addspritedialog.h"
#include "mainwindow.h"
#include "theme.h"

using namespace std;


ProjectViewWidget::ProjectViewWidget(QWidget* parent, MainWindow* mainWindow):
	QWidget(parent), m_mainWindow(mainWindow)
{
	QPalette style(this->palette());
	style.setColor(QPalette::Window, Theme::backgroundDark);
	setPalette(style);

	QVBoxLayout* layout = new QVBoxLayout();

	QHBoxLayout* mapHeaderLayout = new QHBoxLayout();
	QLabel* mapLabel = new QLabel("Maps");
	QFont headerFont = QGuiApplication::font();
	headerFont.setPointSize(headerFont.pointSize() * 5 / 4);
	mapLabel->setFont(headerFont);
	mapHeaderLayout->addWidget(mapLabel, 1);

	QPushButton* mapAddButton = new QPushButton("New...");
	mapHeaderLayout->addWidget(mapAddButton);

	layout->addLayout(mapHeaderLayout);

	m_mapLayout = new QVBoxLayout();
	layout->addLayout(m_mapLayout);
	layout->addSpacing(16);

	QHBoxLayout* spriteHeaderLayout = new QHBoxLayout();
	QLabel* spriteLabel = new QLabel("Sprites");
	spriteLabel->setFont(headerFont);
	spriteHeaderLayout->addWidget(spriteLabel, 1);

	QPushButton* spriteAddButton = new QPushButton("New...");
	spriteHeaderLayout->addWidget(spriteAddButton);

	layout->addLayout(spriteHeaderLayout);

	m_spriteLayout = new QVBoxLayout();
	layout->addLayout(m_spriteLayout);
	layout->addSpacing(16);

	QHBoxLayout* tileSetHeaderLayout = new QHBoxLayout();
	QLabel* tileSetLabel = new QLabel("Tile Sets");
	tileSetLabel->setFont(headerFont);
	tileSetHeaderLayout->addWidget(tileSetLabel, 1);

	QPushButton* tileSetAddButton = new QPushButton("New...");
	tileSetHeaderLayout->addWidget(tileSetAddButton);

	layout->addLayout(tileSetHeaderLayout);

	m_tileSetLayout = new QVBoxLayout();
	layout->addLayout(m_tileSetLayout);
	layout->addSpacing(16);

	QHBoxLayout* effectLayerHeaderLayout = new QHBoxLayout();
	QLabel* effectLayerLabel = new QLabel("Effect Layers");
	effectLayerLabel->setFont(headerFont);
	effectLayerHeaderLayout->addWidget(effectLayerLabel, 1);

	QPushButton* effectLayerAddButton = new QPushButton("New...");
	effectLayerHeaderLayout->addWidget(effectLayerAddButton);

	layout->addLayout(effectLayerHeaderLayout);

	m_effectLayerLayout = new QVBoxLayout();
	layout->addLayout(m_effectLayerLayout);
	layout->addSpacing(16);

	QHBoxLayout* paletteHeaderLayout = new QHBoxLayout();
	QLabel* paletteLabel = new QLabel("Palettes");
	paletteLabel->setFont(headerFont);
	paletteHeaderLayout->addWidget(paletteLabel, 1);

	QPushButton* paletteAddButton = new QPushButton("New...");
	paletteHeaderLayout->addWidget(paletteAddButton);

	layout->addLayout(paletteHeaderLayout);

	m_paletteLayout = new QVBoxLayout();
	layout->addLayout(m_paletteLayout);

	layout->addStretch(1);
	setLayout(layout);

	connect(paletteAddButton, &QPushButton::clicked, this, &ProjectViewWidget::AddPalette);
	connect(tileSetAddButton, &QPushButton::clicked, this, &ProjectViewWidget::AddTileset);
	connect(effectLayerAddButton, &QPushButton::clicked, this, &ProjectViewWidget::AddEffectLayer);
	connect(mapAddButton, &QPushButton::clicked, this, &ProjectViewWidget::AddMap);
	connect(spriteAddButton, &QPushButton::clicked, this, &ProjectViewWidget::AddSprite);
}


void ProjectViewWidget::SetProject(shared_ptr<Project> project)
{
	m_project = project;
	UpdateList();
}


bool ProjectViewWidget::ChooseName(const string& defaultName, string& chosenName,
	const QString& title, const QString& prompt)
{
	bool ok;
	chosenName = QInputDialog::getText(this, title, prompt, QLineEdit::Normal,
		QString::fromStdString(defaultName), &ok).toStdString();
	return ok;
}


void ProjectViewWidget::RenamePalette(shared_ptr<Palette> palette)
{
	string oldName = palette->GetName();
	string newName;
	if (ChooseName(oldName, newName, "Rename", "Name:"))
	{
		if ((newName.size() == 0) || (!m_project->RenamePalette(palette, newName)))
		{
			QMessageBox::critical(this, "Error", "New palette name is invalid or already in use.");
			return;
		}
		m_mainWindow->UpdatePaletteName(palette);
		m_mainWindow->AddUndoAction(
			[=]() { // Undo
				m_project->RenamePalette(palette, oldName);
				m_mainWindow->UpdatePaletteName(palette);
				UpdateList();
			},
			[=]() { // Redo
				m_project->RenamePalette(palette, newName);
				m_mainWindow->UpdatePaletteName(palette);
				UpdateList();
			}
		);
		UpdateList();
	}
}


void ProjectViewWidget::DuplicatePalette(shared_ptr<Palette> palette)
{
	string oldName = palette->GetName();
	string newName;
	if (ChooseName(oldName, newName, "Duplicate", "Name of duplicated palette:"))
	{
		shared_ptr<Palette> newCopy = make_shared<Palette>(*palette);
		newCopy->SetName(newName);
		if ((newName.size() == 0) || (!m_project->AddPalette(newCopy)))
		{
			QMessageBox::critical(this, "Error", "New palette name is invalid or already in use.");
			return;
		}
		m_mainWindow->OpenPalette(newCopy);
		m_mainWindow->UpdatePaletteContents(newCopy);
		m_mainWindow->AddUndoAction(
			[=]() { // Undo
				m_mainWindow->ClosePalette(newCopy);
				m_project->DeletePalette(newCopy);
				m_mainWindow->UpdatePaletteContents(newCopy);
				UpdateList();
			},
			[=]() { // Redo
				m_project->AddPalette(newCopy);
				m_mainWindow->UpdatePaletteContents(newCopy);
				UpdateList();
			}
		);
		UpdateList();
	}
}


void ProjectViewWidget::RemovePalette(shared_ptr<Palette> palette)
{
	if (m_project->GetPalettes().size() <= 1)
	{
		QMessageBox::critical(this, "Error", "Cannot remove the last palette.");
		return;
	}

	if (m_project->GetTileSetsUsingPalette(palette).size() != 0)
	{
		QMessageBox::critical(this, "Error", "This palette is still in use. Open the palette to view its "
			"dependency list, clear the dependencies, and try again.");
		return;
	}

	if (QMessageBox::question(this, "Delete Palette", QString("Are you sure you want to remove the palette '") +
		QString::fromStdString(palette->GetName()) + QString("'?"), QMessageBox::Yes,
		QMessageBox::No | QMessageBox::Default | QMessageBox::Escape, QMessageBox::NoButton) !=
		QMessageBox::Yes)
		return;

	m_mainWindow->ClosePalette(palette);
	m_project->DeletePalette(palette);
	m_mainWindow->UpdatePaletteContents(palette);
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			m_project->AddPalette(palette);
			m_mainWindow->UpdatePaletteContents(palette);
			UpdateList();
		},
		[=]() { // Redo
			m_mainWindow->ClosePalette(palette);
			m_project->DeletePalette(palette);
			m_mainWindow->UpdatePaletteContents(palette);
			UpdateList();
		}
	);
	UpdateList();
}


void ProjectViewWidget::RenameTileSet(shared_ptr<TileSet> tileSet)
{
	string oldName = tileSet->GetName();
	string newName;
	if (ChooseName(oldName, newName, "Rename", "Name:"))
	{
		if ((newName.size() == 0) || (!m_project->RenameTileSet(tileSet, newName)))
		{
			QMessageBox::critical(this, "Error", "New tile set name is invalid or already in use.");
			return;
		}
		m_mainWindow->UpdateTileSetName(tileSet);
		m_mainWindow->AddUndoAction(
			[=]() { // Undo
				m_project->RenameTileSet(tileSet, oldName);
				m_mainWindow->UpdateTileSetName(tileSet);
				UpdateList();
			},
			[=]() { // Redo
				m_project->RenameTileSet(tileSet, newName);
				m_mainWindow->UpdateTileSetName(tileSet);
				UpdateList();
			}
		);
		UpdateList();
	}
}


void ProjectViewWidget::DuplicateTileSet(shared_ptr<TileSet> tileSet)
{
	string oldName = tileSet->GetName();
	string newName;
	if (ChooseName(oldName, newName, "Duplicate", "Name of duplicated tile set:"))
	{
		shared_ptr<TileSet> newCopy = make_shared<TileSet>(*tileSet);
		newCopy->SetName(newName);
		if ((newName.size() == 0) || (!m_project->AddTileSet(newCopy)))
		{
			QMessageBox::critical(this, "Error", "New tile set name is invalid or already in use.");
			return;
		}
		m_mainWindow->OpenTileSet(newCopy);
		m_mainWindow->UpdateTileSetContents(newCopy);
		m_mainWindow->AddUndoAction(
			[=]() { // Undo
				m_mainWindow->CloseTileSet(newCopy);
				m_project->DeleteTileSet(newCopy);
				m_mainWindow->UpdateTileSetContents(newCopy);
				UpdateList();
			},
			[=]() { // Redo
				m_project->AddTileSet(newCopy);
				m_mainWindow->UpdateTileSetContents(newCopy);
				UpdateList();
			}
		);
		UpdateList();
	}
}


void ProjectViewWidget::RemoveTileSet(shared_ptr<TileSet> tileSet)
{
	if (m_project->GetMapsUsingTileSet(tileSet).size() != 0)
	{
		QMessageBox::critical(this, "Error", "This tile set is still in use. Open the tile set to view its "
			"dependency list, clear the dependencies, and try again.");
		return;
	}
	if (m_project->GetEffectLayersUsingTileSet(tileSet).size() != 0)
	{
		QMessageBox::critical(this, "Error", "This tile set is still in use. Open the tile set to view its "
			"dependency list, clear the dependencies, and try again.");
		return;
	}

	if (QMessageBox::question(this, "Delete Tile Set", QString("Are you sure you want to remove the tile set '") +
		QString::fromStdString(tileSet->GetName()) + QString("'?"), QMessageBox::Yes,
		QMessageBox::No | QMessageBox::Default | QMessageBox::Escape, QMessageBox::NoButton) !=
		QMessageBox::Yes)
		return;

	m_mainWindow->CloseTileSet(tileSet);
	m_project->DeleteTileSet(tileSet);
	m_mainWindow->UpdateTileSetContents(tileSet);
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			m_project->AddTileSet(tileSet);
			m_mainWindow->UpdateTileSetContents(tileSet);
			UpdateList();
		},
		[=]() { // Redo
			m_mainWindow->CloseTileSet(tileSet);
			m_project->DeleteTileSet(tileSet);
			m_mainWindow->UpdateTileSetContents(tileSet);
			UpdateList();
		}
	);
	UpdateList();
}


void ProjectViewWidget::RenameEffectLayer(shared_ptr<MapLayer> layer)
{
	string oldName = layer->GetName();
	string newName;
	if (ChooseName(oldName, newName, "Rename", "Name:"))
	{
		if ((newName.size() == 0) || (!m_project->RenameEffectLayer(layer, newName)))
		{
			QMessageBox::critical(this, "Error", "New effect layer name is invalid or already in use.");
			return;
		}
		m_mainWindow->UpdateEffectLayerName(layer);
		m_mainWindow->AddUndoAction(
			[=]() { // Undo
				m_project->RenameEffectLayer(layer, oldName);
				m_mainWindow->UpdateEffectLayerName(layer);
				UpdateList();
			},
			[=]() { // Redo
				m_project->RenameEffectLayer(layer, newName);
				m_mainWindow->UpdateEffectLayerName(layer);
				UpdateList();
			}
		);
		UpdateList();
	}
}


void ProjectViewWidget::DuplicateEffectLayer(shared_ptr<MapLayer> layer)
{
	string oldName = layer->GetName();
	string newName;
	if (ChooseName(oldName, newName, "Duplicate", "Name of duplicated effect layer:"))
	{
		shared_ptr<MapLayer> newCopy = make_shared<MapLayer>(*layer);
		newCopy->SetName(newName);
		if ((newName.size() == 0) || (!m_project->AddEffectLayer(newCopy)))
		{
			QMessageBox::critical(this, "Error", "New effect layer name is invalid or already in use.");
			return;
		}
		m_mainWindow->OpenEffectLayer(newCopy);
		m_mainWindow->UpdateEffectLayerContents(newCopy);
		m_mainWindow->AddUndoAction(
			[=]() { // Undo
				m_mainWindow->CloseEffectLayer(newCopy);
				m_project->DeleteEffectLayer(newCopy);
				m_mainWindow->UpdateEffectLayerContents(newCopy);
				UpdateList();
			},
			[=]() { // Redo
				m_project->AddEffectLayer(newCopy);
				m_mainWindow->UpdateEffectLayerContents(newCopy);
				UpdateList();
			}
		);
		UpdateList();
	}
}


void ProjectViewWidget::RemoveEffectLayer(shared_ptr<MapLayer> layer)
{
	if (m_project->GetMapsUsingEffectLayer(layer).size() != 0)
	{
		QMessageBox::critical(this, "Error", "This effect layer is still in use. Open the effect layer to view its "
			"dependency list, clear the dependencies, and try again.");
		return;
	}

	if (QMessageBox::question(this, "Delete Effect Layer", QString("Are you sure you want to remove the effect layer '") +
		QString::fromStdString(layer->GetName()) + QString("'?"), QMessageBox::Yes,
		QMessageBox::No | QMessageBox::Default | QMessageBox::Escape, QMessageBox::NoButton) !=
		QMessageBox::Yes)
		return;

	m_mainWindow->CloseEffectLayer(layer);
	m_project->DeleteEffectLayer(layer);
	m_mainWindow->UpdateEffectLayerContents(layer);
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			m_project->AddEffectLayer(layer);
			m_mainWindow->UpdateEffectLayerContents(layer);
			UpdateList();
		},
		[=]() { // Redo
			m_mainWindow->CloseEffectLayer(layer);
			m_project->DeleteEffectLayer(layer);
			m_mainWindow->UpdateEffectLayerContents(layer);
			UpdateList();
		}
	);
	UpdateList();
}


void ProjectViewWidget::RenameMap(shared_ptr<Map> map)
{
	string oldName = map->GetName();
	string newName;
	if (ChooseName(oldName, newName, "Rename", "Name:"))
	{
		if ((newName.size() == 0) || (!m_project->RenameMap(map, newName)))
		{
			QMessageBox::critical(this, "Error", "New map name is invalid or already in use.");
			return;
		}
		m_mainWindow->UpdateMapName(map);
		m_mainWindow->AddUndoAction(
			[=]() { // Undo
				m_project->RenameMap(map, oldName);
				m_mainWindow->UpdateMapName(map);
				UpdateList();
			},
			[=]() { // Redo
				m_project->RenameMap(map, newName);
				m_mainWindow->UpdateMapName(map);
				UpdateList();
			}
		);
		UpdateList();
	}
}


void ProjectViewWidget::DuplicateMap(shared_ptr<Map> map)
{
	string oldName = map->GetName();
	string newName;
	if (ChooseName(oldName, newName, "Duplicate", "Name of duplicated map:"))
	{
		shared_ptr<Map> newCopy = make_shared<Map>(*map);
		newCopy->SetName(newName);
		if ((newName.size() == 0) || (!m_project->AddMap(newCopy)))
		{
			QMessageBox::critical(this, "Error", "New map name is invalid or already in use.");
			return;
		}
		m_mainWindow->OpenMap(newCopy);
		m_mainWindow->UpdateMapContents(newCopy);
		m_mainWindow->AddUndoAction(
			[=]() { // Undo
				m_mainWindow->CloseMap(newCopy);
				m_project->DeleteMap(newCopy);
				m_mainWindow->UpdateMapContents(newCopy);
				UpdateList();
			},
			[=]() { // Redo
				m_project->AddMap(newCopy);
				m_mainWindow->UpdateMapContents(newCopy);
				UpdateList();
			}
		);
		UpdateList();
	}
}


void ProjectViewWidget::RemoveMap(shared_ptr<Map> map)
{
	if (QMessageBox::question(this, "Delete Map", QString("Are you sure you want to remove the map '") +
		QString::fromStdString(map->GetName()) + QString("'?"), QMessageBox::Yes,
		QMessageBox::No | QMessageBox::Default | QMessageBox::Escape, QMessageBox::NoButton) !=
		QMessageBox::Yes)
		return;

	m_mainWindow->CloseMap(map);
	m_project->DeleteMap(map);
	m_mainWindow->UpdateMapContents(map);
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			m_project->AddMap(map);
			m_mainWindow->UpdateMapContents(map);
			UpdateList();
		},
		[=]() { // Redo
			m_mainWindow->CloseMap(map);
			m_project->DeleteMap(map);
			m_mainWindow->UpdateMapContents(map);
			UpdateList();
		}
	);
	UpdateList();
}


void ProjectViewWidget::RenameSprite(shared_ptr<Sprite> sprite)
{
	string oldName = sprite->GetName();
	string newName;
	if (ChooseName(oldName, newName, "Rename", "Name:"))
	{
		if ((newName.size() == 0) || (!m_project->RenameSprite(sprite, newName)))
		{
			QMessageBox::critical(this, "Error", "New sprite name is invalid or already in use.");
			return;
		}
		m_mainWindow->UpdateSpriteName(sprite);
		m_mainWindow->AddUndoAction(
			[=]() { // Undo
				m_project->RenameSprite(sprite, oldName);
				m_mainWindow->UpdateSpriteName(sprite);
				UpdateList();
			},
			[=]() { // Redo
				m_project->RenameSprite(sprite, newName);
				m_mainWindow->UpdateSpriteName(sprite);
				UpdateList();
			}
		);
		UpdateList();
	}
}


void ProjectViewWidget::DuplicateSprite(shared_ptr<Sprite> sprite)
{
	string oldName = sprite->GetName();
	string newName;
	if (ChooseName(oldName, newName, "Duplicate", "Name of duplicated sprite:"))
	{
		shared_ptr<Sprite> newCopy = make_shared<Sprite>(*sprite);
		newCopy->SetName(newName);
		if ((newName.size() == 0) || (!m_project->AddSprite(newCopy)))
		{
			QMessageBox::critical(this, "Error", "New sprite name is invalid or already in use.");
			return;
		}
		m_mainWindow->OpenSprite(newCopy);
		m_mainWindow->UpdateSpriteContents(newCopy);
		m_mainWindow->AddUndoAction(
			[=]() { // Undo
				m_mainWindow->CloseSprite(newCopy);
				m_project->DeleteSprite(newCopy);
				m_mainWindow->UpdateSpriteContents(newCopy);
				UpdateList();
			},
			[=]() { // Redo
				m_project->AddSprite(newCopy);
				m_mainWindow->UpdateSpriteContents(newCopy);
				UpdateList();
			}
		);
		UpdateList();
	}
}


void ProjectViewWidget::RemoveSprite(shared_ptr<Sprite> sprite)
{
	if (QMessageBox::question(this, "Delete Sprite", QString("Are you sure you want to remove the sprite '") +
		QString::fromStdString(sprite->GetName()) + QString("'?"), QMessageBox::Yes,
		QMessageBox::No | QMessageBox::Default | QMessageBox::Escape, QMessageBox::NoButton) !=
		QMessageBox::Yes)
		return;

	m_mainWindow->CloseSprite(sprite);
	m_project->DeleteSprite(sprite);
	m_mainWindow->UpdateSpriteContents(sprite);
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			m_project->AddSprite(sprite);
			m_mainWindow->UpdateSpriteContents(sprite);
			UpdateList();
		},
		[=]() { // Redo
			m_mainWindow->CloseSprite(sprite);
			m_project->DeleteSprite(sprite);
			m_mainWindow->UpdateSpriteContents(sprite);
			UpdateList();
		}
	);
	UpdateList();
}


void ProjectViewWidget::UpdateList()
{
	for (auto i : m_paletteItems)
	{
		m_paletteLayout->removeWidget(i);
		i->deleteLater();
	}
	for (auto i : m_tileSetItems)
	{
		m_tileSetLayout->removeWidget(i);
		i->deleteLater();
	}
	for (auto i : m_effectLayerItems)
	{
		m_effectLayerLayout->removeWidget(i);
		i->deleteLater();
	}
	for (auto i : m_mapItems)
	{
		m_mapLayout->removeWidget(i);
		i->deleteLater();
	}
	for (auto i : m_spriteItems)
	{
		m_spriteLayout->removeWidget(i);
		i->deleteLater();
	}

	m_paletteItems.clear();
	m_tileSetItems.clear();
	m_effectLayerItems.clear();
	m_mapItems.clear();
	m_spriteItems.clear();

	for (auto& i : m_project->GetPalettes())
	{
		shared_ptr<Palette> palette = i.second;
		ProjectItemWidget* item = new ProjectItemWidget(this, i.first,
			[=]() { m_mainWindow->OpenPalette(palette); },
			[=]() { RenamePalette(palette); },
			[=]() { DuplicatePalette(palette); },
			[=]() { RemovePalette(palette); }
		);
		m_paletteLayout->addWidget(item);
		m_paletteItems.push_back(item);
	}

	for (auto& i : m_project->GetTileSets())
	{
		shared_ptr<TileSet> tileSet = i.second;
		ProjectItemWidget* item = new ProjectItemWidget(this, i.first,
			[=]() { m_mainWindow->OpenTileSet(tileSet); },
			[=]() { RenameTileSet(tileSet); },
			[=]() { DuplicateTileSet(tileSet); },
			[=]() { RemoveTileSet(tileSet); }
		);
		m_tileSetLayout->addWidget(item);
		m_tileSetItems.push_back(item);
	}

	for (auto& i : m_project->GetEffectLayers())
	{
		shared_ptr<MapLayer> layer = i.second;
		ProjectItemWidget* item = new ProjectItemWidget(this, i.first,
			[=]() { m_mainWindow->OpenEffectLayer(layer); },
			[=]() { RenameEffectLayer(layer); },
			[=]() { DuplicateEffectLayer(layer); },
			[=]() { RemoveEffectLayer(layer); }
		);
		m_effectLayerLayout->addWidget(item);
		m_effectLayerItems.push_back(item);
	}

	for (auto& i : m_project->GetMaps())
	{
		shared_ptr<Map> map = i.second;
		ProjectItemWidget* item = new ProjectItemWidget(this, i.first,
			[=]() { m_mainWindow->OpenMap(map); },
			[=]() { RenameMap(map); },
			[=]() { DuplicateMap(map); },
			[=]() { RemoveMap(map); }
		);
		m_mapLayout->addWidget(item);
		m_mapItems.push_back(item);
	}

	for (auto& i : m_project->GetSprites())
	{
		shared_ptr<Sprite> sprite = i.second;
		ProjectItemWidget* item = new ProjectItemWidget(this, i.first,
			[=]() { m_mainWindow->OpenSprite(sprite); },
			[=]() { RenameSprite(sprite); },
			[=]() { DuplicateSprite(sprite); },
			[=]() { RemoveSprite(sprite); }
		);
		m_spriteLayout->addWidget(item);
		m_spriteItems.push_back(item);
	}
}


QSize ProjectViewWidget::sizeHint() const
{
	return QSize(200, 100);
}


void ProjectViewWidget::AddPalette()
{
	AddPaletteDialog dialog(this, m_project);
	if (dialog.exec() == QDialog::Accepted)
	{
		shared_ptr<Palette> palette = dialog.GetResult();
		m_mainWindow->UpdatePaletteContents(palette);
		m_mainWindow->OpenPalette(palette);
		m_mainWindow->AddUndoAction(
			[=]() { // Undo
				m_mainWindow->ClosePalette(palette);
				m_project->DeletePalette(palette);
				m_mainWindow->UpdatePaletteContents(palette);
				UpdateList();
			},
			[=]() { // Redo
				m_project->AddPalette(palette);
				m_mainWindow->UpdatePaletteContents(palette);
				UpdateList();
			}
		);
		UpdateList();
	}
}


void ProjectViewWidget::AddTileset()
{
	AddTileSetDialog dialog(this, m_project);
	if (dialog.exec() == QDialog::Accepted)
	{
		shared_ptr<TileSet> tileSet = dialog.GetResult();
		m_mainWindow->UpdateTileSetContents(tileSet);
		m_mainWindow->OpenTileSet(tileSet);
		m_mainWindow->AddUndoAction(
			[=]() { // Undo
				m_mainWindow->UpdateTileSetContents(tileSet);
				m_mainWindow->CloseTileSet(tileSet);
				m_project->DeleteTileSet(tileSet);
				UpdateList();
			},
			[=]() { // Redo
				m_project->AddTileSet(tileSet);
				m_mainWindow->UpdateTileSetContents(tileSet);
				UpdateList();
			}
		);
		UpdateList();
	}
}


void ProjectViewWidget::AddEffectLayer()
{
	AddEffectLayerDialog dialog(this, m_project);
	if (dialog.exec() == QDialog::Accepted)
	{
		shared_ptr<MapLayer> layer = dialog.GetResult();
		m_mainWindow->UpdateEffectLayerContents(layer);
		m_mainWindow->OpenEffectLayer(layer);
		m_mainWindow->AddUndoAction(
			[=]() { // Undo
				m_mainWindow->UpdateEffectLayerContents(layer);
				m_mainWindow->CloseEffectLayer(layer);
				m_project->DeleteEffectLayer(layer);
				UpdateList();
			},
			[=]() { // Redo
				m_project->AddEffectLayer(layer);
				m_mainWindow->UpdateEffectLayerContents(layer);
				UpdateList();
			}
		);
		UpdateList();
	}
}


void ProjectViewWidget::AddMap()
{
	AddMapDialog dialog(this, m_project);
	if (dialog.exec() == QDialog::Accepted)
	{
		shared_ptr<Map> map = dialog.GetResult();
		m_mainWindow->UpdateMapContents(map);
		m_mainWindow->OpenMap(map);
		m_mainWindow->AddUndoAction(
			[=]() { // Undo
				m_mainWindow->UpdateMapContents(map);
				m_mainWindow->CloseMap(map);
				m_project->DeleteMap(map);
				UpdateList();
			},
			[=]() { // Redo
				m_project->AddMap(map);
				m_mainWindow->UpdateMapContents(map);
				UpdateList();
			}
		);
		UpdateList();
	}
}


void ProjectViewWidget::AddSprite()
{
	AddSpriteDialog dialog(this, m_project);
	if (dialog.exec() == QDialog::Accepted)
	{
		shared_ptr<Sprite> sprite = dialog.GetResult();
		m_mainWindow->UpdateSpriteContents(sprite);
		m_mainWindow->OpenSprite(sprite);
		m_mainWindow->AddUndoAction(
			[=]() { // Undo
				m_mainWindow->UpdateSpriteContents(sprite);
				m_mainWindow->CloseSprite(sprite);
				m_project->DeleteSprite(sprite);
				UpdateList();
			},
			[=]() { // Redo
				m_project->AddSprite(sprite);
				m_mainWindow->UpdateSpriteContents(sprite);
				UpdateList();
			}
		);
		UpdateList();
	}
}


ProjectView::ProjectView(MainWindow* parent): QDockWidget(parent)
{
	setWindowTitle("Project");
	setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
	QScrollArea* scrollArea = new QScrollArea(this);
	m_widget = new ProjectViewWidget(this, parent);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setWidgetResizable(true);
	scrollArea->setAlignment(Qt::AlignTop);
	scrollArea->setWidget(m_widget);
	setWidget(scrollArea);
}


void ProjectView::SetProject(shared_ptr<Project> project)
{
	m_widget->SetProject(project);
}


void ProjectView::UpdateList()
{
	m_widget->UpdateList();
}
