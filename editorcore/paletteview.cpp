#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QGuiApplication>
#include <QPicture>
#include <QPainter>
#include <QColorDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QScrollBar>
#include "paletteview.h"
#include "theme.h"
#include "mainwindow.h"

using namespace std;


PaletteEntryWidget::PaletteEntryWidget(QWidget* parent, const QColor& color, int size, int selection,
	const function<void()>& leftFunc, const function<void()>& rightFunc,
	const function<void()>& doubleClickFunc): QLabel(parent), m_leftFunc(leftFunc),
	m_rightFunc(rightFunc), m_doubleClickFunc(doubleClickFunc)
{
	QPicture picture;
	QPainter painter;
	picture.setBoundingRect(QRect(0, 0, size, size));
	painter.begin(&picture);
	if (color.alpha() == 0)
		painter.setBrush(QBrush(Theme::disabled, Qt::BDiagPattern));
	else
		painter.setBrush(QBrush(color));
	if (selection == 1)
		painter.setPen(QPen(QBrush(Theme::blue), 2));
	else if (selection == 2)
		painter.setPen(QPen(QBrush(Theme::orange), 2));
	else
		painter.setPen(Theme::backgroundDark);
	painter.drawRect(1, 1, size - 2, size - 2);
	painter.end();
	setPicture(picture);
}


void PaletteEntryWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
		m_leftFunc();
	else if (event->button() == Qt::RightButton)
		m_rightFunc();
}


void PaletteEntryWidget::mouseDoubleClickEvent(QMouseEvent*)
{
	m_doubleClickFunc();
}


PaletteEditorWidget::PaletteEditorWidget(QWidget* parent, MainWindow* mainWindow, shared_ptr<Project> project,
	shared_ptr<Palette> palette):
	QWidget(parent), m_mainWindow(mainWindow), m_project(project), m_palette(palette)
{
	QPalette style(this->palette());
	style.setColor(QPalette::Window, Theme::background);
	setPalette(style);

	QVBoxLayout* layout = new QVBoxLayout();

	QHBoxLayout* headerLayout = new QHBoxLayout();
	m_name = new QLabel();
	QFont headerFont = QGuiApplication::font();
	headerFont.setPointSize(headerFont.pointSize() * 4 / 3);
	m_name->setFont(headerFont);
	headerLayout->addWidget(m_name, 1);

	QPushButton* resizeButton = new QPushButton("Resize...");
	connect(resizeButton, &QPushButton::clicked, this, &PaletteEditorWidget::ResizePalette);
	headerLayout->addWidget(resizeButton);
	layout->addLayout(headerLayout);

	m_entryLayout = new QGridLayout();
	m_entryLayout->setSpacing(6);
	m_entryLayout->setColumnStretch(17, 1);
	layout->addLayout(m_entryLayout);

	layout->addStretch(1);
	setLayout(layout);
	UpdateView();
}


void PaletteEditorWidget::EditPaletteEntry(size_t i)
{
	uint16_t existingRawColor = m_palette->GetEntry(i);
	QColor existingColor = QColor::fromRgba(Palette::ToRGB32(existingRawColor) | 0xff000000);
	QColor newColor = QColorDialog::getColor(existingColor, this, "Change Palette Entry");
	if (!newColor.isValid())
		return;

	m_palette->SetEntry(i, Palette::FromRGB32(newColor.rgba()));
	m_mainWindow->UpdatePaletteContents(m_palette);

	shared_ptr<Palette> palette = m_palette;
	MainWindow* mainWindow = m_mainWindow;
	m_mainWindow->AddUndoAction(
		[=]() { // Undo
			palette->SetEntry(i, existingRawColor);
			mainWindow->UpdatePaletteContents(palette);
		},
		[=]() { // Redo
			palette->SetEntry(i, Palette::FromRGB32(newColor.rgba()));
			mainWindow->UpdatePaletteContents(palette);
		}
	);
}


