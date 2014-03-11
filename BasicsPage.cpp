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

#include "BasicsPage.h"
#include "MissionParser.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>

BasicsPage::BasicsPage(QWidget *parent) :
    QWidget(parent)
{
    QGridLayout *gridLayout = new QGridLayout(this);

    losePicture  = new QLineEdit(Mission.LosePicture);
    winPicture   = new QLineEdit(Mission.WinPicture);
    briefPicture = new QLineEdit(Mission.BriefPicture);
    timeOut      = new QSpinBox;
    mapScale     = new QComboBox;
    cursorPos    = new QSpinBox;
    tacticalPos  = new QSpinBox;
    mapSeed      = new QSpinBox;

    losePicture->setMaxLength(100);
    winPicture->setMaxLength(100);
    briefPicture->setMaxLength(100);

    mapScale->addItem( tr("(0) Big") );
    mapScale->addItem( tr("(1) Small") );

    mapScale->setCurrentIndex(Mission.MapScale?1:0);

    timeOut->setMaximum(0xFFFF);
    cursorPos->setMaximum(0xFFFF);
    tacticalPos->setMaximum(0xFFFF);
    mapSeed->setMaximum(0xFFFF);

    timeOut->setValue(Mission.TimeOut);
    cursorPos->setValue(Mission.CursorPos);
    tacticalPos->setValue(Mission.TacticalPos);
    mapSeed->setValue(Mission.MapSeed);

    gridLayout->addWidget(losePicture, 0, 1);
    gridLayout->addWidget(winPicture, 1, 1);
    gridLayout->addWidget(briefPicture, 2, 1);
    gridLayout->addWidget(timeOut, 3, 1);
    gridLayout->addWidget(mapScale, 4, 1);
    gridLayout->addWidget(cursorPos, 5, 1);
    gridLayout->addWidget(tacticalPos, 6, 1);
    gridLayout->addWidget(mapSeed, 9, 1);

    gridLayout->addWidget(new QLabel( tr("Lose Picture:")), 0, 0);
    gridLayout->addWidget(new QLabel( tr("Win Picture:")), 1, 0);
    gridLayout->addWidget(new QLabel( tr("Brief Picture:")), 2, 0);
    gridLayout->addWidget(new QLabel( tr("Time Out:")), 3, 0);
    gridLayout->addWidget(new QLabel( tr("Map Scale:")), 4, 0);
    gridLayout->addWidget(new QLabel( tr("Cursor Pos:")), 5, 0);
    gridLayout->addWidget(new QLabel( tr("Tactical Pos:")), 6, 0);
    gridLayout->addWidget(new QLabel( tr("Lose Flags:")), 7, 0);
    gridLayout->addWidget(new QLabel( tr("Win Flags:")), 8, 0);
    gridLayout->addWidget(new QLabel( tr("Map Seed:")), 9, 0);

    char *loseTooltips[] = {
        "Lost all units and buildings",
        "Allies lost all buildings",
        "Lost all buildings",
        "Time is out",
        "Lost all units and buildings and buidings of allies",
        "Flag 6",
        "Flag 7",
        "Flag 8",
    };
    char *winTooltips[] = {
        "Destroyed all enemy buildings",
        "FLAG 2",
        "Enough spice collected",
        "Time is out",
        "Destroyed all enemy buildings and enough spice colleted",
        "Destroyed all enemy units",
        "Destroyed all enemy units and enough spice colleted",
        "Flag 8",
    };

    QHBoxLayout *loseFlagsLayout = new QHBoxLayout;
    QHBoxLayout *winFlagsLayout = new QHBoxLayout;
    for (int i=0; i<8; ++i)
    {
        loseFlagBox[i] = new QCheckBox;
        winFlagBox[i] = new QCheckBox;

        loseFlagBox[i]->setChecked( Mission.LoseFlags&(1<<i) );
        winFlagBox[i]->setChecked( Mission.WinFlags&(1<<i) );

        loseFlagBox[i]->setToolTip( tr(loseTooltips[i]) );
        winFlagBox[i]->setToolTip( tr(winTooltips[i]) );

        loseFlagsLayout->addWidget(loseFlagBox[i]);
        winFlagsLayout->addWidget(winFlagBox[i]);
    }
    gridLayout->addLayout(loseFlagsLayout, 7, 1);
    gridLayout->addLayout(winFlagsLayout, 8, 1);
    //for (int i=0; i<10; ++i)
    //    gridLayout->setRowStretch(i,0);
    //gridLayout->setRowStretch(10,1000);
}

void BasicsPage::Apply()
{
    Mission.LosePicture  = losePicture->text();
    Mission.WinPicture   = winPicture->text();
    Mission.BriefPicture = briefPicture->text();

    Mission.TimeOut     = timeOut->value();
    Mission.MapScale    = mapScale->currentIndex();
    Mission.CursorPos   = cursorPos->value();
    Mission.TacticalPos = tacticalPos->value();
    Mission.MapSeed     = mapSeed->value();

    Mission.LoseFlags = 0;
    Mission.WinFlags  = 0;
    for (int i=0; i<8; ++i)
    {
        Mission.LoseFlags |= ((loseFlagBox[i]->isChecked()?1:0)<<i);
        Mission.WinFlags  |= (( winFlagBox[i]->isChecked()?1:0)<<i);
    }
}
