#include <QGuiApplication>
#include <QStyle>
#include <QInputDialog>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QPushButton>
#include "maplayerwidget.h"
#include "maplayeritemwidget.h"
#include "theme.h"
#include "mapeditorwidget.h"
#include "mainwindow.h"
#include "layersettingsdialog.h"

using namespace std;


MapLayerWidget::MapLayerWidget(QWidget* parent, MapEditorWidget* editor, MainWindow* mainWindow,
	shared_ptr<Project> project, shared_ptr<Map> map):
	QWidget(parent), m_mainWindow(mainWindow), m_editor(editor), m_project(project), m_map(map)
{
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);

	QHBoxLayout* headerLayout = new QHBoxLayout();
	QLabel* layers = new QLabel("Layers");
	QFont headerFont = QGuiApplication::font();
	headerFont.setPointSize(headerFont.pointSize() * 5 / 4);
	layers->setFont(headerFont);
	headerLayout->addWidget(layers, 1);
	QPushButton* effectLayer = new QPushButton("Effect...");
	connect(effectLayer, &QPushButton::clicked, this, &MapLayerWidget::OnAddEffectLayer);
	headerLayout->addWidget(effectLayer);
	QPushButton* addLayer = new QPushButton("New...");
	connect(addLayer, &QPushButton::clicked, this, &MapLayerWidget::OnAddLayer);
	headerLayout->addWidget(addLayer);
	headerLayout->addSpacing(8);
	layout->addLayout(headerLayout);

	m_entryLayout = new QVBoxLayout();
	layout->addLayout(m_entryLayout);
	layout->addSpacing(16);

	m_fadeOtherLayers = new QCheckBox("Fade inactive layers");
	m_fadeOtherLayers->setCheckState(m_editor->IsFadeOtherLayersEnabled() ? Qt::Checked : Qt::Unchecked);
	layout->addWidget(m_fadeOtherLayers);
	connect(m_fadeOtherLayers, &QCheckBox::stateChanged, this, &MapLayerWidget::OnFadeOtherLayersChanged);

	m_animate = new QCheckBox("Preview animation");
	m_animate->setCheckState(m_editor->IsAnimationEnabled() ? Qt::Checked : Qt::Unchecked);
	layout->addWidget(m_animate);
	connect(m_animate, &QCheckBox::stateChanged, this, &MapLayerWidget::OnAnimationChanged);

	setLayout(layout);
}


void MapLayerWidget::MoveLayerUp(shared_ptr<MapLayer> layer)
{
	for (size_t i = 0; i < m_map->GetLayers().size(); i++)
	{
		if (m_map->GetLayers()[i] == layer)
		{
			if ((i + 1) >= m_map->GetLayers().size())
				break;
			m_map->SwapLayers(i, i + 1);
			m_mainWindow->UpdateMapContents(m_map);

			shared_ptr<Map> map = m_map;
			MainWindow* mainWindow = m_mainWindow;
			m_mainWindow->AddUndoAction(
				[=]() { // Undo
					map->SwapLayers(i, i + 1);
					mainWindow->UpdateMapContents(map);
				},
				[=]() { // Redo
					map->SwapLayers(i, i + 1);
					mainWindow->UpdateMapContents(map);
				}
			);
			break;
		}
	}
}


void MapLayerWidget::MoveLayerDown(shared_ptr<MapLayer> layer)
{
	for (size_t i = 0; i < m_map->GetLayers().size(); i++)
	{
		if (m_map->GetLayers()[i] == layer)
		{
			if (i == 0)
				break;
			m_map->SwapLayers(i - 1, i);
			m_mainWindow->UpdateMapContents(m_map);

			shared_ptr<Map> map = m_map;
			MainWindow* mainWindow = m_mainWindow;
			m_mainWindow->AddUndoAction(
				[=]() { // Undo
					map->SwapLayers(i - 1, i);
					mainWindow->UpdateMapContents(map);
				},
				[=]() { // Redo
					map->SwapLayers(i - 1, i);
					mainWindow->UpdateMapContents(map);
				}
			);
			break;
		}
	}
}


