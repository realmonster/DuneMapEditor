#include "StarportPage.h"
#include "MissionParser.h"
#include <QScrollArea>
#include <QVBoxLayout>

static int units_order[] = {
 4, // Solder
 2, // Infantry
 5, // Trooper
 3, // Troopers
13, // Trike
14, // Raider Trike
15, // Quad
16, // Harvester
17, // MCV
 9, // Tank
10, // Siege Tank
 7, // Launcher
 8, // Deviator
12, // Sonic
11, // Devastator
 6, // ?
 0, // Carryall
 1, // Thopter
25, // Sandworm
};

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

    for (int i=0; i<Mission.Starport.size(); ++i)
    {
        DuneMission::StarportEntry &se = Mission.Starport[i];
        add();

        for (int j=0; j<sizeof(units_order)/sizeof(units_order[0]); ++j)
            if (units_order[j] == se.unit)
                units[i]->setCurrentIndex(j);
        counts[i]->setValue(se.count);
    }
}

void StarportPage::add()
{
    QLabel *label = new QLabel(QString().sprintf("%d:",units.size()));
    QComboBox *unit = new QComboBox;
    QSpinBox *count = new QSpinBox;
    QPushButton *del = new QPushButton("x");
    del->setMaximumWidth(30);

    connect(del, SIGNAL(clicked()), this, SLOT(remove()));
    count->setMaximum(0xFFFF);

    for (int z=0; z<sizeof(units_order)/sizeof(units_order[0]); ++z)
    {
        int i = units_order[z];
        unit->addItem(QIcon(QString().sprintf(":/units/unit%02d.png",i)),"");
    }
    unit->setIconSize(QSize(32,24));

    labels.push_back(label);
    units.push_back(unit);
    counts.push_back(count);
    removeButtons.push_back(del);

    gridLayout->addWidget(label, units.size(), 0);
    gridLayout->addWidget( unit, units.size(), 1);
    gridLayout->addWidget(count, units.size(), 2);
    gridLayout->addWidget(  del, units.size(), 3);

    gridLayout->setRowStretch(units.size(),0);
    gridLayout->setRowStretch(units.size()+1,1000);
}

void StarportPage::remove()
{
    for (int i=0; i<units.size(); ++i)
        if (QObject::sender() == removeButtons[i])
        {
            for (int j=i; j<units.size(); ++j)
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

            for (int j=i; j<units.size(); ++j)
            {
                labels[j]->setText(QString().sprintf("%d:",j));
                gridLayout->addWidget(labels[j], j+1, 0);
                gridLayout->addWidget(units[j], j+1, 1);
                gridLayout->addWidget(counts[j], j+1, 2);
                gridLayout->addWidget(removeButtons[j], j+1, 3);
            }

            gridLayout->setRowStretch(units.size()+1, 1000);
        }
}


void StarportPage::Apply()
{
    Mission.Starport.clear();
    for (int i=0; i<units.size(); ++i)
    {
        DuneMission::StarportEntry se;
        se.unit = units_order[units[i]->currentIndex()];
        se.count = counts[i]->value();
        Mission.Starport.push_back(se);
    }
}
