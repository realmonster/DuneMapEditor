#ifndef BASICSPAGE_H
#define BASICSPAGE_H

#include <QWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>

class BasicsPage : public QWidget
{
    Q_OBJECT
public:
    explicit BasicsPage(QWidget *parent = 0);
    void Apply();

private:
    QLineEdit *losePicture;
    QLineEdit *winPicture;
    QLineEdit *briefPicture;
    QSpinBox *timeOut;
    QComboBox *mapScale;
    QSpinBox *cursorPos;
    QSpinBox *tacticalPos;
    QSpinBox *mapSeed;

    QCheckBox *loseFlagBox[8];
    QCheckBox *winFlagBox[8];
};

#endif // BASICSPAGE_H
