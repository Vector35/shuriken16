#pragma once

#include <QWidget>
#include <QLabel>
#include <functional>

class SpriteAnimationHeaderLabel: public QLabel
{
	Q_OBJECT

	std::function<void()> m_func;

public:
	SpriteAnimationHeaderLabel(const QString& name, const QColor& color, const std::function<void()>& func);

protected:
	virtual void mousePressEvent(QMouseEvent* event);
};

class SpriteAnimationHeaderWidget: public QWidget
{
	Q_OBJECT

	SpriteAnimationHeaderLabel* m_renameButton;
	SpriteAnimationHeaderLabel* m_copyButton;
	SpriteAnimationHeaderLabel* m_removeButton;

public:
	SpriteAnimationHeaderWidget(QWidget* parent, const std::string& name,
		const std::function<void()>& renameFunc, const std::function<void()>& copyFunc,
		const std::function<void()>& removeFunc);

protected:
	virtual void enterEvent(QEvent* event) override;
	virtual void leaveEvent(QEvent* event) override;
};
