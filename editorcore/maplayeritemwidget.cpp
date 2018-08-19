#include <QHBoxLayout>
#include <QLabel>
#include "maplayeritemwidget.h"
#include "theme.h"

using namespace std;


MapLayerItemLabel::MapLayerItemLabel(const QString& name, const QColor& color,
	const function<void()>& func): QLabel(name), m_func(func)
{
	QPalette style(this->palette());
	style.setColor(QPalette::WindowText, color);
	setPalette(style);
}


void MapLayerItemLabel::mousePressEvent(QMouseEvent*)
{
	m_func();
}


MapLayerItemWidget::MapLayerItemWidget(QWidget* parent, const string& name, bool selected, bool canDelete,
	bool canEdit, bool canMoveUp, bool canMoveDown, bool visible,
	const function<void()>& openFunc, const function<void()>& renameFunc,
	const function<void()>& removeFunc, const function<void()>& moveUpFunc,
	const function<void()>& moveDownFunc, const function<void(bool)>& visibleFunc):
	QWidget(parent), m_visibleFunc(visibleFunc)
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
	layout->setContentsMargins(4, 2, 16, 2);
	m_visible = new QCheckBox();
	m_visible->setCheckState(visible ? Qt::Checked : Qt::Unchecked);
	connect(m_visible, &QCheckBox::stateChanged, this, &MapLayerItemWidget::OnVisibleChanged);
	layout->addWidget(m_visible);
	MapLayerItemLabel* label = new MapLayerItemLabel(QString::fromStdString(name), Theme::blue, openFunc);
	layout->addWidget(label, 1);

	if (canEdit)
	{
		m_renameButton = new MapLayerItemLabel("✎", m_backgroundColor, renameFunc);
		m_renameButton->setToolTip("Rename");
		layout->addWidget(m_renameButton);
	}
	else
	{
		m_renameButton = nullptr;
	}

	if (canDelete)
	{
		m_removeButton = new MapLayerItemLabel("✖", m_backgroundColor, removeFunc);
		m_removeButton->setToolTip("Delete");
		layout->addWidget(m_removeButton);
	}
	else
	{
		m_removeButton = nullptr;
	}

	if (canMoveUp)
	{
		m_moveUpButton = new MapLayerItemLabel("⬆", m_backgroundColor, moveUpFunc);
		m_moveUpButton->setToolTip("Move Layer Up");
		layout->addWidget(m_moveUpButton);
	}
	else
	{
		m_moveUpButton = nullptr;
	}

	if (canMoveDown)
	{
		m_moveDownButton = new MapLayerItemLabel("⬇", m_backgroundColor, moveDownFunc);
		m_moveDownButton->setToolTip("Move Layer Down");
		layout->addWidget(m_moveDownButton);
	}
	else
	{
		m_moveDownButton = nullptr;
	}

	setLayout(layout);
}


void MapLayerItemWidget::enterEvent(QEvent*)
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
	if (m_renameButton)
		m_renameButton->setPalette(green);
	if (m_removeButton)
		m_removeButton->setPalette(red);
}


void MapLayerItemWidget::leaveEvent(QEvent*)
{
	QPalette style(this->palette());
	style.setColor(QPalette::WindowText, m_backgroundColor);
	if (m_moveUpButton)
		m_moveUpButton->setPalette(style);
	if (m_moveDownButton)
		m_moveDownButton->setPalette(style);
	if (m_renameButton)
		m_renameButton->setPalette(style);
	if (m_removeButton)
		m_removeButton->setPalette(style);
}


void MapLayerItemWidget::OnVisibleChanged(int value)
{
	m_visibleFunc(value == Qt::Checked);
}
