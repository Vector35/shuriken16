#include <QHBoxLayout>
#include <QLabel>
#include "projectitemwidget.h"
#include "theme.h"

using namespace std;


ProjectItemLabel::ProjectItemLabel(const QString& name, const QColor& color,
	const function<void()>& func): QLabel(name), m_func(func)
{
	QPalette style(this->palette());
	style.setColor(QPalette::WindowText, color);
	setPalette(style);
}


void ProjectItemLabel::mousePressEvent(QMouseEvent*)
{
	m_func();
}


ProjectItemWidget::ProjectItemWidget(QWidget* parent, const string& name, const function<void()>& openFunc,
	const function<void()>& renameFunc, const function<void()>& copyFunc, const function<void()>& removeFunc):
	QWidget(parent)
{
	QHBoxLayout* layout = new QHBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);
	ProjectItemLabel* label = new ProjectItemLabel(QString::fromStdString(name), Theme::blue, openFunc);
	layout->addWidget(label, 1);
	m_renameButton = new ProjectItemLabel("✎", Theme::backgroundDark, renameFunc);
	m_renameButton->setToolTip("Rename");
	layout->addWidget(m_renameButton);
	m_copyButton = new ProjectItemLabel("⊕", Theme::backgroundDark, copyFunc);
	m_copyButton->setToolTip("Duplicate");
	layout->addWidget(m_copyButton);
	m_removeButton = new ProjectItemLabel("✖", Theme::backgroundDark, removeFunc);
	m_removeButton->setToolTip("Delete");
	layout->addWidget(m_removeButton);
	setLayout(layout);
}


void ProjectItemWidget::enterEvent(QEvent*)
{
	QPalette red(this->palette());
	red.setColor(QPalette::WindowText, Theme::red);
	QPalette green(this->palette());
	green.setColor(QPalette::WindowText, Theme::green);
	m_renameButton->setPalette(green);
	m_copyButton->setPalette(green);
	m_removeButton->setPalette(red);
}


void ProjectItemWidget::leaveEvent(QEvent*)
{
	QPalette style(this->palette());
	style.setColor(QPalette::WindowText, Theme::backgroundDark);
	m_renameButton->setPalette(style);
	m_copyButton->setPalette(style);
	m_removeButton->setPalette(style);
}
