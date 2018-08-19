#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QLabel>
#include "addspritedialog.h"

using namespace std;


AddSpriteDialog::AddSpriteDialog(QWidget* parent, shared_ptr<Project> project): QDialog(parent), m_project(project)
{
	QVBoxLayout* layout = new QVBoxLayout();

	layout->addWidget(new QLabel("Name:"));
	m_name = new QLineEdit();
	layout->addWidget(m_name);

	layout->addWidget(new QLabel("Sprite Format:"));
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

	connect(okButton, &QPushButton::clicked, this, &AddSpriteDialog::OKButton);
	connect(cancelButton, &QPushButton::clicked, this, &AddSpriteDialog::reject);

	setLayout(layout);
}


void AddSpriteDialog::OKButton()
{
	string name = m_name->text().toStdString();
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
	if ((width < 4) || (width > 512))
	{
		QMessageBox::critical(this, "Error", "Sprite width is invalid.");
		return;
	}
	if ((height < 4) || (height > 512))
	{
		QMessageBox::critical(this, "Error", "Sprite height is invalid.");
		return;
	}

	m_sprite = make_shared<Sprite>(width, height, depth);
	m_sprite->SetName(name);

	if (!m_project->AddSprite(m_sprite))
	{
		QMessageBox::critical(this, "Error", "Sprite name is already used. Please choose a different name.");
		return;
	}

	done(Accepted);
}
