#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QLabel>
#include "layersettingsdialog.h"

using namespace std;


LayerSettingsDialog::LayerSettingsDialog(QWidget* parent, shared_ptr<MapLayer> layer):
	QDialog(parent), m_layer(layer)
{
	QVBoxLayout* layout = new QVBoxLayout();

	layout->addWidget(new QLabel("Name:"));
	m_name = new QLineEdit();
	m_name->setText(QString::fromStdString(m_layer->GetName()));
	layout->addWidget(m_name);

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

	QHBoxLayout* buttonLayout = new QHBoxLayout();
	buttonLayout->addStretch(1);
	QPushButton* okButton = new QPushButton("OK");
	okButton->setDefault(true);
	buttonLayout->addWidget(okButton);
	QPushButton* cancelButton = new QPushButton("Cancel");
	cancelButton->setDefault(false);
	buttonLayout->addWidget(cancelButton);
	layout->addLayout(buttonLayout);

	connect(okButton, &QPushButton::clicked, this, &LayerSettingsDialog::accept);
	connect(cancelButton, &QPushButton::clicked, this, &LayerSettingsDialog::reject);

	setLayout(layout);
}


string LayerSettingsDialog::GetName()
{
	return m_name->text().toStdString();
}


BlendMode LayerSettingsDialog::GetBlendMode()
{
	return (BlendMode)m_blendMode->currentIndex();
}


uint8_t LayerSettingsDialog::GetAlpha()
{
	return (uint8_t)(15 - m_alpha->value());
}
