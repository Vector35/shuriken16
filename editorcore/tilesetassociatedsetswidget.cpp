#include <QLabel>
#include <QGuiApplication>
#include <QInputDialog>
#include <QMessageBox>
#include "tilesetassociatedsetswidget.h"
#include "associatedtilesetitemwidget.h"
#include "mainwindow.h"
#include "tilesetview.h"

using namespace std;


TileSetAssociatedSetsWidget::TileSetAssociatedSetsWidget(QWidget* parent,
	MainWindow* mainWindow, shared_ptr<Project> project, shared_ptr<TileSet> tileSet):
	QWidget(parent), m_mainWindow(mainWindow), m_project(project), m_tileSet(tileSet)
{
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 8);

	QHBoxLayout* headerLayout = new QHBoxLayout();
	QLabel* label = new QLabel("Associated Tile Sets");
	QFont headerFont = QGuiApplication::font();
	headerFont.setPointSize(headerFont.pointSize() * 5 / 4);
	label->setFont(headerFont);
	headerLayout->addWidget(label, 1);
	m_addButton = new QPushButton("Add");
	connect(m_addButton, &QPushButton::clicked, this, &TileSetAssociatedSetsWidget::OnAddSet);
	headerLayout->addWidget(m_addButton);
	headerLayout->addSpacing(16);
	layout->addLayout(headerLayout);

	m_entryLayout = new QVBoxLayout();
	layout->addLayout(m_entryLayout);

	setLayout(layout);
}


void TileSetAssociatedSetsWidget::UpdateView()
{
	for (auto i : m_entries)
	{
		m_entryLayout->removeWidget(i);
		i->deleteLater();
	}
	m_entries.clear();

	if (m_tileSet->GetAssociatedTileSets().size() == 0)
	{
		QLabel* label = new QLabel("No associated tile sets");
		QFont labelFont = QGuiApplication::font();
		labelFont.setItalic(true);
		label->setFont(labelFont);
		m_entryLayout->addWidget(label);
		m_entries.push_back(label);
		return;
	}

	for (auto& i : m_tileSet->GetAssociatedTileSets())
	{
		shared_ptr<TileSet> associatedSet = i;
		AssociatedTileSetItemWidget* widget = new AssociatedTileSetItemWidget(this, i->GetName(),
			[=]() {
				m_tileSet->RemoveAssociatedTileSet(associatedSet);
				m_mainWindow->UpdateTileSetContents(m_tileSet);

				MainWindow* mainWindow = m_mainWindow;
				shared_ptr<TileSet> tileSet = m_tileSet;
				m_mainWindow->AddUndoAction(
					[=]() { // Undo
						tileSet->AddAssociatedTileSet(associatedSet);
						mainWindow->UpdateTileSetContents(tileSet);
					},
					[=]() { // Redo
						tileSet->RemoveAssociatedTileSet(associatedSet);
						mainWindow->UpdateTileSetContents(tileSet);
					}
				);
			});
		m_entryLayout->addWidget(widget);
		m_entries.push_back(widget);
	}
}


void TileSetAssociatedSetsWidget::OnAddSet()
{
	QStringList tileSetNames;
	map<QString, shared_ptr<TileSet>> tileSets;

	vector<shared_ptr<TileSet>> sortedTileSets;
	for (auto& i : m_project->GetTileSets())
	{
		if (!m_tileSet->IsCompatibleForSmartTiles(i.second))
			sortedTileSets.push_back(i.second);
	}
	sort(sortedTileSets.begin(), sortedTileSets.end(), [&](const shared_ptr<TileSet>& a, const shared_ptr<TileSet>& b) {
		return a->GetName() < b->GetName();
	});

	if (sortedTileSets.size() == 0)
	{
		QMessageBox::critical(this, "Error", "No other tile sets are available.");
		return;
	}

	for (auto& i : sortedTileSets)
	{
		QString name = QString::fromStdString(i->GetName());
		tileSetNames.append(name);
		tileSets[name] = i;
	}

	bool ok;
	QString chosenName = QInputDialog::getItem(this, "Add Associated Tile Set", "Tile Set:", tileSetNames, 0, false, &ok);
	if (!ok)
		return;

	auto i = tileSets.find(chosenName);
	if (i == tileSets.end())
		return;
	shared_ptr<TileSet> associatedSet = i->second;

	m_tileSet->AddAssociatedTileSet(associatedSet);
	m_mainWindow->UpdateTileSetContents(m_tileSet);

	MainWindow* mainWindow = m_mainWindow;
	shared_ptr<TileSet> tileSet = m_tileSet;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			tileSet->RemoveAssociatedTileSet(associatedSet);
			mainWindow->UpdateTileSetContents(m_tileSet);
		},
		[=]() { // Redo
			tileSet->AddAssociatedTileSet(associatedSet);
			mainWindow->UpdateTileSetContents(m_tileSet);
		}
	);
}
