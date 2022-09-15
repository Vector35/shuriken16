#include <QHBoxLayout>
#include <QLabel>
#include <QGuiApplication>
#include "spriteanimationheaderwidget.h"
#include "theme.h"

using namespace std;


SpriteAnimationHeaderLabel::SpriteAnimationHeaderLabel(const QString& name, const QColor& color,
	const function<void()>& func): QLabel(name), m_func(func)
{
	QPalette style(this->palette());
	style.setColor(QPalette::WindowText, color);
	setPalette(style);
}


void SpriteAnimationHeaderLabel::mousePressEvent(QMouseEvent*)
{
	m_func();
}


SpriteAnimationHeaderWidget::SpriteAnimationHeaderWidget(QWidget* parent, const string& name,
	const function<void()>& renameFunc, const function<void()>& copyFunc, const function<void()>& removeFunc):
	QWidget(parent)
{
	QHBoxLayout* layout = new QHBoxLayout();
	layout->setContentsMargins(4, 4, 16, 4);
	QLabel* label = new QLabel(QString::fromStdString(name));
	QFont headerFont = QGuiApplication::font();
	headerFont.setPointSize(headerFont.pointSize() * 6 / 5);
	label->setFont(headerFont);
	layout->addWidget(label, 1);
	m_renameButton = new SpriteAnimationHeaderLabel("✎", Theme::backgroundWindow, renameFunc);
	m_renameButton->setToolTip("Rename");
	layout->addWidget(m_renameButton);
	m_copyButton = new SpriteAnimationHeaderLabel("⊕", Theme::backgroundWindow, copyFunc);
	m_copyButton->setToolTip("Duplicate");
	layout->addWidget(m_copyButton);
	m_removeButton = new SpriteAnimationHeaderLabel("✖", Theme::backgroundWindow, removeFunc);
	m_removeButton->setToolTip("Delete");
	layout->addWidget(m_removeButton);
	setLayout(layout);
}


void SpriteAnimationHeaderWidget::enterEvent(QEnterEvent*)
{
	QPalette red(this->palette());
	red.setColor(QPalette::WindowText, Theme::red);
	QPalette green(this->palette());
	green.setColor(QPalette::WindowText, Theme::green);
	m_renameButton->setPalette(green);
	m_copyButton->setPalette(green);
	m_removeButton->setPalette(red);
}


void SpriteAnimationHeaderWidget::leaveEvent(QEvent*)
{
	QPalette style(this->palette());
	style.setColor(QPalette::WindowText, Theme::backgroundWindow);
	m_renameButton->setPalette(style);
	m_copyButton->setPalette(style);
	m_removeButton->setPalette(style);
}
