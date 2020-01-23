#include "mainwindow.h"
#include "paletteview.h"
#include "tilesetview.h"
#include "effectlayerview.h"
#include "mapview.h"
#include "spriteview.h"
#include "editorview.h"
#include "actortypeview.h"
#include "importnesdialog.h"
#include <QMenu>
#include <QMenuBar>
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>
#include <QStatusBar>
#include <QCoreApplication>
#include <QPlainTextEdit>
#include <QFont>
#include <stdio.h>

using namespace std;


UndoAction::UndoAction(const function<void()>& undoAction, const function<void()>& redoAction):
	m_undo(undoAction), m_redo(redoAction)
{
}


void UndoAction::Undo()
{
	m_undo();
}


void UndoAction::Redo()
{
	m_redo();
}


MainWindow::MainWindow(const QString& title, const QString& basePath, const QString& assetPath, QWidget* parent):
	QMainWindow(parent)
{
	m_basePath = basePath;

	resize(QSize(1024, 640));
	setWindowTitle(title);

	m_tabs = new QTabWidget(this);
	m_tabs->setDocumentMode(true);
	m_tabs->setMovable(true);
	m_tabs->setTabsClosable(true);
	m_tabs->setTabBarAutoHide(false);
	setCentralWidget(m_tabs);
	connect(m_tabs, &QTabWidget::tabCloseRequested, this, &MainWindow::TabClose);

	QMenu* fileMenu = new QMenu("File");

	m_newAction = new QAction("New Project...");
	connect(m_newAction, &QAction::triggered, this, &MainWindow::OnNew);
	fileMenu->addAction(m_newAction);

	m_openAction = new QAction("Open Project...");
	m_openAction->setShortcuts(QKeySequence::Open);
	connect(m_openAction, &QAction::triggered, this, &MainWindow::OnOpen);
	fileMenu->addAction(m_openAction);

	m_saveAction = new QAction("Save Project");
	m_saveAction->setShortcuts(QKeySequence::Save);
	connect(m_saveAction, &QAction::triggered, this, &MainWindow::OnSave);
	fileMenu->addAction(m_saveAction);

	QMenu* editMenu = new QMenu("Edit");

	m_undoAction = new QAction("Undo");
	m_undoAction->setShortcuts(QKeySequence::Undo);
	connect(m_undoAction, &QAction::triggered, this, &MainWindow::OnUndo);
	editMenu->addAction(m_undoAction);

	m_redoAction = new QAction("Redo");
	m_redoAction->setShortcuts(QKeySequence::Redo);
	connect(m_redoAction, &QAction::triggered, this, &MainWindow::OnRedo);
	editMenu->addAction(m_redoAction);

	editMenu->addSeparator();

	m_cutAction = new QAction("Cut");
	m_cutAction->setShortcuts(QKeySequence::Cut);
	connect(m_cutAction, &QAction::triggered, this, &MainWindow::OnCut);
	editMenu->addAction(m_cutAction);

	m_copyAction = new QAction("Copy");
	m_copyAction->setShortcuts(QKeySequence::Copy);
	connect(m_copyAction, &QAction::triggered, this, &MainWindow::OnCopy);
	editMenu->addAction(m_copyAction);

	m_pasteAction = new QAction("Paste");
	m_pasteAction->setShortcuts(QKeySequence::Paste);
	connect(m_pasteAction, &QAction::triggered, this, &MainWindow::OnPaste);
	editMenu->addAction(m_pasteAction);

	editMenu->addSeparator();

	m_selectAllAction = new QAction("Select All");
	m_selectAllAction->setShortcuts(QKeySequence::SelectAll);
	connect(m_selectAllAction, &QAction::triggered, this, &MainWindow::OnSelectAll);
	editMenu->addAction(m_selectAllAction);

	QMenu* importMenu = new QMenu("Import");

	m_importNESAction = new QAction("NES CHR...");
	connect(m_importNESAction, &QAction::triggered, this, &MainWindow::OnImportNES);
	importMenu->addAction(m_importNESAction);

	QMenu* exportMenu = new QMenu("Export");

	m_exportPNGAction = new QAction("PNG...");
	connect(m_exportPNGAction, &QAction::triggered, this, &MainWindow::OnExportPNG);
	exportMenu->addAction(m_exportPNGAction);

	QMenu* gameMenu = new QMenu("Game");

	m_runAction = new QAction("Run");
	m_runAction->setShortcut(QString("Ctrl+R"));
	connect(m_runAction, &QAction::triggered, this, &MainWindow::OnRun);
	gameMenu->addAction(m_runAction);

	menuBar()->addMenu(fileMenu);
	menuBar()->addMenu(editMenu);
	menuBar()->addMenu(importMenu);
	menuBar()->addMenu(exportMenu);
	menuBar()->addMenu(gameMenu);

	m_projectView = new ProjectView(this);
	addDockWidget(Qt::LeftDockWidgetArea, m_projectView);

	if (!OpenProject(assetPath))
		NewProject(assetPath);
}


