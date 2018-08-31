#pragma once

#include <QWidget>
#include <QLabel>
#include <functional>

class AssociatedTileSetItemLabel: public QLabel
{
	Q_OBJECT

	std::function<void()> m_func;

public:
	AssociatedTileSetItemLabel(const QString& name, const QColor& color, const std::function<void()>& func);

protected:
	virtual void mousePressEvent(QMouseEvent* event) override;
};

class AssociatedTileSetItemWidget: public QWidget
{
	Q_OBJECT

	QColor m_backgroundColor;
	AssociatedTileSetItemLabel* m_removeButton;

public:
	AssociatedTileSetItemWidget(QWidget* parent, const std::string& name,
		const std::function<void()>& removeFunc);

protected:
	virtual void enterEvent(QEvent* event) override;
	virtual void leaveEvent(QEvent* event) override;
};
