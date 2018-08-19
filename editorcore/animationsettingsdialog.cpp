#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QLabel>
#include "animationsettingsdialog.h"

using namespace std;


AnimationSettingsDialog::AnimationSettingsDialog(QWidget* parent, const shared_ptr<SpriteAnimation>& anim):
	QDialog(parent)
{
	QVBoxLayout* layout = new QVBoxLayout();

	layout->addWidget(new QLabel("Name:"));
	m_name = new QLineEdit();
	if (anim)
		m_name->setText(QString::fromStdString(anim->GetName()));
	layout->addWidget(m_name);

	m_looping = new QCheckBox("Looping animation");
	if (anim)
		m_looping->setChecked(anim->IsLooping());
	else
		m_looping->setChecked(true);
	layout->addWidget(m_looping);

	QHBoxLayout* buttonLayout = new QHBoxLayout();
	buttonLayout->addStretch(1);
	QPushButton* okButton = new QPushButton("OK");
	okButton->setDefault(true);
	buttonLayout->addWidget(okButton);
	QPushButton* cancelButton = new QPushButton("Cancel");
	cancelButton->setDefault(false);
	buttonLayout->addWidget(cancelButton);
	layout->addLayout(buttonLayout);

	connect(okButton, &QPushButton::clicked, this, &AnimationSettingsDialog::accept);
	connect(cancelButton, &QPushButton::clicked, this, &AnimationSettingsDialog::reject);

	setLayout(layout);
}


string AnimationSettingsDialog::GetName()
{
	return m_name->text().toStdString();
}


bool AnimationSettingsDialog::IsLooping()
{
	return m_looping->isChecked();
}