MainWindow::~MainWindow()
{
}


void MainWindow::OpenPalette(shared_ptr<Palette> palette)
{
	auto i = m_openPalettes.find(palette);
	if (i != m_openPalettes.end())
	{
		m_tabs->setCurrentWidget(i->second);
		return;
	}

	PaletteView* editor = new PaletteView(this, m_project, palette);
	m_tabs->addTab(editor, QString::fromStdString(palette->GetName()));
	m_tabs->setCurrentWidget(editor);
	m_openPalettes[palette] = editor;
}


void MainWindow::ClosePalette(shared_ptr<Palette> palette)
{
	auto i = m_openPalettes.find(palette);
	if (i != m_openPalettes.end())
	{
		int j = m_tabs->indexOf(i->second);
		if (j != -1)
		{
			m_openPalettes.erase(i);
			m_tabs->removeTab(j);
		}
		return;
	}
}


void MainWindow::UpdatePaletteName(shared_ptr<Palette> palette)
{
	auto i = m_openPalettes.find(palette);
	if (i != m_openPalettes.end())
	{
		i->second->UpdateView();
		int j = m_tabs->indexOf(i->second);
		if (j != -1)
			m_tabs->setTabText(j, QString::fromStdString(palette->GetName()));
	}

	for (auto& i : m_openTileSets)
		i.second->UpdateView();
	for (auto& i : m_openSprites)
		i.second->UpdateView();
}


void MainWindow::UpdatePaletteContents(shared_ptr<Palette> palette)
{
	auto i = m_openPalettes.find(palette);
	if (i != m_openPalettes.end())
	{
		i->second->UpdateView();
	}

	for (auto& i : m_openTileSets)
		i.second->UpdateView();
	for (auto& i : m_openSprites)
		i.second->UpdateView();
	for (auto& i : m_openEffectLayers)
		i.second->UpdateView();
	for (auto& i : m_openMaps)
		i.second->UpdateView();
}


void MainWindow::OpenTileSet(shared_ptr<TileSet> tileSet)
{
	auto i = m_openTileSets.find(tileSet);
	if (i != m_openTileSets.end())
	{
		m_tabs->setCurrentWidget(i->second);
		return;
	}

	TileSetView* editor = new TileSetView(this, m_project, tileSet);
	m_tabs->addTab(editor, QString::fromStdString(tileSet->GetName()));
	m_tabs->setCurrentWidget(editor);
	m_openTileSets[tileSet] = editor;
}


void MainWindow::CloseTileSet(shared_ptr<TileSet> tileSet)
{
	auto i = m_openTileSets.find(tileSet);
	if (i != m_openTileSets.end())
	{
		int j = m_tabs->indexOf(i->second);
		if (j != -1)
		{
			m_openTileSets.erase(i);
			m_tabs->removeTab(j);
		}
		return;
	}
}


void MainWindow::UpdateTileSetName(shared_ptr<TileSet> tileSet)
{
	auto i = m_openTileSets.find(tileSet);
	if (i != m_openTileSets.end())
	{
		i->second->UpdateView();
		int j = m_tabs->indexOf(i->second);
		if (j != -1)
			m_tabs->setTabText(j, QString::fromStdString(tileSet->GetName()));
	}

	for (auto& i : m_openEffectLayers)
		i.second->UpdateView();
	for (auto& i : m_openMaps)
		i.second->UpdateView();
}


