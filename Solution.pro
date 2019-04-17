TEMPLATE = subdirs

SUBDIRS += \
    QtnProperty \
    Editor
	
QtnProperty.file = 3rdParty/QtnProperty/QtnProperty.pro
Editor.file = ./Editor.pro

Editor.depends = QtnProperty
