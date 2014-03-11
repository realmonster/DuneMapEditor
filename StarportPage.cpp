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

#include "StarportPage.h"
#include "MissionParser.h"
#include "IconsOrder.h"
#include <QScrollArea>
#include <QVBoxLayout>

StarportPage::StarportPage(QWidget *parent) :
    QWidget(parent)
{
    gridLayout = new QGridLayout();
    gridLayout->addWidget(new QLabel(tr("ID")), 0, 0);
    gridLayout->addWidget(new QLabel(tr("Unit")), 0, 1);
    gridLayout->addWidget(new QLabel(tr("Count")), 0, 2);

    gridLayout->setColumnStretch(0, 0);
    gridLayout->setColumnStretch(1, 0);
    gridLayout->setColumnStretch(2, 0);
    gridLayout->setColumnStretch(3, 0);
    gridLayout->setColumnStretch(4, 1000);

    gridLayout->setRowStretch(1,1000);

    QWidget *tempWidget = new QWidget;
    tempWidget->setLayout(gridLayout);

    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidget(tempWidget);
    scrollArea->setWidgetResizable(true);

    QPushButton *button = new QPushButton(tr("Add"),this);

    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->addWidget(scrollArea);
    layout->addWidget(button);

    connect(button,SIGNAL(clicked()),this,SLOT(add()));

    for (size_t i=0; i<Mission.Starport.size(); ++i)
    {
        DuneMission::StarportEntry &se = Mission.Starport[i];
        add();

        for (int j=0; UnitsOrder[j] != -1; ++j)
            if (UnitsOrder[j] == se.unit)
                units[i]->setCurrentIndex(j);
        counts[i]->setValue(se.count);
    }
}

void StarportPage::add()
{
    QLabel *label = new QLabel(QString().sprintf("%d:",labels.size()));
    QComboBox *unit = new QComboBox;
    QSpinBox *count = new QSpinBox;
    QPushButton *del = new QPushButton("x");
    del->setMaximumWidth(30);

    connect(del, SIGNAL(clicked()), this, SLOT(remove()));
    count->setMaximum(0xFFFF);

    for (int z=0; UnitsOrder[z] != -1; ++z)
    {
        int i = UnitsOrder[z];
        unit->addItem(QIcon(QString().sprintf(":/units/unit%02d.png",i)),"");
    }
    unit->setIconSize(QSize(32,24));

    labels.push_back(label);
    units.push_back(unit);
    counts.push_back(count);
    removeButtons.push_back(del);

    int row = labels.size();
    gridLayout->addWidget(label, row, 0);
    gridLayout->addWidget( unit, row, 1);
    gridLayout->addWidget(count, row, 2);
    gridLayout->addWidget(  del, row, 3);

    gridLayout->setRowStretch(row,0);
    gridLayout->setRowStretch(row+1,1000);
}

void StarportPage::remove()
{
    for (size_t i=0; i<labels.size(); ++i)
        if (QObject::sender() == removeButtons[i])
        {
            for (size_t j=i; j<labels.size(); ++j)
            {
                gridLayout->removeWidget(labels[j]);
                gridLayout->removeWidget(units[j]);
                gridLayout->removeWidget(counts[j]);
                gridLayout->removeWidget(removeButtons[j]);
            }
            delete labels[i];
            delete units[i];
            delete counts[i];
            delete removeButtons[i];

            labels.erase(labels.begin()+i);
            units.erase(units.begin()+i);
            counts.erase(counts.begin()+i);
            removeButtons.erase(removeButtons.begin()+i);

            for (size_t j=i; j<labels.size(); ++j)
            {
                labels[j]->setText(QString().sprintf("%d:",j));
                gridLayout->addWidget(labels[j], j+1, 0);
                gridLayout->addWidget(units[j], j+1, 1);
                gridLayout->addWidget(counts[j], j+1, 2);
                gridLayout->addWidget(removeButtons[j], j+1, 3);
            }

            gridLayout->setRowStretch(labels.size()+1, 1000);
        }
}


void StarportPage::Apply()
{
    Mission.Starport.clear();
    for (size_t i=0; i<labels.size(); ++i)
    {
        DuneMission::StarportEntry se;
        se.unit = UnitsOrder[units[i]->currentIndex()];
        se.count = counts[i]->value();
        Mission.Starport.push_back(se);
    }
}
