/*
    This file is part of DuneMapEditor.
 
    DuneMapEditor is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
 
    DuneMapEditor is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with DuneMapEditor.  If not, see <http://www.gnu.org/licenses/>.
*/

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
