#pragma once

#include <QWidget>

class EditorView: public QWidget
{
	Q_OBJECT

public:
	EditorView(QWidget* parent);

	virtual void Cut() {}
	virtual void Copy() {}
	virtual void Paste() {}
	virtual void SelectAll() {}

	virtual void ExportPNG() {}
};
