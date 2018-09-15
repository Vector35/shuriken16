#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QMessageBox>
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif
#include "theme.h"
#include "editor.h"
#include "stringfieldtype.h"
#include "intfieldtype.h"
#include "floatfieldtype.h"
#include "choicefieldtype.h"
#include "mapfieldtype.h"
#include "jsonfieldtype.h"


QString GetRootPath()
{
#ifdef WIN32
	// Find the directory that contains the executable
	char exePath[MAX_PATH];
	if (!GetModuleFileNameA(NULL, exePath, MAX_PATH))
		return "";

	char drive[_MAX_DRIVE], dir[_MAX_DIR];
	if (_splitpath_s(exePath, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0))
		return "";

	// Construct path to plugins
	char dirPath[_MAX_DRIVE + _MAX_DIR + 32];
	_snprintf(dirPath, sizeof(dirPath), "%s%s", drive, dir);

	return QString::fromLocal8Bit(dirPath);
#else
	char exePath[1024];
	QString relPath;
#ifdef __linux__
	// Find the directory that contains the executable
	ssize_t pathLen = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
	if (pathLen < 0)
		return "";
	exePath[pathLen] = 0;
	relPath = ".";
#elif defined(__APPLE__)
	uint32_t size = sizeof(exePath);
	if (_NSGetExecutablePath(exePath, &size))
		return "";
	relPath = "../../..";
#else
	#warning Relative asset path loading not implemented for this platform
	return "";
#endif

	char* filePath = strrchr(exePath, '/');
	if (!filePath)
		return "";
	*filePath = 0;

	return QDir::cleanPath(QDir(QString::fromLocal8Bit(exePath)).absoluteFilePath(relPath));
#endif
}


void InitEditor()
{
	// Set up the theme
	QPalette palette;
	palette.setColor(QPalette::Window, Theme::backgroundWindow);
	palette.setColor(QPalette::WindowText, Theme::content);
	palette.setColor(QPalette::Base, Theme::backgroundDark);
	palette.setColor(QPalette::AlternateBase, Theme::background);
	palette.setColor(QPalette::ToolTipBase, Theme::backgroundHighlight);
	palette.setColor(QPalette::ToolTipText, Theme::content);
	palette.setColor(QPalette::Text, Theme::content);
	palette.setColor(QPalette::Button, Theme::backgroundHighlight);
	palette.setColor(QPalette::ButtonText, Theme::content);
	palette.setColor(QPalette::BrightText, Theme::yellow);
	palette.setColor(QPalette::Link, Theme::blue);
	palette.setColor(QPalette::Highlight, Theme::blue);
	palette.setColor(QPalette::HighlightedText, Theme::backgroundDark);
	palette.setColor(QPalette::Light, Theme::light);
	QApplication::setPalette(palette);
	QApplication::setStyle(QStyleFactory::create("Fusion"));

	// Ensure high DPI displays work properly
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	// Register built-in field types for actors
	StringFieldType::Register();
	IntFieldType::Register();
	FloatFieldType::Register();
	ChoiceFieldType::Register();
	MapFieldType::Register();
	JsonFieldType::Register();
}


void StartEditor(const QString& title, const QString& basePath, const QString& mainAssetPath)
{
	QString baseAbsolutePath = QDir::cleanPath(QDir(GetRootPath()).absoluteFilePath(basePath));
	QString mainAssetAbsolutePath = QDir::cleanPath(QDir(GetRootPath()).absoluteFilePath(mainAssetPath));
	MainWindow* wnd = new MainWindow(title, baseAbsolutePath, mainAssetAbsolutePath);
	wnd->show();
}
