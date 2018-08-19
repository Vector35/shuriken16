#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QLabel>
#include "addtilesetdialog.h"

using namespace std;


AddTileSetDialog::AddTileSetDialog(QWidget* parent, shared_ptr<Project> project): QDialog(parent), m_project(project)
{
	QVBoxLayout* layout = new QVBoxLayout();

	layout->addWidget(new QLabel("Name:"));
	m_name = new QLineEdit();
	layout->addWidget(m_name);

	layout->addWidget(new QLabel("Tile Set Type:"));
	m_type = new QComboBox();
	QStringList types;
	types.append("Normal tile set");
	types.append("Simplified double width smart tile set");
	m_type->setEditable(false);
	m_type->addItems(types);
	m_type->setCurrentIndex(0);
	connect(m_type, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AddTileSetDialog::OnTypeChanged);
	layout->addWidget(m_type);

	m_countLabel = new QLabel("Count:");
	layout->addWidget(m_countLabel);
	m_count = new QSpinBox();
	m_count->setMinimum(1);
	m_count->setMaximum(1024);
	m_count->setValue(16);
	m_count->setSingleStep(1);
	layout->addWidget(m_count);

	layout->addWidget(new QLabel("Tile Format:"));
	QHBoxLayout* tileSizeLayout = new QHBoxLayout();
	m_width = new QSpinBox();
	m_width->setMinimum(4);
	m_width->setMaximum(512);
	m_width->setValue(16);
	m_width->setSingleStep(1);
	tileSizeLayout->addWidget(m_width);
	tileSizeLayout->addWidget(new QLabel(" x "));
	m_height = new QSpinBox();
	m_height->setMinimum(4);
	m_height->setMaximum(512);
	m_height->setValue(16);
	m_height->setSingleStep(1);
	tileSizeLayout->addWidget(m_height);
	tileSizeLayout->addWidget(new QLabel(" x "));
	m_depth = new QComboBox();
	m_depth->setEditable(false);
	QStringList depths;
	depths.append("4bpp (16 colors)");
	depths.append("8bpp (256 colors)");
	m_depth->addItems(depths);
	m_depth->setCurrentIndex(0);
	tileSizeLayout->addWidget(m_depth);
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

	connect(okButton, &QPushButton::clicked, this, &AddTileSetDialog::OKButton);
	connect(cancelButton, &QPushButton::clicked, this, &AddTileSetDialog::reject);

	setLayout(layout);
}


void AddTileSetDialog::OnTypeChanged(int type)
{
	m_countLabel->setVisible(type == 0);
	m_count->setVisible(type == 0);
}


void AddTileSetDialog::OKButton()
{
	string name = m_name->text().toStdString();
	size_t count = (size_t)m_count->value();
	size_t width = (size_t)m_width->value();
	size_t height = (size_t)m_height->value();
	size_t depth = 4;
	if (m_depth->currentIndex() == 1)
		depth = 8;

	if (name.size() == 0)
	{
		QMessageBox::critical(this, "Error", "Name must not be blank.");
		return;
	}
	if ((count < 1) || (count > 1024))
	{
		QMessageBox::critical(this, "Error", "Tile set count must be between 1 and 1024.");
		return;
	}
	if ((width < 4) || (width > 512))
	{
		QMessageBox::critical(this, "Error", "Tile width is invalid.");
		return;
	}
	if ((height < 4) || (height > 512))
	{
		QMessageBox::critical(this, "Error", "Tile height is invalid.");
		return;
	}

	m_tileSet = make_shared<TileSet>(width, height, depth, (SmartTileSetType)m_type->currentIndex());
	m_tileSet->SetName(name);
	if (m_tileSet->GetSmartTileSetType() == NormalTileSet)
		m_tileSet->SetTileCount(count);

	if (!m_project->AddTileSet(m_tileSet))
	{
		QMessageBox::critical(this, "Error", "Tile set name is already used. Please choose a different name.");
		return;
	}

	done(Accepted);
}
