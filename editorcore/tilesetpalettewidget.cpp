#include <QVBoxLayout>
#include <QGuiApplication>
#include <QStyle>
#include <QColorDialog>
#include "tilesetpalettewidget.h"
#include "theme.h"
#include "paletteview.h"
#include "tilesetview.h"
#include "mainwindow.h"

using namespace std;


TileSetPaletteWidget::TileSetPaletteWidget(QWidget* parent, TileSetView* view, MainWindow* mainWindow,
	shared_ptr<Project> project, shared_ptr<TileSet> tileSet, bool activeOnly):
	QWidget(parent), m_mainWindow(mainWindow), m_view(view), m_project(project), m_tileSet(tileSet),
	m_activeOnly(activeOnly)
{
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);
	m_entryLayout = new QGridLayout();
	m_entryLayout->setSpacing(2);
	m_entryLayout->setColumnStretch(17, 1);
	layout->addLayout(m_entryLayout);
	setLayout(layout);

	QFontMetrics metrics(QGuiApplication::font());
	setMinimumSize(metrics.horizontalAdvance("FF ") + 256 + 16 + style()->pixelMetric(QStyle::PM_ScrollBarExtent), 0);
}


void TileSetPaletteWidget::EditPaletteEntry(shared_ptr<Palette> palette, size_t i)
{
	uint16_t existingRawColor = palette->GetEntry(i);
	QColor existingColor = QColor::fromRgba(Palette::ToRGB32(existingRawColor) | 0xff000000);
	QColor newColor = QColorDialog::getColor(existingColor, this, "Change Palette Entry");
	if (!newColor.isValid())
		return;

	palette->SetEntry(i, Palette::FromRGB32(newColor.rgba()));
	m_mainWindow->UpdatePaletteContents(palette);

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


void TileSetPaletteWidget::AddPaletteWidgets(shared_ptr<Palette> palette, int& row)
{
	if (!m_activeOnly)
	{
		QLabel* name = new QLabel(QString::fromStdString(palette->GetName()));
		QPalette style(this->palette());
		style.setColor(QPalette::WindowText, Theme::blue);
		name->setPalette(style);
		m_entryLayout->addWidget(name, row, 0, 1, 18);
		m_entries.push_back(name);
		row++;
	}

	for (size_t i = 0; i < palette->GetEntryCount(); i += 16)
	{
		if (!m_activeOnly)
		{
			QLabel* num = new QLabel(QString::asprintf("%.2X ", (unsigned int)i));
			m_entryLayout->addWidget(num, row + (int)(i / 16), 0);
			m_entries.push_back(num);
		}

		QLabel* blank = new QLabel();
		m_entryLayout->addWidget(blank, row + (int)(i / 16), 17);
		m_entries.push_back(blank);
	}

	for (size_t i = 0; i < palette->GetEntryCount(); i++)
	{
		bool transparent = false;
		if ((i == 0) || ((m_tileSet->GetDepth() == 4) && ((i & 0xf) == 0)))
			transparent = true;
		int selection = 0;
		if ((palette == m_view->GetSelectedPalette()) && (i == m_view->GetSelectedLeftPaletteEntry()))
			selection = 1;
		else if ((palette == m_view->GetSelectedPalette()) && (i == m_view->GetSelectedRightPaletteEntry()))
			selection = 2;
		PaletteEntryWidget* entry = new PaletteEntryWidget(this,
			QColor::fromRgba(Palette::ToRGB32(palette->GetEntry(i)) | (transparent ? 0 : 0xff000000)), 14,
			selection, [=]() { m_view->SetSelectedLeftPaletteEntry(palette, i); },
			[=]() { m_view->SetSelectedRightPaletteEntry(palette, i); },
			[=]() { EditPaletteEntry(palette, i); });
		m_entryLayout->addWidget(entry, row + (int)(i / 16), (int)(i % 16) + 1);
		m_entries.push_back(entry);
	}

	row += (int)((palette->GetEntryCount() + 15) / 16);
}


void TileSetPaletteWidget::UpdateView()
{
	for (auto i : m_entries)
	{
		m_entryLayout->removeWidget(i);
		i->deleteLater();
	}
	m_entries.clear();

	int row = 0;

	if (m_activeOnly)
	{
		if (m_view->GetSelectedPalette())
			AddPaletteWidgets(m_view->GetSelectedPalette(), row);
		return;
	}

	vector<shared_ptr<Palette>> usedPalettes;
	vector<shared_ptr<Palette>> availablePalettes;
	for (auto& i : m_project->GetPalettes())
	{
		if (m_tileSet->UsesPalette(i.second))
			usedPalettes.push_back(i.second);
		else
			availablePalettes.push_back(i.second);
	}

	if (usedPalettes.size() != 0)
	{
		QLabel* available = new QLabel("Active Palettes");
		QFont headerFont = QGuiApplication::font();
		headerFont.setPointSize(headerFont.pointSize() * 5 / 4);
		available->setFont(headerFont);
		m_entryLayout->addWidget(available, row, 0, 1, 18);
		m_entries.push_back(available);
		row++;

		for (auto& i : usedPalettes)
			AddPaletteWidgets(i, row);

		if (availablePalettes.size() != 0)
		{
			m_entryLayout->addWidget(new QLabel(), row, 17);
			row++;
		}
	}

	if (availablePalettes.size() != 0)
	{
		QLabel* available = new QLabel("Available Palettes");
		QFont headerFont = QGuiApplication::font();
		headerFont.setPointSize(headerFont.pointSize() * 5 / 4);
		available->setFont(headerFont);
		m_entryLayout->addWidget(available, row, 0, 1, 18);
		m_entries.push_back(available);
		row++;

		for (auto& i : availablePalettes)
			AddPaletteWidgets(i, row);
	}
}
