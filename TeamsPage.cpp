#include "TeamsPage.h"
#include "MissionParser.h"
#include "IconsOrder.h"
#include <QScrollArea>
#include <QVBoxLayout>

TeamsPage::TeamsPage(QWidget *parent) :
    QWidget(parent)
{
    gridLayout = new QGridLayout();
    gridLayout->addWidget(new QLabel(tr("ID")), 0, 0);
    gridLayout->addWidget(new QLabel(tr("House")), 0, 1);
    gridLayout->addWidget(new QLabel(tr("AI")), 0, 2);
    gridLayout->addWidget(new QLabel(tr("Type")), 0, 3);
    gridLayout->addWidget(new QLabel(tr("Min")), 0, 4);
    gridLayout->addWidget(new QLabel(tr("Max")), 0, 5);

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

    for (size_t i=0; i<Mission.Teams.size(); ++i)
    {
        DuneMission::Team &t = Mission.Teams[i];
        add();

        houses[i]->setCurrentIndex(t.house);

        if (t.ai == 'K')
            AI[i]->setCurrentIndex(1);

        if (t.type == 'W')
            types[i]->setCurrentIndex(1);

        if (t.type == 'T')
            types[i]->setCurrentIndex(2);

        mins[i]->setValue(t.min);
        maxs[i]->setValue(t.max);
    }
}

void TeamsPage::add()
{
    QLabel *label = new QLabel(QString().sprintf("%d:",houses.size()));
    QComboBox *house = new QComboBox;
    QComboBox *ai = new QComboBox;
    QComboBox *type = new QComboBox;
    QSpinBox *min = new QSpinBox;
    QSpinBox *max = new QSpinBox;
    QPushButton *del = new QPushButton("x");
    del->setMaximumWidth(30);

    connect(del, SIGNAL(clicked()), this, SLOT(remove()));
    min->setMaximum(0xFFFF);
    max->setMaximum(0xFFFF);

    for (int z=0; HouseName[z]; ++z)
    {
        house->addItem(tr(HouseName[z]));
    }

    char *ais[] = {"Normal","Kamikaze",0};
    for (int z=0; ais[z] != 0; ++z)
    {
        ai->addItem(tr(ais[z]));
    }

    char *typenames[] = {"Foot","Wheel","Track",0};
    for (int z=0; typenames[z] != 0; ++z)
    {
        type->addItem(tr(typenames[z]));
    }

    labels.push_back(label);
    houses.push_back(house);
    AI.push_back(ai);
    types.push_back(type);
    mins.push_back(min);
    maxs.push_back(max);
    removeButtons.push_back(del);

    int row = houses.size();
    gridLayout->addWidget(label, row, 0);
    gridLayout->addWidget(house, row, 1);
    gridLayout->addWidget(   ai, row, 2);
    gridLayout->addWidget( type, row, 3);
    gridLayout->addWidget(  min, row, 4);
    gridLayout->addWidget(  max, row, 5);
    gridLayout->addWidget(  del, row, 6);

    gridLayout->setRowStretch(row,0);
    gridLayout->setRowStretch(row+1,1000);
}

void TeamsPage::remove()
{
    for (size_t i=0; i<houses.size(); ++i)
        if (QObject::sender() == removeButtons[i])
        {
            for (size_t j=i; j<houses.size(); ++j)
            {
                gridLayout->removeWidget(labels[j]);
                gridLayout->removeWidget(houses[j]);
                gridLayout->removeWidget(AI[j]);
                gridLayout->removeWidget(types[j]);
                gridLayout->removeWidget(mins[j]);
                gridLayout->removeWidget(maxs[j]);
                gridLayout->removeWidget(removeButtons[j]);
            }
            delete labels[i];
            delete houses[i];
            delete AI[i];
            delete types[i];
            delete mins[i];
            delete maxs[i];
            delete removeButtons[i];

            labels.erase(labels.begin()+i);
            houses.erase(houses.begin()+i);
            AI.erase(AI.begin()+i);
            types.erase(types.begin()+i);
            mins.erase(mins.begin()+i);
            maxs.erase(maxs.begin()+i);
            removeButtons.erase(removeButtons.begin()+i);

            for (size_t j=i; j<houses.size(); ++j)
            {
                labels[j]->setText(QString().sprintf("%d:",j));
                gridLayout->addWidget(labels[j], j+1, 0);
                gridLayout->addWidget(houses[j], j+1, 1);
                gridLayout->addWidget(AI[j], j+1, 2);
                gridLayout->addWidget(types[j], j+1, 3);
                gridLayout->addWidget(mins[j], j+1, 4);
                gridLayout->addWidget(maxs[j], j+1, 5);
                gridLayout->addWidget(removeButtons[j], j+1, 6);
            }

            gridLayout->setRowStretch(houses.size()+1, 1000);
        }
}


void TeamsPage::Apply()
{
    Mission.Teams.clear();
    for (size_t i=0; i<houses.size(); ++i)
    {
        DuneMission::Team t;
        t.house = houses[i]->currentIndex();

        if (AI[i]->currentIndex() == 0)
            t.ai = 'N';
        else
            t.ai = 'K';

        char tval[] = "FWT";
        t.type = tval[types[i]->currentIndex()];

        t.min = mins[i]->value();
        t.max = maxs[i]->value();
        Mission.Teams.push_back(t);
    }
}
