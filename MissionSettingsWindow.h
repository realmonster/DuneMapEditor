#ifndef MISSIONSETTINGSWINDOW_H
#define MISSIONSETTINGSWINDOW_H

#include <QDialog>
#include "BasicsPage.h"
#include "HousesPage.h"
#include "StarportPage.h"

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
};

#endif // MISSIONSETTINGSWINDOW_H
