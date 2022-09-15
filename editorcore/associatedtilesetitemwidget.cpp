#include <QHBoxLayout>
#include <QLabel>
#include "associatedtilesetitemwidget.h"
#include "theme.h"

using namespace std;


AssociatedTileSetItemLabel::AssociatedTileSetItemLabel(const QString& name, const QColor& color,
	const function<void()>& func): QLabel(name), m_func(func)
{
	QPalette style(this->palette());
	style.setColor(QPalette::WindowText, color);
	setPalette(style);
}


void AssociatedTileSetItemLabel::mousePressEvent(QMouseEvent*)
{
	m_func();
}


AssociatedTileSetItemWidget::AssociatedTileSetItemWidget(QWidget* parent, const string& name,
	const function<void()>& removeFunc): QWidget(parent)
{
	m_backgroundColor = Theme::backgroundWindow;

	QHBoxLayout* layout = new QHBoxLayout();
	layout->setContentsMargins(4, 1, 16, 1);
	AssociatedTileSetItemLabel* label = new AssociatedTileSetItemLabel(QString::fromStdString(name), Theme::blue, [=]() {});
	layout->addWidget(label, 1);

	m_removeButton = new AssociatedTileSetItemLabel("✖", m_backgroundColor, removeFunc);
	m_removeButton->setToolTip("Delete");
	layout->addWidget(m_removeButton);

	setLayout(layout);
}


void AssociatedTileSetItemWidget::enterEvent(QEnterEvent*)
{
	QPalette red(this->palette());
	red.setColor(QPalette::WindowText, Theme::red);
	m_removeButton->setPalette(red);
}


void AssociatedTileSetItemWidget::leaveEvent(QEvent*)
{
	QPalette style(this->palette());
	style.setColor(QPalette::WindowText, m_backgroundColor);
	m_removeButton->setPalette(style);
}
