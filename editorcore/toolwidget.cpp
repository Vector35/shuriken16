#include <QGuiApplication>
#include "toolwidget.h"
#include "theme.h"

using namespace std;


ToolWidget::ToolWidget(const QString& name, const QString& hint, const function<bool()>& isActive,
	const function<void()>& activate): QLabel(name), m_isActiveFunc(isActive), m_activateFunc(activate)
{
	setToolTip(hint);

	QPalette style(this->palette());
	bool active = isActive();
	if (active)
		style.setColor(QPalette::WindowText, Theme::blue);
	setPalette(style);

	QFont toolFont = QGuiApplication::font();
	toolFont.setPointSize(16);
	setFrameStyle(QFrame::Plain);
	setFrameShape(active ? QFrame::Box : QFrame::NoFrame);
	setLineWidth(active ? 2 : 1);
	setFont(toolFont);
	setFixedSize(QSize(24, 24));
	setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
}


void ToolWidget::mousePressEvent(QMouseEvent*)
{
	m_activateFunc();
}


void ToolWidget::UpdateState()
{
	QPalette style(this->palette());
	bool active = m_isActiveFunc();
	style.setColor(QPalette::WindowText, active ? Theme::blue : Theme::content);
	setPalette(style);
	setFrameShape(active ? QFrame::Box : QFrame::NoFrame);
	setLineWidth(active ? 2 : 1);
}
