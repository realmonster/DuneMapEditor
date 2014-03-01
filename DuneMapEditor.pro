HEADERS       = Dunes.h \
				DuneStructures.h \
				GLWidget.h \
    DuneGround.h \
    MissionParser.h \
    HousesPage.h \
    MissionSettingsWindow.h \
    StarportPage.h \
    TeamsPage.h \
    IconsOrder.h \
    BasicsPage.h \
    ReinforcementsPage.h \
    Electropage.h

SOURCES       = GLWidget.cpp \
                Main.cpp \
    HousesPage.cpp \
    MissionParser.cpp \
    MissionSettingsWindow.cpp \
    StarportPage.cpp \
    TeamsPage.cpp \
    IconsOrder.cpp \
    BasicsPage.cpp \
    ReinforcementsPage.cpp

MOC_DIR       =	.moc
OBJECTS_DIR   = .obj

QT           += opengl widgets

RESOURCES += \
    resources.qrc

