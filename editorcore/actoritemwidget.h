#pragma once

#include <QWidget>
#include <QLabel>
#include <functional>

class ActorItemLabel: public QLabel
{
	Q_OBJECT

	std::function<void()> m_func;

public:
	ActorItemLabel(const QString& name, const QColor& color, const std::function<void()>& func);

protected:
	virtual void mousePressEvent(QMouseEvent* event) override;
};

class ActorItemWidget: public QWidget
{
	Q_OBJECT

	QColor m_backgroundColor;
	ActorItemLabel* m_removeButton;

public:
	ActorItemWidget(QWidget* parent, const std::string& name, bool selected,
		const std::function<void()>& openFunc, const std::function<void()>& removeFunc);

protected:
	virtual void enterEvent(QEvent* event) override;
	virtual void leaveEvent(QEvent* event) override;
};
