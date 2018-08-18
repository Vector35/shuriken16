#pragma once

#include <QWidget>
#include <QLabel>
#include <functional>

class AnimationFrameLabel: public QLabel
{
	Q_OBJECT

	std::function<void()> m_func;

public:
	AnimationFrameLabel(const QString& name, const QColor& color, const std::function<void()>& func);

protected:
	virtual void mousePressEvent(QMouseEvent* event) override;
};

class AnimationFrameWidget: public QWidget
{
	Q_OBJECT

	QColor m_backgroundColor;
	AnimationFrameLabel* m_editButton;
	AnimationFrameLabel* m_duplicateButton;
	AnimationFrameLabel* m_removeButton;
	AnimationFrameLabel* m_moveUpButton;
	AnimationFrameLabel* m_moveDownButton;

public:
	AnimationFrameWidget(QWidget* parent, const std::string& name, bool selected,
		bool canMoveUp, bool canMoveDown,
		const std::function<void()>& openFunc, const std::function<void()>& editFunc,
		const std::function<void()>& duplicateFunc, const std::function<void()>& removeFunc,
		const std::function<void()>& moveUpFunc, const std::function<void()>& moveDownFunc);

protected:
	virtual void enterEvent(QEvent* event) override;
	virtual void leaveEvent(QEvent* event) override;
};
