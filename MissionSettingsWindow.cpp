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

#include "MissionSettingsWindow.h"
#include <QTabWidget>
#include <QDialogButtonBox>
#include <QVBoxLayout>

MissionSettingsWindow::MissionSettingsWindow(QWidget *parent) :
    QDialog(parent)
{
    basics = new BasicsPage;
    houses = new HousesPage;
    starport = new StarportPage;
    teams = new TeamsPage;
    reinforcements = new ReinforcementsPage;

    QTabWidget *tabWidget = new QTabWidget;
    tabWidget->addTab(basics, tr("Basics"));
    tabWidget->addTab(houses, tr("Houses"));
    tabWidget->addTab(starport, tr("Starport"));
    tabWidget->addTab(teams, tr("Teams"));
    tabWidget->addTab(reinforcements, tr("Reinforcements"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(apply()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(cancel()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Mission Settings"));
}

void MissionSettingsWindow::apply()
{
    basics->Apply();
    houses->Apply();
    starport->Apply();
    teams->Apply();
    reinforcements->Apply();
    hide();
}

void MissionSettingsWindow::cancel()
{
    hide();
}
