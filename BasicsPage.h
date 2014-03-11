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