void MainWindow::UpdateTileSetContents(shared_ptr<TileSet> tileSet)
{
	auto i = m_openTileSets.find(tileSet);
	if (i != m_openTileSets.end())
	{
		i->second->UpdateView();
	}

	for (auto& i : m_openEffectLayers)
		i.second->UpdateView();
	for (auto& i : m_openMaps)
		i.second->UpdateView();
}


TileSetView* MainWindow::GetTileSetView(shared_ptr<TileSet> tileSet)
{
	auto i = m_openTileSets.find(tileSet);
	if (i != m_openTileSets.end())
		return i->second;
	return nullptr;
}


void MainWindow::OpenEffectLayer(shared_ptr<MapLayer> layer)
{
	auto i = m_openEffectLayers.find(layer);
	if (i != m_openEffectLayers.end())
	{
		m_tabs->setCurrentWidget(i->second);
		return;
	}

	EffectLayerView* editor = new EffectLayerView(this, m_project, layer);
	m_tabs->addTab(editor, QString::fromStdString(layer->GetName()));
	m_tabs->setCurrentWidget(editor);
	m_openEffectLayers[layer] = editor;
}


void MainWindow::CloseEffectLayer(shared_ptr<MapLayer> layer)
{
	auto i = m_openEffectLayers.find(layer);
	if (i != m_openEffectLayers.end())
	{
		int j = m_tabs->indexOf(i->second);
		if (j != -1)
		{
			m_openEffectLayers.erase(i);
			m_tabs->removeTab(j);
		}
		return;
	}
}


void MainWindow::UpdateEffectLayerName(shared_ptr<MapLayer> layer)
{
	auto i = m_openEffectLayers.find(layer);
	if (i != m_openEffectLayers.end())
	{
		i->second->UpdateView();
		int j = m_tabs->indexOf(i->second);
		if (j != -1)
			m_tabs->setTabText(j, QString::fromStdString(layer->GetName()));
	}

	for (auto& i : m_openMaps)
		i.second->UpdateView();
}


void MainWindow::UpdateEffectLayerContents(shared_ptr<MapLayer> layer)
{
	auto i = m_openEffectLayers.find(layer);
	if (i != m_openEffectLayers.end())
	{
		i->second->UpdateView();
	}

	for (auto& i : m_openMaps)
		i.second->UpdateView();
}


EffectLayerView* MainWindow::GetEffectLayerView(shared_ptr<MapLayer> layer)
{
	auto i = m_openEffectLayers.find(layer);
	if (i != m_openEffectLayers.end())
		return i->second;
	return nullptr;
}


void MainWindow::OpenMap(shared_ptr<Map> map)
{
	auto i = m_openMaps.find(map);
	if (i != m_openMaps.end())
	{
		m_tabs->setCurrentWidget(i->second);
		return;
	}

	MapView* editor = new MapView(this, m_project, map);
	m_tabs->addTab(editor, QString::fromStdString(map->GetName()));
	m_tabs->setCurrentWidget(editor);
	m_openMaps[map] = editor;
}


void MainWindow::CloseMap(shared_ptr<Map> map)
{
	auto i = m_openMaps.find(map);
	if (i != m_openMaps.end())
	{
		int j = m_tabs->indexOf(i->second);
		if (j != -1)
		{
			m_openMaps.erase(i);
			m_tabs->removeTab(j);
		}
		return;
	}
}


void MainWindow::UpdateMapName(shared_ptr<Map> map)
{
	auto i = m_openMaps.find(map);
	if (i != m_openMaps.end())
	{
		i->second->UpdateView();
		int j = m_tabs->indexOf(i->second);
		if (j != -1)
			m_tabs->setTabText(j, QString::fromStdString(map->GetName()));
	}
}


void MainWindow::UpdateMapContents(shared_ptr<Map> map)
{
	auto i = m_openMaps.find(map);
	if (i != m_openMaps.end())
	{
		i->second->UpdateView();
	}
}


