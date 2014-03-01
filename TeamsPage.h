#ifndef TEAMSPAGE_H
#define TEAMSPAGE_H

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <vector>

class TeamsPage : public QWidget
{
    Q_OBJECT
public:
    explicit TeamsPage(QWidget *parent = 0);

    void Apply();

private slots:
    void add();
    void remove();

private:
    QGridLayout *gridLayout;
    std::vector<QLabel*> labels;
    std::vector<QComboBox*> houses;
    std::vector<QComboBox*> AI;
    std::vector<QComboBox*> types;
    std::vector<QSpinBox*> mins;
    std::vector<QSpinBox*> maxs;
    std::vector<QPushButton*> removeButtons;
};

#endif // TEAMSPAGE_H
