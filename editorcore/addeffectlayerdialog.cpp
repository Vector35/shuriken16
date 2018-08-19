#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QLabel>
#include "addeffectlayerdialog.h"

using namespace std;


AddEffectLayerDialog::AddEffectLayerDialog(QWidget* parent, shared_ptr<Project> project):
	QDialog(parent), m_project(project)
{
	QVBoxLayout* layout = new QVBoxLayout();

	layout->addWidget(new QLabel("Name:"));
	m_name = new QLineEdit();
	layout->addWidget(m_name);

	layout->addWidget(new QLabel("Layer Size:"));
	QHBoxLayout* mapSizeLayout = new QHBoxLayout();
	m_width = new QSpinBox();
	m_width->setMinimum(1);
	m_width->setMaximum(65535);
	m_width->setValue(32);
	m_width->setSingleStep(1);
	mapSizeLayout->addWidget(m_width);
	mapSizeLayout->addWidget(new QLabel(" x "));
	m_height = new QSpinBox();
	m_height->setMinimum(1);
	m_height->setMaximum(65535);
	m_height->setValue(32);
	m_height->setSingleStep(1);
	mapSizeLayout->addWidget(m_height);
	mapSizeLayout->addStretch(1);
	layout->addLayout(mapSizeLayout);

	layout->addWidget(new QLabel("Tile Format:"));
	QHBoxLayout* tileSizeLayout = new QHBoxLayout();
	m_tileWidth = new QSpinBox();
	m_tileWidth->setMinimum(4);
	m_tileWidth->setMaximum(512);
	m_tileWidth->setValue(16);
	m_tileWidth->setSingleStep(1);
	tileSizeLayout->addWidget(m_tileWidth);
	tileSizeLayout->addWidget(new QLabel(" x "));
	m_tileHeight = new QSpinBox();
	m_tileHeight->setMinimum(4);
	m_tileHeight->setMaximum(512);
	m_tileHeight->setValue(16);
	m_tileHeight->setSingleStep(1);
	tileSizeLayout->addWidget(m_tileHeight);
	tileSizeLayout->addWidget(new QLabel(" x "));
	m_tileDepth = new QComboBox();
	m_tileDepth->setEditable(false);
	QStringList depths;
	depths.append("4bpp (16 colors)");
	depths.append("8bpp (256 colors)");
	m_tileDepth->addItems(depths);
	m_tileDepth->setCurrentIndex(0);
	tileSizeLayout->addWidget(m_tileDepth);
	layout->addLayout(tileSizeLayout);

	QHBoxLayout* buttonLayout = new QHBoxLayout();
	buttonLayout->addStretch(1);
	QPushButton* okButton = new QPushButton("OK");
	okButton->setDefault(true);
	buttonLayout->addWidget(okButton);
	QPushButton* cancelButton = new QPushButton("Cancel");
	cancelButton->setDefault(false);
	buttonLayout->addWidget(cancelButton);
	layout->addLayout(buttonLayout);

	connect(okButton, &QPushButton::clicked, this, &AddEffectLayerDialog::OKButton);
	connect(cancelButton, &QPushButton::clicked, this, &AddEffectLayerDialog::reject);

	setLayout(layout);
}


void AddEffectLayerDialog::OKButton()
{
	string name = m_name->text().toStdString();
	size_t width = (size_t)m_width->value();
	size_t height = (size_t)m_height->value();
	size_t tileWidth = (size_t)m_tileWidth->value();
	size_t tileHeight = (size_t)m_tileHeight->value();
	size_t tileDepth = 4;
	if (m_tileDepth->currentIndex() == 1)
		tileDepth = 8;

	if (name.size() == 0)
	{
		QMessageBox::critical(this, "Error", "Name must not be blank.");
		return;
	}
	if ((width < 1) || (width > 65535))
	{
		QMessageBox::critical(this, "Error", "Layer width is invalid.");
		return;
	}
	if ((height < 1) || (height > 65535))
	{
		QMessageBox::critical(this, "Error", "Layer height is invalid.");
		return;
	}
	if ((tileWidth < 4) || (tileWidth > 512))
	{
		QMessageBox::critical(this, "Error", "Tile width is invalid.");
		return;
	}
	if ((tileHeight < 4) || (tileHeight > 512))
	{
		QMessageBox::critical(this, "Error", "Tile height is invalid.");
		return;
	}

	m_layer = make_shared<MapLayer>(width, height, tileWidth, tileHeight, tileDepth, true);
	m_layer->SetName(name);

	if (!m_project->AddEffectLayer(m_layer))
	{
		QMessageBox::critical(this, "Error", "Effect layer name is already used. Please choose a different name.");
		return;
	}

	done(Accepted);
}
