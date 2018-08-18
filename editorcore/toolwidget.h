#pragma once

#include <QLabel>
#include <functional>

class ToolWidget: public QLabel
{
	Q_OBJECT

	std::function<bool()> m_isActiveFunc;
	std::function<void()> m_activateFunc;

public:
	ToolWidget(const QString& text, const QString& hint, const std::function<bool()>& isActive,
		const std::function<void()>& activate);

	void UpdateState();

protected:
	virtual void mousePressEvent(QMouseEvent* event) override;
};
