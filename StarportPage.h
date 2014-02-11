#ifndef STARPORTPAGE_H
#define STARPORTPAGE_H

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <vector>

class StarportPage : public QWidget
{
    Q_OBJECT

public:
    explicit StarportPage(QWidget *parent = 0);
    void Apply();

private slots:
    void add();
    void remove();

private:
    QGridLayout *gridLayout;
    std::vector<QLabel*> labels;
    std::vector<QComboBox*> units;
    std::vector<QSpinBox*> counts;
    std::vector<QPushButton*> removeButtons;
};

#endif // STARPORTPAGE_H
