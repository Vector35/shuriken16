#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QLabel>
#include "addactortypefielddialog.h"

using namespace std;


AddActorTypeFieldDialog::AddActorTypeFieldDialog(QWidget* parent):
	QDialog(parent)
{
	QVBoxLayout* layout = new QVBoxLayout();

	layout->addWidget(new QLabel("Name:"));
	m_name = new QLineEdit();
	layout->addWidget(m_name);

	m_availableTypes = ActorFieldType::GetTypes();

	layout->addWidget(new QLabel("Type:"));

	m_type = new QComboBox();
	m_type->setEditable(false);
	QStringList types;
	for (auto& i : m_availableTypes)
		types.append(QString::fromStdString(i->GetDescription()));
	m_type->addItems(types);
	m_type->setCurrentIndex(0);
	layout->addWidget(m_type);

	QHBoxLayout* buttonLayout = new QHBoxLayout();
	buttonLayout->addStretch(1);
	QPushButton* okButton = new QPushButton("OK");
	okButton->setDefault(true);
	buttonLayout->addWidget(okButton);
	QPushButton* cancelButton = new QPushButton("Cancel");
	cancelButton->setDefault(false);
	buttonLayout->addWidget(cancelButton);
	layout->addLayout(buttonLayout);

	connect(okButton, &QPushButton::clicked, this, &AddActorTypeFieldDialog::OKButton);
	connect(cancelButton, &QPushButton::clicked, this, &AddActorTypeFieldDialog::reject);

	setLayout(layout);
}


string AddActorTypeFieldDialog::GetName()
{
	return m_name->text().toStdString();
}


ActorFieldType* AddActorTypeFieldDialog::GetType()
{
	return m_availableTypes[m_type->currentIndex()];
}


void AddActorTypeFieldDialog::OKButton()
{
	string name = m_name->text().toStdString();
	if (name.size() == 0)
	{
		QMessageBox::critical(this, "Error", "Name must not be blank.");
		return;
	}

	done(Accepted);
}