MapView* MainWindow::GetMapView(std::shared_ptr<Map> map)
{
	auto i = m_openMaps.find(map);
	if (i != m_openMaps.end())
		return i->second;
	return nullptr;
}


void MainWindow::OpenSprite(shared_ptr<Sprite> sprite)
{
	auto i = m_openSprites.find(sprite);
	if (i != m_openSprites.end())
	{
		m_tabs->setCurrentWidget(i->second);
		return;
	}

	SpriteView* editor = new SpriteView(this, m_project, sprite);
	m_tabs->addTab(editor, QString::fromStdString(sprite->GetName()));
	m_tabs->setCurrentWidget(editor);
	m_openSprites[sprite] = editor;
}


void MainWindow::CloseSprite(shared_ptr<Sprite> sprite)
{
	auto i = m_openSprites.find(sprite);
	if (i != m_openSprites.end())
	{
		int j = m_tabs->indexOf(i->second);
		if (j != -1)
		{
			m_openSprites.erase(i);
			m_tabs->removeTab(j);
		}
		return;
	}
}


void MainWindow::UpdateSpriteName(shared_ptr<Sprite> sprite)
{
	auto i = m_openSprites.find(sprite);
	if (i != m_openSprites.end())
	{
		i->second->UpdateView();
		int j = m_tabs->indexOf(i->second);
		if (j != -1)
			m_tabs->setTabText(j, QString::fromStdString(sprite->GetName()));
	}
}


void MainWindow::UpdateSpriteContents(shared_ptr<Sprite> sprite)
{
	auto i = m_openSprites.find(sprite);
	if (i != m_openSprites.end())
	{
		i->second->UpdateView();
	}

	for (auto& i : m_openMaps)
		i.second->UpdateView();
	for (auto& i : m_openActorTypes)
		i.second->UpdateView();
}


SpriteView* MainWindow::GetSpriteView(std::shared_ptr<Sprite> sprite)
{
	auto i = m_openSprites.find(sprite);
	if (i != m_openSprites.end())
		return i->second;
	return nullptr;
}


void MainWindow::OpenActorType(shared_ptr<ActorType> actorType)
{
	auto i = m_openActorTypes.find(actorType);
	if (i != m_openActorTypes.end())
	{
		m_tabs->setCurrentWidget(i->second);
		return;
	}

	ActorTypeView* editor = new ActorTypeView(this, m_project, actorType);
	m_tabs->addTab(editor, QString::fromStdString(actorType->GetName()));
	m_tabs->setCurrentWidget(editor);
	m_openActorTypes[actorType] = editor;
}


void MainWindow::CloseActorType(shared_ptr<ActorType> actorType)
{
	auto i = m_openActorTypes.find(actorType);
	if (i != m_openActorTypes.end())
	{
		int j = m_tabs->indexOf(i->second);
		if (j != -1)
		{
			m_openActorTypes.erase(i);
			m_tabs->removeTab(j);
		}
		return;
	}
}


void MainWindow::UpdateActorTypeName(shared_ptr<ActorType> actorType)
{
	auto i = m_openActorTypes.find(actorType);
	if (i != m_openActorTypes.end())
	{
		i->second->UpdateView();
		int j = m_tabs->indexOf(i->second);
		if (j != -1)
			m_tabs->setTabText(j, QString::fromStdString(actorType->GetName()));
	}
}


void MainWindow::UpdateActorTypeContents(shared_ptr<ActorType> actorType)
{
	auto i = m_openActorTypes.find(actorType);
	if (i != m_openActorTypes.end())
	{
		i->second->UpdateView();
	}
}


ActorTypeView* MainWindow::GetActorTypeView(std::shared_ptr<ActorType> actorType)
{
	auto i = m_openActorTypes.find(actorType);
	if (i != m_openActorTypes.end())
		return i->second;
	return nullptr;
}


void MainWindow::AddUndoAction(const function<void()>& undoAction, const function<void()>& redoAction)
{
	m_redoStack = stack<shared_ptr<UndoAction>>();
	m_undoStack.push(make_shared<UndoAction>(undoAction, redoAction));
	m_modified = true;
}


