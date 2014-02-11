HEADERS       = Dunes.h \
				DuneStructures.h \
				GLWidget.h \
    DuneGround.h \
    MissionParser.h \
    HousesPage.h \
    BasicsPage.h \
    MissionSettingsWindow.h \
    StarportPage.h

SOURCES       = GLWidget.cpp \
                Main.cpp \
    HousesPage.cpp \
    BasicsPage.cpp \
    MissionParser.cpp \
    MissionSettingsWindow.cpp \
    StarportPage.cpp

MOC_DIR       =	.moc
OBJECTS_DIR   = .obj

QT           += opengl widgets

RESOURCES += \
    resources.qrc

