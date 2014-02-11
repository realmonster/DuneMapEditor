#ifndef HOUSESPAGE_H
#define HOUSESPAGE_H

#include <QWidget>
#include <QComboBox>
#include <QSpinBox>

class HousesPage : public QWidget
{
    Q_OBJECT

public:
    explicit HousesPage(QWidget *parent = 0);
    void Apply();

private slots:
    void changedAI(int index);

private:
    QComboBox *houseAI[5];
    QSpinBox *houseCredits[5];
    QSpinBox *houseQuota[5];
    QSpinBox *houseMaxUnits[5];
};

#endif // HOUSESPAGE_H
