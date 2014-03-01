#include "ReinforcementsPage.h"
#include "MissionParser.h"
#include "IconsOrder.h"
#include <QScrollArea>
#include <QVBoxLayout>

ReinforcementsPage::ReinforcementsPage(QWidget *parent) :
    QWidget(parent)
{
    gridLayout = new QGridLayout();
    gridLayout->addWidget(new QLabel(tr("ID")), 0, 0);
    gridLayout->addWidget(new QLabel(tr("House")), 0, 1);
    gridLayout->addWidget(new QLabel(tr("Unit")), 0, 2);
    gridLayout->addWidget(new QLabel(tr("Pos")), 0, 3);
    gridLayout->addWidget(new QLabel(tr("Timeout")), 0, 4);
    gridLayout->addWidget(new QLabel(tr("Repeat")), 0, 5);

    gridLayout->setColumnStretch(0, 0);
    gridLayout->setColumnStretch(1, 0);
    gridLayout->setColumnStretch(2, 0);
    gridLayout->setColumnStretch(3, 0);
    gridLayout->setColumnStretch(4, 0);
    gridLayout->setColumnStretch(5, 0);
    gridLayout->setColumnStretch(6, 0);
    gridLayout->setColumnStretch(7, 1000);

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

    for (size_t i=0; i<Mission.Reinforcements.size(); ++i)
    {
        DuneMission::Reinforcement &r = Mission.Reinforcements[i];
        add();

        houses[i]->setCurrentIndex(r.house);

        for (int j=0; UnitsOrder[j] != -1; ++j)
            if (UnitsOrder[j] == r.id)
                units[i]->setCurrentIndex(j);

        coords[i]->setValue(r.pos);
        timeouts[i]->setValue(r.delay);
        repeats[i]->setValue(r.repeat+1);
    }
}

void ReinforcementsPage::add()
{
    QLabel *label = new QLabel(QString().sprintf("%d:",labels.size()));
    QComboBox *house = new QComboBox;
    QComboBox *unit = new QComboBox;
    QSpinBox *coord = new QSpinBox;
    QSpinBox *timeout = new QSpinBox;
    QSpinBox *repeat = new QSpinBox;
    QPushButton *del = new QPushButton("x");
    del->setMaximumWidth(30);

    connect(del, SIGNAL(clicked()), this, SLOT(remove()));

    coord->setToolTip(tr("7 = player base\n6 = enemy base\n0..5 = random\nother = unit position"));

    coord->setMaximum(0xFFFF);
    timeout->setMaximum(0xFFFF);
    repeat->setRange(1,0x10000);

    for (int z=0; HouseName[z]; ++z)
    {
        house->addItem(tr(HouseName[z]));
    }

    for (int z=0; UnitsOrder[z] != -1; ++z)
    {
        int i = UnitsOrder[z];
        unit->addItem(QIcon(QString().sprintf(":/units/unit%02d.png",i)),"");
    }
    unit->setIconSize(QSize(32,24));

    labels.push_back(label);
    houses.push_back(house);
    units.push_back(unit);
    coords.push_back(coord);
    timeouts.push_back(timeout);
    repeats.push_back(repeat);
    removeButtons.push_back(del);

    int row = labels.size();
    gridLayout->addWidget(  label, row, 0);
    gridLayout->addWidget(  house, row, 1);
    gridLayout->addWidget(   unit, row, 2);
    gridLayout->addWidget(  coord, row, 3);
    gridLayout->addWidget(timeout, row, 4);
    gridLayout->addWidget( repeat, row, 5);
    gridLayout->addWidget(    del, row, 6);

    gridLayout->setRowStretch(row,0);
    gridLayout->setRowStretch(row+1,1000);
}

void ReinforcementsPage::remove()
{
    for (size_t i=0; i<houses.size(); ++i)
        if (QObject::sender() == removeButtons[i])
        {
            for (size_t j=i; j<houses.size(); ++j)
            {
                gridLayout->removeWidget(labels[j]);
                gridLayout->removeWidget(houses[j]);
                gridLayout->removeWidget(units[j]);
                gridLayout->removeWidget(coords[j]);
                gridLayout->removeWidget(timeouts[j]);
                gridLayout->removeWidget(repeats[j]);
                gridLayout->removeWidget(removeButtons[j]);
            }
            delete labels[i];
            delete houses[i];
            delete units[i];
            delete coords[i];
            delete timeouts[i];
            delete repeats[i];
            delete removeButtons[i];

            labels.erase(labels.begin()+i);
            houses.erase(houses.begin()+i);
            units.erase(units.begin()+i);
            coords.erase(coords.begin()+i);
            timeouts.erase(timeouts.begin()+i);
            repeats.erase(repeats.begin()+i);
            removeButtons.erase(removeButtons.begin()+i);

            for (size_t j=i; j<labels.size(); ++j)
            {
                labels[j]->setText(QString().sprintf("%d:",j));
                gridLayout->addWidget(labels[j], j+1, 0);
                gridLayout->addWidget(houses[j], j+1, 1);
                gridLayout->addWidget(units[j], j+1, 2);
                gridLayout->addWidget(coords[j], j+1, 3);
                gridLayout->addWidget(timeouts[j], j+1, 4);
                gridLayout->addWidget(repeats[j], j+1, 5);
                gridLayout->addWidget(removeButtons[j], j+1, 6);
            }

            gridLayout->setRowStretch(labels.size()+1, 1000);
        }
}


void ReinforcementsPage::Apply()
{
    Mission.Reinforcements.clear();
    for (size_t i=0; i<labels.size(); ++i)
    {
        DuneMission::Reinforcement r;
        r.house = houses[i]->currentIndex();
        r.id = UnitsOrder[units[i]->currentIndex()];
        r.pos = coords[i]->value();
        r.delay = timeouts[i]->value();
        r.repeat = repeats[i]->value()-1;

        Mission.Reinforcements.push_back(r);
    }
}
