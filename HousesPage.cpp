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

#include "HousesPage.h"
#include "MissionParser.h"
#include <QGridLayout>
#include <QLabel>

HousesPage::HousesPage(QWidget *parent) :
    QWidget(parent)
{
    QGridLayout *gridLayout = new QGridLayout();
    setLayout(gridLayout);

    char *houseName[] = {
        "Harkonnen",
        "Atreides",
        "Ordos",
        "Sardaukar",
        "Fremen",
    };

    gridLayout->addWidget(new QLabel(tr("ID")), 0, 0);
    gridLayout->addWidget(new QLabel(tr("House")), 0, 1);
    gridLayout->addWidget(new QLabel(tr("AI")), 0, 2);
    gridLayout->addWidget(new QLabel(tr("Credits")), 0, 3);
    gridLayout->addWidget(new QLabel(tr("Quota")), 0, 4);
    gridLayout->addWidget(new QLabel(tr("MaxUnits")), 0, 5);
    for (int i=0; i<5; ++i)
    {
        gridLayout->addWidget(new QLabel(QString().sprintf("%d:",i)), i+1, 0);
        gridLayout->addWidget(new QLabel(tr(houseName[i])), i+1, 1);

        houseAI[i] = new QComboBox;
        houseAI[i]->addItem(tr("None"));
        houseAI[i]->addItem(tr("Human"));
        houseAI[i]->addItem(tr("Computer"));

        int index = 0;
        switch(Mission.House[i].Brain)
        {
            case 'H':
                index = 1;
                break;
            case 'C':
                index = 2;
                break;
        }
        houseAI[i]->setCurrentIndex(index);

        connect(houseAI[i], SIGNAL(activated(int)), this, SLOT(changedAI(int)));

        houseCredits[i] = new QSpinBox;
        houseQuota[i] = new QSpinBox;
        houseMaxUnits[i] = new QSpinBox;

        houseCredits[i]->setMaximum(0xFFFF);
        houseQuota[i]->setMaximum(0xFFFF);
        houseMaxUnits[i]->setMaximum(0xFFFF);

        houseCredits[i]->setValue(Mission.House[i].Credits);
        houseQuota[i]->setValue(Mission.House[i].Quota);
        houseMaxUnits[i]->setValue(Mission.House[i].MaxUnits);

        gridLayout->addWidget(houseAI[i], i+1, 2);
        gridLayout->addWidget(houseCredits[i], i+1, 3);
        gridLayout->addWidget(houseQuota[i], i+1, 4);
        gridLayout->addWidget(houseMaxUnits[i], i+1, 5);

        houseCredits[i]->setDisabled(index == 0);
        houseQuota[i]->setDisabled(index == 0);
        houseMaxUnits[i]->setDisabled(index == 0);
    }
    gridLayout->setRowStretch(6,1000);
}

void HousesPage::Apply()
{
    int AIval[] = {
        0,   // None
        'H', // Human
        'C', // Computer
    };

    for (int i=0; i<5; ++i)
    {
        Mission.House[i].Brain = AIval[houseAI[i]->currentIndex()];
        Mission.House[i].Credits = houseCredits[i]->value();
        Mission.House[i].Quota = houseQuota[i]->value();
        Mission.House[i].MaxUnits = houseMaxUnits[i]->value();
    }
}


void HousesPage::changedAI(int index)
{
    QObject *sender = QObject::sender();
    for (int i=0; i<5; ++i)
        if (sender == houseAI[i])
        {
            houseCredits[i]->setDisabled(index == 0);
            houseQuota[i]->setDisabled(index == 0);
            houseMaxUnits[i]->setDisabled(index == 0);
        }
}