void MapLayerWidget::EditLayer(shared_ptr<MapLayer> layer)
{
	string oldName = layer->GetName();
	BlendMode oldBlendMode = layer->GetBlendMode();
	uint8_t oldAlpha = layer->GetAlpha();

	LayerSettingsDialog dialog(this, layer);
	if (dialog.exec() != QDialog::Accepted)
		return;

	string newName = dialog.GetName();
	BlendMode newBlendMode = dialog.GetBlendMode();
	uint8_t newAlpha = dialog.GetAlpha();

	if (newName.size() == 0)
	{
		QMessageBox::critical(this, "Error", "New layer name is invalid.");
		return;
	}
	layer->SetName(newName);
	layer->SetBlendMode(newBlendMode);
	layer->SetAlpha(newAlpha);
	m_mainWindow->UpdateMapContents(m_map);

	shared_ptr<Map> map = m_map;
	MainWindow* mainWindow = m_mainWindow;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			layer->SetName(oldName);
			layer->SetBlendMode(oldBlendMode);
			layer->SetAlpha(oldAlpha);
			mainWindow->UpdateMapContents(map);
		},
		[=]() { // Redo
			layer->SetName(newName);
			layer->SetBlendMode(newBlendMode);
			layer->SetAlpha(newAlpha);
			mainWindow->UpdateMapContents(map);
		}
	);
}


void MapLayerWidget::RemoveLayer(shared_ptr<MapLayer> layer)
{
	if (layer == m_map->GetMainLayer())
		return;

	if (QMessageBox::question(this, "Delete Layer", QString("Are you sure you want to remove the layer '") +
		QString::fromStdString(layer->GetName()) + QString("'?"), QMessageBox::Yes,
		QMessageBox::No | QMessageBox::Default | QMessageBox::Escape, QMessageBox::NoButton) !=
		QMessageBox::Yes)
		return;

	for (size_t i = 0; i < m_map->GetLayers().size(); i++)
	{
		if (m_map->GetLayers()[i] == layer)
		{
			m_map->DeleteLayer(i);
			if (m_editor->GetActiveLayer() == layer)
				m_editor->SetActiveLayer(m_map->GetMainLayer());
			m_mainWindow->UpdateMapContents(m_map);

			shared_ptr<Map> map = m_map;
			MainWindow* mainWindow = m_mainWindow;
			m_mainWindow->AddUndoAction(
				[=]() { // Undo
					map->InsertLayer(i, layer);
					mainWindow->UpdateMapContents(map);
				},
				[=]() { // Redo
					map->DeleteLayer(i);
					mainWindow->UpdateMapContents(map);
				}
			);
			break;
		}
	}
}


void MapLayerWidget::UpdateView()
{
	for (auto i : m_entries)
	{
		m_entryLayout->removeWidget(i);
		i->deleteLater();
	}
	m_entries.clear();

	// Layers are shown with the highest priority (largest layer index) on top like most editors
	bool activeLayerValid = false;
	for (auto i = m_map->GetLayers().rbegin(); i != m_map->GetLayers().rend(); ++i)
	{
		auto next = i;
		++next;

		shared_ptr<MapLayer> layer = *i;
		if (m_editor->GetActiveLayer() == layer)
			activeLayerValid = true;

		MapLayerItemWidget* item;
		if (layer->IsEffectLayer())
		{
			item = new MapLayerItemWidget(this, layer->GetName(),
				false, true, false, i != m_map->GetLayers().rbegin(), next != m_map->GetLayers().rend(),
				m_editor->IsLayerVisible(layer),
				[=]() {},
				[=]() {},
				[=]() { RemoveLayer(layer); },
				[=]() { MoveLayerUp(layer); },
				[=]() { MoveLayerDown(layer); },
				[=](bool visible) { m_editor->SetLayerVisibility(layer, visible); }
			);
		}
		else
		{
			item = new MapLayerItemWidget(this, layer->GetName(),
				layer == m_editor->GetActiveLayer(), layer != m_map->GetMainLayer(), true,
				i != m_map->GetLayers().rbegin(), next != m_map->GetLayers().rend(),
				m_editor->IsLayerVisible(layer),
				[=]() { m_editor->SetActiveLayer(layer); },
				[=]() { EditLayer(layer); },
				[=]() { RemoveLayer(layer); },
				[=]() { MoveLayerUp(layer); },
				[=]() { MoveLayerDown(layer); },
				[=](bool visible) { m_editor->SetLayerVisibility(layer, visible); }
			);
		}
		m_entryLayout->addWidget(item);
		m_entries.push_back(item);
	}

	if (!activeLayerValid)
		m_editor->SetActiveLayer(m_map->GetMainLayer());
}


