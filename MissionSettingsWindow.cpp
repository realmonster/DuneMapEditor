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
