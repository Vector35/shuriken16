QT += core gui widgets

TARGET = editorcore
TEMPLATE = lib 
CONFIG += staticlib

DEFINES += QT_DEPRECATED_WARNINGS
INCLUDEPATH += $$PWD/editor

SOURCES += \
	main.cpp \
	mainwindow.cpp \
	theme.cpp \
	palette.cpp \
	tile.cpp \
	tileset.cpp \
	map.cpp \
	maplayer.cpp \
	project.cpp \
	projectview.cpp \
	projectitemwidget.cpp \
	addpalettedialog.cpp \
	addtilesetdialog.cpp \
	addeffectlayerdialog.cpp \
	addmapdialog.cpp \
	paletteview.cpp \
	tilesetview.cpp \
	tileseteditorwidget.cpp \
	tilesetpreviewwidget.cpp \
	tilesetanimationwidget.cpp \
	animationframewidget.cpp \
	tilesetpalettewidget.cpp \
	tilesetfloatinglayer.cpp \
	tilesetassociatedsetswidget.cpp \
	associatedtilesetitemwidget.cpp \
	mapview.cpp \
	effectlayerview.cpp \
	mapeditorwidget.cpp \
	renderer.cpp \
	mapfloatinglayer.cpp \
	maplayerwidget.cpp \
	maplayeritemwidget.cpp \
	maptilewidget.cpp \
	mapactorwidget.cpp \
	actoritemwidget.cpp \
	tileselectwidget.cpp \
	emptytilewidget.cpp \
	effectlayersettingswidget.cpp \
	toolwidget.cpp \
	editorview.cpp \
	animation.cpp \
	layersettingsdialog.cpp \
	sprite.cpp \
	spriteanimation.cpp \
	addspritedialog.cpp \
	spriteview.cpp \
	spriteeditorwidget.cpp \
	spritepreviewwidget.cpp \
	spriteanimationwidget.cpp \
	spriteanimationheaderwidget.cpp \
	spritepalettewidget.cpp \
	animationsettingsdialog.cpp \
	spriteselectwidget.cpp \
	actor.cpp \
	actortype.cpp \
	actortypeview.cpp \
	addactortypefielddialog.cpp \
	stringfieldtype.cpp \
	intfieldtype.cpp \
	floatfieldtype.cpp \
	json/jsoncpp.cpp

HEADERS += \
	mainwindow.h \
	theme.h \
	palette.h \
	tile.h \
	tileset.h \
	map.h \
	maplayer.h \
	project.h \
	projectview.h \
	projectitemwidget.h \
	addpalettedialog.h \
	addtilesetdialog.h \
	addeffectlayerdialog.h \
	addmapdialog.h \
	paletteview.h \
	tilesetview.h \
	tileseteditorwidget.h \
	tilesetpreviewwidget.h \
	tilesetanimationwidget.h \
	animationframewidget.h \
	tilesetpalettewidget.h \
	tilesetfloatinglayer.h \
	tilesetassociatedsetswidget.h \
	associatedtilesetitemwidget.h \
	mapview.h \
	effectlayerview.h \
	mapeditorwidget.h \
	renderer.h \
	mapfloatinglayer.h \
	maplayerwidget.h \
	maplayeritemwidget.h \
	maptilewidget.h \
	mapactorwidget.h \
	actoritemwidget.h \
	tileselectwidget.h \
	emptytilewidget.h \
	effectlayersettingswidget.h \
	toolwidget.h \
	editorview.h \
	animation.h \
	layersettingsdialog.h \
	sprite.h \
	spriteanimation.h \
	addspritedialog.h \
	spriteview.h \
	spriteeditorwidget.h \
	spritepreviewwidget.h \
	spriteanimationwidget.h \
	spriteanimationheaderwidget.h \
	spritepalettewidget.h \
	animationsettingsdialog.h \
	spriteselectwidget.h \
	actor.h \
	actortype.h \
	actortypeview.h \
	addactortypefielddialog.h \
	stringfieldtype.h \
	intfieldtype.h \
	floatfieldtype.h \
	json/json.h
