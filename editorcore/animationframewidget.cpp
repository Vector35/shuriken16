#include <QHBoxLayout>
#include <QLabel>
#include "animationframewidget.h"
#include "theme.h"

using namespace std;


AnimationFrameLabel::AnimationFrameLabel(const QString& name, const QColor& color,
	const function<void()>& func): QLabel(name), m_func(func)
{
	QPalette style(this->palette());
	style.setColor(QPalette::WindowText, color);
	setPalette(style);
}


void AnimationFrameLabel::mousePressEvent(QMouseEvent*)
{
	m_func();
}


AnimationFrameWidget::AnimationFrameWidget(QWidget* parent, const string& name, bool selected,
	bool canMoveUp, bool canMoveDown,
	const function<void()>& openFunc, const function<void()>& editFunc,
	const function<void()>& duplicateFunc, const function<void()>& removeFunc,
	const function<void()>& moveUpFunc, const function<void()>& moveDownFunc):
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
	layout->setContentsMargins(4, 4, 16, 4);
	AnimationFrameLabel* label = new AnimationFrameLabel(QString::fromStdString(name), Theme::blue, openFunc);
	layout->addWidget(label, 1);

	m_editButton = new AnimationFrameLabel("✎", m_backgroundColor, editFunc);
	m_editButton->setToolTip("Edit Frame");
	layout->addWidget(m_editButton);

	m_duplicateButton = new AnimationFrameLabel("⊕", m_backgroundColor, duplicateFunc);
	m_duplicateButton->setToolTip("Duplicate");
	layout->addWidget(m_duplicateButton);

	m_removeButton = new AnimationFrameLabel("✖", m_backgroundColor, removeFunc);
	m_removeButton->setToolTip("Delete");
	layout->addWidget(m_removeButton);

	if (canMoveUp)
	{
		m_moveUpButton = new AnimationFrameLabel("⬆", m_backgroundColor, moveUpFunc);
		m_moveUpButton->setToolTip("Move Frame Up");
		layout->addWidget(m_moveUpButton);
	}
	else
	{
		m_moveUpButton = nullptr;
	}

	if (canMoveDown)
	{
		m_moveDownButton = new AnimationFrameLabel("⬇", m_backgroundColor, moveDownFunc);
		m_moveDownButton->setToolTip("Move Frame Down");
		layout->addWidget(m_moveDownButton);
	}
	else
	{
		m_moveDownButton = nullptr;
	}

	setLayout(layout);
}


void AnimationFrameWidget::enterEvent(QEvent*)
{
	QPalette red(this->palette());
	red.setColor(QPalette::WindowText, Theme::red);
	QPalette green(this->palette());
	green.setColor(QPalette::WindowText, Theme::green);
	QPalette blue(this->palette());
	blue.setColor(QPalette::WindowText, Theme::blue);
	if (m_moveUpButton)
		m_moveUpButton->setPalette(blue);
	if (m_moveDownButton)
		m_moveDownButton->setPalette(blue);
	m_editButton->setPalette(green);
	m_duplicateButton->setPalette(green);
	m_removeButton->setPalette(red);
}


void AnimationFrameWidget::leaveEvent(QEvent*)
{
	QPalette style(this->palette());
	style.setColor(QPalette::WindowText, m_backgroundColor);
	if (m_moveUpButton)
		m_moveUpButton->setPalette(style);
	if (m_moveDownButton)
		m_moveDownButton->setPalette(style);
	m_editButton->setPalette(style);
	m_duplicateButton->setPalette(style);
	m_removeButton->setPalette(style);
}
