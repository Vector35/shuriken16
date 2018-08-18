#pragma once

#include <QWidget>
#include <QLabel>
#include <functional>

class ProjectItemLabel: public QLabel
{
	Q_OBJECT

	std::function<void()> m_func;

public:
	ProjectItemLabel(const QString& name, const QColor& color, const std::function<void()>& func);

protected:
	virtual void mousePressEvent(QMouseEvent* event);
};

class ProjectItemWidget: public QWidget
{
	Q_OBJECT

	ProjectItemLabel* m_renameButton;
	ProjectItemLabel* m_copyButton;
	ProjectItemLabel* m_removeButton;

public:
	ProjectItemWidget(QWidget* parent, const std::string& name, const std::function<void()>& openFunc,
		const std::function<void()>& renameFunc, const std::function<void()>& copyFunc,
		const std::function<void()>& removeFunc);

protected:
	virtual void enterEvent(QEvent* event) override;
	virtual void leaveEvent(QEvent* event) override;
};