void PaletteEditorWidget::UpdateView()
{
	m_name->setText(QString("Palette - ") + QString::fromStdString(m_palette->GetName()) + QString(" - ") +
		QString::asprintf("%u", (unsigned int)m_palette->GetEntryCount()) +
		QString((m_palette->GetEntryCount() == 1) ? " entry" : " entries"));

	for (auto i : m_entries)
	{
		m_entryLayout->removeWidget(i);
		i->deleteLater();
	}
	m_entries.clear();

	for (size_t i = 0; i < m_palette->GetEntryCount(); i += 16)
	{
		QLabel* num = new QLabel(QString::asprintf("%.2X", (unsigned int)i));
		m_entryLayout->addWidget(num, (int)(i / 16), 0);
		m_entries.push_back(num);

		QLabel* blank = new QLabel();
		m_entryLayout->addWidget(blank, (int)(i / 16), 17);
		m_entries.push_back(blank);
	}

	for (size_t i = 0; i < m_palette->GetEntryCount(); i++)
	{
		PaletteEntryWidget* entry = new PaletteEntryWidget(this,
			QColor::fromRgba(Palette::ToRGB32(m_palette->GetEntry(i)) | 0xff000000), 24, 0,
			[=]() { EditPaletteEntry(i); }, [=]() { EditPaletteEntry(i); }, []() {});
		m_entryLayout->addWidget(entry, (int)(i / 16), (int)(i % 16) + 1);
		m_entries.push_back(entry);
	}
}


void PaletteEditorWidget::ResizePalette()
{
	bool ok;
	size_t existingCount = m_palette->GetEntryCount();
	size_t count = (size_t)QInputDialog::getInt(this, "Resize Palette", "Number of palette entries:",
		(int)existingCount, 1, 256, 16, &ok);
	if (!ok)
		return;

	if ((count < 1) || (count > 256))
	{
		QMessageBox::critical(this, "Error", "Palette size must be between 1 and 256.");
		return;
	}

	if (count < existingCount)
	{
		std::vector<uint16_t> deletedEntries;
		deletedEntries.insert(deletedEntries.end(), m_palette->GetEntries().begin() + count,
			m_palette->GetEntries().end());
		if (deletedEntries.size() != (existingCount - count))
		{
			QMessageBox::critical(this, "Error", "Internal error saving undo action for deletion of "
				"palette entries.");
			return;
		}
		m_palette->SetEntryCount(count);
		m_mainWindow->UpdatePaletteContents(m_palette);

		shared_ptr<Palette> palette = m_palette;
		MainWindow* mainWindow = m_mainWindow;
		m_mainWindow->AddUndoAction(
			[=]() { // Undo
				palette->SetEntryCount(existingCount);
				for (size_t i = 0; i < deletedEntries.size(); i++)
					palette->SetEntry(count + i, deletedEntries[i]);
				mainWindow->UpdatePaletteContents(palette);
			},
			[=]() { // Redo
				palette->SetEntryCount(count);
				mainWindow->UpdatePaletteContents(palette);
			}
		);
	}
	else
	{
		m_palette->SetEntryCount(count);
		m_mainWindow->UpdatePaletteContents(m_palette);

		shared_ptr<Palette> palette = m_palette;
		MainWindow* mainWindow = m_mainWindow;
		m_mainWindow->AddUndoAction(
			[=]() { // Undo
				palette->SetEntryCount(existingCount);
				mainWindow->UpdatePaletteContents(palette);
			},
			[=]() { // Redo
				palette->SetEntryCount(count);
				mainWindow->UpdatePaletteContents(palette);
			}
		);
	}
}


PaletteView::PaletteView(MainWindow* parent, shared_ptr<Project> project,
	shared_ptr<Palette> palette): EditorView(parent)
{
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);
	QScrollArea* scrollArea = new QScrollArea(this);
	m_widget = new PaletteEditorWidget(this, parent, project, palette);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setWidgetResizable(true);
	scrollArea->setAlignment(Qt::AlignTop);
	scrollArea->horizontalScrollBar()->setEnabled(false);
	scrollArea->setWidget(m_widget);
	layout->addWidget(scrollArea, 1);
	setLayout(layout);
}


void PaletteView::UpdateView()
{
	m_widget->UpdateView();
}


shared_ptr<Palette> PaletteView::GetPalette() const
{
	return m_widget->GetPalette();
}
