#ifndef MISSIONSETTINGSWINDOW_H
#define MISSIONSETTINGSWINDOW_H

#include <QDialog>
#include "BasicsPage.h"
#include "HousesPage.h"
#include "StarportPage.h"
#include "TeamsPage.h"
#include "ReinforcementsPage.h"

class MissionSettingsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MissionSettingsWindow(QWidget *parent = 0);

private slots:
    void apply();
    void cancel();

private:
    BasicsPage *basics;
    HousesPage *houses;
    StarportPage *starport;
    TeamsPage *teams;
    ReinforcementsPage *reinforcements;
};

#endif // MISSIONSETTINGSWINDOW_H
