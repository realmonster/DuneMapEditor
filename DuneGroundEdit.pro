HEADERS       = Dunes.h \
				DuneStructures.h \
				GLWidget.h \
    DuneGround.h

SOURCES       = GLWidget.cpp \
                Main.cpp

MOC_DIR       =	.moc
OBJECTS_DIR   = .obj

QT           += opengl widgets

RESOURCES += \
    resources.qrc