void MapLayerWidget::OnAddLayer()
{
	string name;
	bool ok;
	name = QInputDialog::getText(this, "New Layer", "Name:", QLineEdit::Normal, "", &ok).toStdString();
	if (ok)
	{
		if (name.size() == 0)
		{
			QMessageBox::critical(this, "Error", "New layer name is invalid.");
			return;
		}
		shared_ptr<MapLayer> mainLayer = m_map->GetMainLayer();
		shared_ptr<MapLayer> layer = make_shared<MapLayer>(mainLayer->GetWidth(), mainLayer->GetHeight(),
			mainLayer->GetTileWidth(), mainLayer->GetTileHeight(), mainLayer->GetTileDepth());
		layer->SetName(name);
		size_t index = m_map->GetLayers().size();
		m_map->InsertLayer(index, layer);
		m_mainWindow->UpdateMapContents(m_map);

		shared_ptr<Map> map = m_map;
		MainWindow* mainWindow = m_mainWindow;
		m_mainWindow->AddUndoAction(
			[=]() { // Undo
				map->DeleteLayer(index);
				mainWindow->UpdateMapContents(map);
			},
			[=]() { // Redo
				map->InsertLayer(index, layer);
				mainWindow->UpdateMapContents(map);
			}
		);
	}
}


void MapLayerWidget::OnAddEffectLayer()
{
	map<string, shared_ptr<MapLayer>> layers = m_project->GetEffectLayers();
	if (layers.size() == 0)
	{
		QMessageBox::critical(this, "Error", "There are no effect layers defined in this project.");
		return;
	}

	QStringList items;
	for (auto& i : layers)
		items.append(QString::fromStdString(i.first));

	bool ok;
	QString selection = QInputDialog::getItem(this, "Add Effect Layer", "Select effect layer to add:",
		items, 0, false, &ok);
	if (!ok)
		return;

	auto layerIter = layers.find(selection.toStdString());
	if (layerIter == layers.end())
	{
		QMessageBox::critical(this, "Error", "Internal error while getting selected effect layer.");
		return;
	}

	shared_ptr<MapLayer> mainLayer = m_map->GetMainLayer();
	shared_ptr<MapLayer> layer = layerIter->second;
	size_t index = m_map->GetLayers().size();
	m_map->InsertLayer(index, layer);
	m_mainWindow->UpdateMapContents(m_map);

	shared_ptr<Map> map = m_map;
	MainWindow* mainWindow = m_mainWindow;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			map->DeleteLayer(index);
			mainWindow->UpdateMapContents(map);
		},
		[=]() { // Redo
			map->InsertLayer(index, layer);
			mainWindow->UpdateMapContents(map);
		}
	);
}


void MapLayerWidget::OnFadeOtherLayersChanged(int state)
{
	m_editor->SetFadeOtherLayersEnabled(state == Qt::Checked);
}


void MapLayerWidget::OnAnimationChanged(int state)
{
	m_editor->SetAnimationEnabled(state == Qt::Checked);
}
