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

#ifndef MISSIONPARSER_H
#define MISSIONPARSER_H

#include <QString>
#include <vector>

struct DuneMission
{
    DuneMission();
    // BASIC
    QString LosePicture;
    QString WinPicture;
    QString BriefPicture;
    int TimeOut;
    int MapScale;
    int CursorPos;
    int TacticalPos;
    int LoseFlags;
    int WinFlags;

    // MAP
    std::vector<int> Bloom;
    std::vector<int> Field;
    int MapSeed;

    // Houses
    struct
    {
        int Quota; // need collect spice
        int Credits;
        int Brain;
        int MaxUnits;
    } House[5];

    // Starport
    struct StarportEntry
    {
        int unit;
        int count;
    };

    std::vector<StarportEntry> Starport;

    // Teams
    struct Team
    {
        int house;
        int ai;
        int type;
        int min;
        int max;
    };

    std::vector<Team> Teams;

    // Units
    struct Unit
    {
        int house;
        int id;
        int life;
        int pos;
        int angle;
        int ai;
    };

    std::vector<Unit> Units;

    // Structures
    struct Structure
    {
        int flag;
        int house;
        int id;
        int life;
        int pos;
    };

    std::vector<Structure> Structures;

    // Reinforcements
    struct Reinforcement
    {
        int house;
        int id;
        int pos;
        int delay;
        int repeat;
    };

    std::vector<Reinforcement> Reinforcements;

    void Clear();
}

extern Mission;

void LoadMission(const QString &filename);
void SaveMission(const QString &filename);

#endif // MISSIONPARSER_H
