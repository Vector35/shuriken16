#include <QHBoxLayout>
#include <QLabel>
#include "actoritemwidget.h"
#include "theme.h"

using namespace std;


ActorItemLabel::ActorItemLabel(const QString& name, const QColor& color,
	const function<void()>& func): QLabel(name), m_func(func)
{
	QPalette style(this->palette());
	style.setColor(QPalette::WindowText, color);
	setPalette(style);
}


void ActorItemLabel::mousePressEvent(QMouseEvent*)
{
	m_func();
}


ActorItemWidget::ActorItemWidget(QWidget* parent, const string& name, bool selected,
	const function<void()>& openFunc, const function<void()>& removeFunc):
	QWidget(parent)
{
	m_backgroundColor = Theme::backgroundWindow;
	if (selected)
	{
		setAutoFillBackground(true);
		QPalette style(this->palette());
		m_backgroundColor = Theme::selection;
		style.setColor(QPalette::Window, Theme::selection);
		setPalette(style);
	}

	QHBoxLayout* layout = new QHBoxLayout();
	layout->setContentsMargins(4, 1, 16, 1);
	ActorItemLabel* label = new ActorItemLabel(QString::fromStdString(name), Theme::blue, openFunc);
	layout->addWidget(label, 1);

	m_removeButton = new ActorItemLabel("✖", m_backgroundColor, removeFunc);
	m_removeButton->setToolTip("Delete");
	layout->addWidget(m_removeButton);

	setLayout(layout);
}


void ActorItemWidget::enterEvent(QEnterEvent*)
{
	QPalette red(this->palette());
	red.setColor(QPalette::WindowText, Theme::red);
	m_removeButton->setPalette(red);
}


void ActorItemWidget::leaveEvent(QEvent*)
{
	QPalette style(this->palette());
	style.setColor(QPalette::WindowText, m_backgroundColor);
	m_removeButton->setPalette(style);
}
