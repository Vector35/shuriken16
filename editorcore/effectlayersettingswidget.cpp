#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <math.h>
#include "effectlayersettingswidget.h"
#include "mainwindow.h"

using namespace std;


EffectLayerSettingsWidget::EffectLayerSettingsWidget(QWidget* parent, MapEditorWidget* editor,
	MainWindow* mainWindow, shared_ptr<Project> project, shared_ptr<MapLayer> layer):
	QWidget(parent), m_editor(editor), m_mainWindow(mainWindow), m_project(project), m_layer(layer)
{
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 8, 16);

	QHBoxLayout* blendModeLayout = new QHBoxLayout();
	blendModeLayout->addWidget(new QLabel("Blend mode:"));

	m_blendMode = new QComboBox();
	QStringList modes;
	modes.append("Normal");
	modes.append("Add");
	modes.append("Subtract");
	modes.append("Multiply");
	m_blendMode->setEditable(false);
	m_blendMode->addItems(modes);
	m_blendMode->setCurrentIndex((int)m_layer->GetBlendMode());
	blendModeLayout->addWidget(m_blendMode, 1);
	layout->addLayout(blendModeLayout);

	QHBoxLayout* alphaLayout = new QHBoxLayout();
	alphaLayout->addWidget(new QLabel("Opacity:"));

	m_alpha = new QSlider(Qt::Horizontal);
	m_alpha->setMinimum(0);
	m_alpha->setMaximum(15);
	m_alpha->setSingleStep(1);
	m_alpha->setPageStep(4);
	m_alpha->setTracking(true);
	m_alpha->setValue((int)(15 - m_layer->GetAlpha()));
	alphaLayout->addWidget(m_alpha, 1);
	layout->addLayout(alphaLayout);

	QHBoxLayout* parallaxLayout = new QHBoxLayout();
	parallaxLayout->addWidget(new QLabel("Parallax Factor X:"));
	m_parallaxFactorX = new QLineEdit();
	m_parallaxFactorX->setValidator(new QDoubleValidator());
	m_parallaxFactorX->setMaximumWidth(65);
	parallaxLayout->addWidget(m_parallaxFactorX);
	parallaxLayout->addWidget(new QLabel("Y:"));
	m_parallaxFactorY = new QLineEdit();
	m_parallaxFactorY->setValidator(new QDoubleValidator());
	m_parallaxFactorY->setMaximumWidth(65);
	parallaxLayout->addWidget(m_parallaxFactorY);
	parallaxLayout->addStretch(1);
	layout->addLayout(parallaxLayout);

	QHBoxLayout* autoScrollLayout = new QHBoxLayout();
	autoScrollLayout->addWidget(new QLabel("Auto Scroll X:"));
	m_autoScrollX = new QLineEdit();
	m_autoScrollX->setValidator(new QDoubleValidator());
	m_autoScrollX->setMaximumWidth(65);
	autoScrollLayout->addWidget(m_autoScrollX);
	autoScrollLayout->addWidget(new QLabel("Y:"));
	m_autoScrollY = new QLineEdit();
	m_autoScrollY->setValidator(new QDoubleValidator());
	m_autoScrollY->setMaximumWidth(65);
	autoScrollLayout->addWidget(m_autoScrollY);
	autoScrollLayout->addStretch(1);
	layout->addLayout(autoScrollLayout);

	setLayout(layout);

	connect(m_blendMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&EffectLayerSettingsWidget::OnBlendModeChanged);
	connect(m_alpha, &QSlider::valueChanged, this, &EffectLayerSettingsWidget::OnAlphaChanged);
	connect(m_parallaxFactorX, &QLineEdit::editingFinished, this, &EffectLayerSettingsWidget::OnParallaxFactorXChanged);
	connect(m_parallaxFactorY, &QLineEdit::editingFinished, this, &EffectLayerSettingsWidget::OnParallaxFactorYChanged);
	connect(m_autoScrollX, &QLineEdit::editingFinished, this, &EffectLayerSettingsWidget::OnAutoScrollXChanged);
	connect(m_autoScrollY, &QLineEdit::editingFinished, this, &EffectLayerSettingsWidget::OnAutoScrollYChanged);
}


void EffectLayerSettingsWidget::UpdateView()
{
	m_blendMode->setCurrentIndex((int)m_layer->GetBlendMode());
	m_alpha->setValue((int)(15 - m_layer->GetAlpha()));
	m_parallaxFactorX->setText(QString::asprintf("%.2g", (float)m_layer->GetParallaxFactorX() / (float)0x100));
	m_parallaxFactorY->setText(QString::asprintf("%.2g", (float)m_layer->GetParallaxFactorY() / (float)0x100));
	m_autoScrollX->setText(QString::asprintf("%.2g", (float)m_layer->GetAutoScrollX() / (float)0x100));
	m_autoScrollY->setText(QString::asprintf("%.2g", (float)m_layer->GetAutoScrollY() / (float)0x100));
}


