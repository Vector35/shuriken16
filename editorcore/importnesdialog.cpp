#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include "importnesdialog.h"

using namespace std;


ImportNESDialog::ImportNESDialog(QWidget* parent, shared_ptr<Project> project): QDialog(parent), m_project(project)
{
	QVBoxLayout* layout = new QVBoxLayout();

	layout->addWidget(new QLabel("Name:"));
	m_name = new QLineEdit();
	layout->addWidget(m_name);

	layout->addWidget(new QLabel("Path:"));
	QHBoxLayout* pathLayout = new QHBoxLayout();
	m_path = new QLineEdit();
	pathLayout->addWidget(m_path, 1);
	QPushButton* browseButton = new QPushButton("Browse...");
	pathLayout->addWidget(browseButton);
	layout->addLayout(pathLayout);

	for (auto& i : project->GetPalettes())
		m_palettes.push_back(i.second);
	sort(m_palettes.begin(), m_palettes.end(), [&](const shared_ptr<Palette>& a, const shared_ptr<Palette>& b) {
		return a->GetName() < b->GetName();
	});

	QStringList choices;
	for (auto& i : m_palettes)
		choices.append(QString::fromStdString(i->GetName()));

	layout->addWidget(new QLabel("Palette:"));
	m_palette = new QComboBox();
	m_palette->setEditable(false);
	m_palette->addItems(choices);
	layout->addWidget(m_palette);

	layout->addWidget(new QLabel("Tile Format:"));
	QHBoxLayout* tileSizeLayout = new QHBoxLayout();
	m_width = new QSpinBox();
	m_width->setMinimum(4);
	m_width->setMaximum(512);
	m_width->setValue(16);
	m_width->setSingleStep(1);
	tileSizeLayout->addWidget(m_width);
	tileSizeLayout->addWidget(new QLabel(" x "));
	m_height = new QSpinBox();
	m_height->setMinimum(4);
	m_height->setMaximum(512);
	m_height->setValue(16);
	m_height->setSingleStep(1);
	tileSizeLayout->addWidget(m_height);
	tileSizeLayout->addStretch(1);
	layout->addLayout(tileSizeLayout);
	m_horizontalLayout = new QCheckBox("Horizontal layout of large tiles");
	layout->addWidget(m_horizontalLayout);

	QHBoxLayout* buttonLayout = new QHBoxLayout();
	buttonLayout->addStretch(1);
	QPushButton* okButton = new QPushButton("OK");
	okButton->setDefault(true);
	buttonLayout->addWidget(okButton);
	QPushButton* cancelButton = new QPushButton("Cancel");
	cancelButton->setDefault(false);
	buttonLayout->addWidget(cancelButton);
	layout->addLayout(buttonLayout);

	connect(browseButton, &QPushButton::clicked, this, &ImportNESDialog::BrowseButton);
	connect(okButton, &QPushButton::clicked, this, &ImportNESDialog::OKButton);
	connect(cancelButton, &QPushButton::clicked, this, &ImportNESDialog::reject);

	setLayout(layout);
}


void ImportNESDialog::OKButton()
{
	string name = m_name->text().toStdString();
	size_t width = (size_t)m_width->value();
	size_t height = (size_t)m_height->value();
	size_t depth = 4;
	bool horizontalLayout = m_horizontalLayout->isChecked();

	int paletteIndex = m_palette->currentIndex();
	if ((paletteIndex < 0) || (paletteIndex >= (int)m_palettes.size()))
	{
		QMessageBox::critical(this, "Error", "Palette is invalid.");
		return;
	}
	shared_ptr<Palette> palette = m_palettes[paletteIndex];

	if (name.size() == 0)
	{
		QMessageBox::critical(this, "Error", "Name must not be blank.");
		return;
	}
	if ((width < 4) || (width > 512))
	{
		QMessageBox::critical(this, "Error", "Tile width is invalid.");
		return;
	}
	if ((height < 4) || (height > 512))
	{
		QMessageBox::critical(this, "Error", "Tile height is invalid.");
		return;
	}
	if (((width % 8) != 0) || ((height % 8) != 0))
	{
		QMessageBox::critical(this, "Error", "Tile size must be a multiple of 8.");
		return;
	}

	FILE* fp = fopen(m_path->text().toStdString().c_str(), "rb");
	if (!fp)
	{
		QMessageBox::critical(this, "Error", "Path is invalid.");
		return;
	}

	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	uint8_t* data = new uint8_t[size];
	if (fread(data, size, 1, fp) != 1)
	{
		delete[] data;
		fclose(fp);
		QMessageBox::critical(this, "Error", "Unable to read file.");
		return;
	}
	fclose(fp);

	size_t tileSize = (width / 8) * (height / 8) * 16;
	if ((size % tileSize) != 0)
	{
		delete[] data;
		QMessageBox::critical(this, "Error", "File size is not a multiple of the tile size.");
		return;
	}

	size_t count = (size_t)size / tileSize;
	if (count == 0)
	{
		delete[] data;
		QMessageBox::critical(this, "Error", "File is empty.");
		return;
	}

	m_tileSet = make_shared<TileSet>(width, height, depth, NormalTileSet);
	m_tileSet->SetName(name);
	m_tileSet->SetTileCount(count);

	for (size_t i = 0; i < count; i++)
	{
		shared_ptr<Tile> tile = m_tileSet->GetTile(i);
		tile->SetPalette(palette, 0);
		uint8_t* destData = tile->GetData();
		for (size_t y = 0; y < height; y++)
		{
			for (size_t x = 0; x < width; x++)
			{
				size_t tileX = x / 8;
				size_t tileY = y / 8;
				size_t tileIndex;
				if (horizontalLayout)
					tileIndex = (tileY * (width / 8)) + tileX;
				else
					tileIndex = (tileX * (height / 8)) + tileY;
				uint8_t* tileData = &data[(i * tileSize) + (tileIndex * 16)];
				size_t pixelX = x % 8;
				size_t pixelY = y % 8;
				uint8_t paletteIndex = ((tileData[pixelY] >> (7 - pixelX)) & 1) |
					(((tileData[pixelY + 8] >> (7 - pixelX)) & 1) << 1);
				destData[(y * width / 2) + (x / 2)] |= paletteIndex << ((x & 1) * 4);
			}
		}
	}

	delete[] data;

	if (!m_project->AddTileSet(m_tileSet))
	{
		QMessageBox::critical(this, "Error", "Tile set name is already used. Please choose a different name.");
		return;
	}

	done(Accepted);
}


void ImportNESDialog::BrowseButton()
{
	QString path = QFileDialog::getOpenFileName(this, "Import File", "", "CHR files (*.chr)");
	if (path.isNull())
		return;
	m_path->setText(path);
}


QSize ImportNESDialog::sizeHint() const
{
	return QSize(300, 100);
}
