#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QLabel>
#include "addpalettedialog.h"

using namespace std;


AddPaletteDialog::AddPaletteDialog(QWidget* parent, shared_ptr<Project> project): QDialog(parent), m_project(project)
{
	QVBoxLayout* layout = new QVBoxLayout();

	layout->addWidget(new QLabel("Name:"));
	m_name = new QLineEdit();
	layout->addWidget(m_name);

	layout->addWidget(new QLabel("Size:"));
	m_size = new QSpinBox();
	m_size->setMinimum(1);
	m_size->setMaximum(256);
	m_size->setValue(16);
	m_size->setSingleStep(16);
	layout->addWidget(m_size);

	QHBoxLayout* buttonLayout = new QHBoxLayout();
	buttonLayout->addStretch(1);
	QPushButton* okButton = new QPushButton("OK");
	okButton->setDefault(true);
	buttonLayout->addWidget(okButton);
	QPushButton* cancelButton = new QPushButton("Cancel");
	cancelButton->setDefault(false);
	buttonLayout->addWidget(cancelButton);
	layout->addLayout(buttonLayout);

	connect(okButton, &QPushButton::clicked, this, &AddPaletteDialog::OKButton);
	connect(cancelButton, &QPushButton::clicked, this, &AddPaletteDialog::reject);

	setLayout(layout);
}


void AddPaletteDialog::OKButton()
{
	string name = m_name->text().toStdString();
	size_t size = (size_t)m_size->value();

	if (name.size() == 0)
	{
		QMessageBox::critical(this, "Error", "Name must not be blank.");
		return;
	}
	if ((size < 1) || (size > 256))
	{
		QMessageBox::critical(this, "Error", "Palette size must be between 1 and 256.");
		return;
	}

	m_palette = make_shared<Palette>();
	m_palette->SetName(name);
	m_palette->SetEntryCount(size);

	if (!m_project->AddPalette(m_palette))
	{
		QMessageBox::critical(this, "Error", "Palette name is already used. Please choose a different name.");
		return;
	}

	done(Accepted);
}