bool MainWindow::PromptToSaveIfRequired()
{
	if (m_modified)
	{
		QMessageBox::StandardButton result = QMessageBox::question(this, "Project Modified",
			"The current project has unsaved changes. Do you want to save these changes?",
			QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
		if (result == QMessageBox::Save)
		{
			if (!AttemptSave())
				return false;
		}
		else if (result == QMessageBox::Cancel)
		{
			return false;
		}
	}
	return true;
}


void MainWindow::CloseProject()
{
	while (m_tabs->count() > 0)
		m_tabs->removeTab(0);
	m_openPalettes.clear();
	m_openTileSets.clear();
	m_openEffectLayers.clear();
	m_openMaps.clear();
}


void MainWindow::NewProject(const QString& path)
{
	CloseProject();

	m_project = make_shared<Project>();
	m_projectPath = path;
	m_modified = false;
	m_projectView->SetProject(m_project);
}


bool MainWindow::OpenProject(const QString& path)
{
	shared_ptr<Project> project = Project::Open(path);
	if (!project)
	{
		QMessageBox::critical(this, "Error", "Assets could not be loaded.");
		return false;
	}

	m_project = project;
	m_projectPath = path;
	m_modified = false;
	m_projectView->SetProject(m_project);
	return true;
}


bool MainWindow::SaveProject(const QString& path)
{
	QDir dir(path);
	dir.cdUp();
	if (!m_project->Save(dir.absoluteFilePath("assets")))
	{
		QMessageBox::critical(this, "Error", "Failed to save project to " + path);
		return false;
	}

	m_projectPath = path;
	m_modified = false;
	return true;
}


bool MainWindow::AttemptSave()
{
	return SaveProject(m_projectPath);
}


void MainWindow::OnNew()
{
	if (!PromptToSaveIfRequired())
		return;

	QString path = QFileDialog::getSaveFileName(this, "New Project", QString(), QString(),
		nullptr, QFileDialog::ShowDirsOnly);
	if (path.isNull())
		return;

	NewProject(path);
}


void MainWindow::OnOpen()
{
	if (!PromptToSaveIfRequired())
		return;

	QString path = QFileDialog::getExistingDirectory(this, "Open Project");
	if (path.isNull())
		return;

	OpenProject(path);
}


void MainWindow::OnSave()
{
	AttemptSave();
}


void MainWindow::OnUndo()
{
	if (m_undoStack.empty())
		return;
	shared_ptr<UndoAction> action = m_undoStack.top();
	m_undoStack.pop();
	m_redoStack.push(action);
	action->Undo();
	m_modified = true;
}


void MainWindow::OnRedo()
{
	if (m_redoStack.empty())
		return;
	shared_ptr<UndoAction> action = m_redoStack.top();
	m_redoStack.pop();
	m_undoStack.push(action);
	action->Redo();
	m_modified = true;
}


void MainWindow::OnCut()
{
	QWidget* widget = m_tabs->currentWidget();
	EditorView* view = dynamic_cast<EditorView*>(widget);
	if (view)
		view->Cut();
}


void MainWindow::OnCopy()
{
	QWidget* widget = m_tabs->currentWidget();
	EditorView* view = dynamic_cast<EditorView*>(widget);
	if (view)
		view->Copy();
}


void MainWindow::OnPaste()
{
	QWidget* widget = m_tabs->currentWidget();
	EditorView* view = dynamic_cast<EditorView*>(widget);
	if (view)
		view->Paste();
}


void MainWindow::OnSelectAll()
{
	QWidget* widget = m_tabs->currentWidget();
	EditorView* view = dynamic_cast<EditorView*>(widget);
	if (view)
		view->SelectAll();
}


void MainWindow::OnRun()
{
	if (m_buildProcess)
	{
		QMessageBox::critical(this, "Error", "Build is already running.");
		return;
	}
	if (m_runProcess)
	{
		QMessageBox::critical(this, "Error", "Game is already running.");
		return;
	}

	m_buildProcess = new QProcess(this);
	connect(m_buildProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
		this, &MainWindow::OnBuildFinished);
	QStringList args;
	args.append("build");
	m_buildProcess->setWorkingDirectory(m_basePath);
	m_buildProcess->start("cargo", args);
}


void MainWindow::OnBuildFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	if ((exitCode != 0) || (exitStatus != QProcess::NormalExit))
	{
		QString errors = QString::fromUtf8(m_buildProcess->readAllStandardError());
		QPlainTextEdit* errorWidget = new QPlainTextEdit(this);
		errorWidget->setPlainText(errors);
		errorWidget->setReadOnly(true);
		QFont font("Menlo", 12);
		font.setStyleHint(QFont::TypeWriter);
		errorWidget->setFont(font);

		m_tabs->addTab(errorWidget, "Build Errors");
		m_tabs->setCurrentWidget(errorWidget);

		m_buildProcess->deleteLater();
		m_buildProcess = nullptr;
		return;
	}

	m_buildProcess->deleteLater();
	m_buildProcess = nullptr;

	m_runProcess = new QProcess(this);
	connect(m_runProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
		this, &MainWindow::OnRunFinished);
	QStringList args;
	args.append("run");
	m_runProcess->setWorkingDirectory(m_basePath);
	m_runProcess->start("cargo", args);
}


