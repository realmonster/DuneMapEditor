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
