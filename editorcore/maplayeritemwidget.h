#pragma once

#include <QWidget>
#include <QLabel>
#include <QCheckBox>
#include <functional>

class MapLayerItemLabel: public QLabel
{
	Q_OBJECT

	std::function<void()> m_func;

public:
	MapLayerItemLabel(const QString& name, const QColor& color, const std::function<void()>& func);

protected:
	virtual void mousePressEvent(QMouseEvent* event) override;
};

class MapLayerItemWidget: public QWidget
{
	Q_OBJECT

	QColor m_backgroundColor;
	QCheckBox* m_visible;
	MapLayerItemLabel* m_renameButton;
	MapLayerItemLabel* m_removeButton;
	MapLayerItemLabel* m_moveUpButton;
	MapLayerItemLabel* m_moveDownButton;
	std::function<void(bool)> m_visibleFunc;

public:
	MapLayerItemWidget(QWidget* parent, const std::string& name, bool selected, bool canDelete,
		bool canEdit, bool canMoveUp, bool canMoveDown, bool visible,
		const std::function<void()>& openFunc, const std::function<void()>& renameFunc,
		const std::function<void()>& removeFunc, const std::function<void()>& moveUpFunc,
		const std::function<void()>& moveDownFunc, const std::function<void(bool)>& visibleFunc);

protected:
	virtual void enterEvent(QEnterEvent* event) override;
	virtual void leaveEvent(QEvent* event) override;

private slots:
	void OnVisibleChanged(int value);
};