void MainWindow::OnRunFinished(int, QProcess::ExitStatus)
{
	m_runProcess->deleteLater();
	m_runProcess = nullptr;
}


void MainWindow::TabClose(int i)
{
	QWidget* widget = m_tabs->widget(i);

	PaletteView* paletteView = dynamic_cast<PaletteView*>(widget);
	if (paletteView)
	{
		m_openPalettes.erase(paletteView->GetPalette());
		m_tabs->removeTab(i);
		return;
	}

	TileSetView* tileSetView = dynamic_cast<TileSetView*>(widget);
	if (tileSetView)
	{
		m_openTileSets.erase(tileSetView->GetTileSet());
		m_tabs->removeTab(i);
		return;
	}

	EffectLayerView* effectLayerView = dynamic_cast<EffectLayerView*>(widget);
	if (effectLayerView)
	{
		m_openEffectLayers.erase(effectLayerView->GetEffectLayer());
		m_tabs->removeTab(i);
		return;
	}

	MapView* mapView = dynamic_cast<MapView*>(widget);
	if (mapView)
	{
		m_openMaps.erase(mapView->GetMap());
		m_tabs->removeTab(i);
		return;
	}

	SpriteView* spriteView = dynamic_cast<SpriteView*>(widget);
	if (spriteView)
	{
		m_openSprites.erase(spriteView->GetSprite());
		m_tabs->removeTab(i);
		return;
	}

	ActorTypeView* actorTypeView = dynamic_cast<ActorTypeView*>(widget);
	if (actorTypeView)
	{
		m_openActorTypes.erase(actorTypeView->GetActorType());
		m_tabs->removeTab(i);
		return;
	}

	if (dynamic_cast<QPlainTextEdit*>(widget))
	{
		m_tabs->removeTab(i);
		return;
	}
}


void MainWindow::closeEvent(QCloseEvent* event)
{
	if (PromptToSaveIfRequired())
	{
		event->accept();
		m_modified = false;
	}
	else
	{
		event->ignore();
	}
}


void MainWindow::OnImportNES()
{
	ImportNESDialog dialog(this, m_project);
	if (dialog.exec() == QDialog::Accepted)
	{
		shared_ptr<TileSet> tileSet = dialog.GetResult();
		UpdateTileSetContents(tileSet);
		OpenTileSet(tileSet);
		AddUndoAction(
			[=]() { // Undo
				UpdateTileSetContents(tileSet);
				CloseTileSet(tileSet);
				m_project->DeleteTileSet(tileSet);
				m_projectView->UpdateList();
			},
			[=]() { // Redo
				m_project->AddTileSet(tileSet);
				UpdateTileSetContents(tileSet);
				m_projectView->UpdateList();
			}
		);
		m_projectView->UpdateList();
	}
}


void MainWindow::OnExportPNG()
{
	QWidget* widget = m_tabs->currentWidget();
	EditorView* view = dynamic_cast<EditorView*>(widget);
	if (view)
		view->ExportPNG();
}