void EffectLayerSettingsWidget::OnBlendModeChanged(int value)
{
	BlendMode mode = (BlendMode)value;
	BlendMode oldMode = m_layer->GetBlendMode();
	if (mode == oldMode)
		return;

	m_layer->SetBlendMode(mode);
	m_mainWindow->UpdateEffectLayerContents(m_layer);

	shared_ptr<MapLayer> layer = m_layer;
	MainWindow* mainWindow = m_mainWindow;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			layer->SetBlendMode(oldMode);
			mainWindow->UpdateEffectLayerContents(layer);
		},
		[=]() { // Redo
			layer->SetBlendMode(mode);
			mainWindow->UpdateEffectLayerContents(layer);
		}
	);
}


void EffectLayerSettingsWidget::OnAlphaChanged(int value)
{
	uint8_t alpha = (uint8_t)(15 - value);
	uint8_t oldAlpha = m_layer->GetAlpha();
	if (alpha == oldAlpha)
		return;

	m_layer->SetAlpha(alpha);
	m_mainWindow->UpdateEffectLayerContents(m_layer);

	shared_ptr<MapLayer> layer = m_layer;
	MainWindow* mainWindow = m_mainWindow;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			layer->SetAlpha(oldAlpha);
			mainWindow->UpdateEffectLayerContents(layer);
		},
		[=]() { // Redo
			layer->SetAlpha(alpha);
			mainWindow->UpdateEffectLayerContents(layer);
		}
	);
}


void EffectLayerSettingsWidget::OnParallaxFactorXChanged()
{
	int16_t value = (int16_t)round(m_parallaxFactorX->text().toDouble() * (double)0x100);
	int16_t oldValue = m_layer->GetParallaxFactorX();
	if (value == oldValue)
		return;

	m_layer->SetParallaxFactorX(value);
	m_mainWindow->UpdateEffectLayerContents(m_layer);

	shared_ptr<MapLayer> layer = m_layer;
	MainWindow* mainWindow = m_mainWindow;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			layer->SetParallaxFactorX(oldValue);
			mainWindow->UpdateEffectLayerContents(layer);
		},
		[=]() { // Redo
			layer->SetParallaxFactorX(value);
			mainWindow->UpdateEffectLayerContents(layer);
		}
	);
}


void EffectLayerSettingsWidget::OnParallaxFactorYChanged()
{
	int16_t value = (int16_t)round(m_parallaxFactorY->text().toDouble() * (double)0x100);
	int16_t oldValue = m_layer->GetParallaxFactorY();
	if (value == oldValue)
		return;

	m_layer->SetParallaxFactorY(value);
	m_mainWindow->UpdateEffectLayerContents(m_layer);

	shared_ptr<MapLayer> layer = m_layer;
	MainWindow* mainWindow = m_mainWindow;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			layer->SetParallaxFactorY(oldValue);
			mainWindow->UpdateEffectLayerContents(layer);
		},
		[=]() { // Redo
			layer->SetParallaxFactorY(value);
			mainWindow->UpdateEffectLayerContents(layer);
		}
	);
}


void EffectLayerSettingsWidget::OnAutoScrollXChanged()
{
	int16_t value = (int16_t)round(m_autoScrollX->text().toDouble() * (double)0x100);
	int16_t oldValue = m_layer->GetAutoScrollX();
	if (value == oldValue)
		return;

	m_layer->SetAutoScrollX(value);
	m_mainWindow->UpdateEffectLayerContents(m_layer);

	shared_ptr<MapLayer> layer = m_layer;
	MainWindow* mainWindow = m_mainWindow;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			layer->SetAutoScrollX(oldValue);
			mainWindow->UpdateEffectLayerContents(layer);
		},
		[=]() { // Redo
			layer->SetAutoScrollX(value);
			mainWindow->UpdateEffectLayerContents(layer);
		}
	);
}


void EffectLayerSettingsWidget::OnAutoScrollYChanged()
{
	int16_t value = (int16_t)round(m_autoScrollY->text().toDouble() * (double)0x100);
	int16_t oldValue = m_layer->GetAutoScrollY();
	if (value == oldValue)
		return;

	m_layer->SetAutoScrollY(value);
	m_mainWindow->UpdateEffectLayerContents(m_layer);

	shared_ptr<MapLayer> layer = m_layer;
	MainWindow* mainWindow = m_mainWindow;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			layer->SetAutoScrollY(oldValue);
			mainWindow->UpdateEffectLayerContents(layer);
		},
		[=]() { // Redo
			layer->SetAutoScrollY(value);
			mainWindow->UpdateEffectLayerContents(layer);
		}
	);
}
