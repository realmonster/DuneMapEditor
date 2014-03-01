#ifndef REINFORCEMENTSPAGE_H
#define REINFORCEMENTSPAGE_H

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <vector>

class ReinforcementsPage : public QWidget
{
    Q_OBJECT
public:
    explicit ReinforcementsPage(QWidget *parent = 0);

    void Apply();

private slots:
    void add();
    void remove();

private:
    QGridLayout *gridLayout;
    std::vector<QLabel*> labels;
    std::vector<QComboBox*> houses;
    std::vector<QComboBox*> units;
    std::vector<QSpinBox*> coords;
    std::vector<QSpinBox*> timeouts;
    std::vector<QSpinBox*> repeats;
    std::vector<QPushButton*> removeButtons;
};

#endif // REINFORCEMENTSPAGE_H
