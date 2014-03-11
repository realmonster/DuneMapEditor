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

#include "MissionParser.h"
#include <QFile>

DuneMission Mission;

DuneMission::DuneMission()
{
    Clear();
}

void DuneMission::Clear()
{
    LosePicture = "LOSTBILD.WSA";
    WinPicture = "WIN1.WSA";
    BriefPicture = "QUAD.WSA";
    MapSeed = 0;
    TimeOut = 0;
    MapScale = 0;
    CursorPos = 0;
    TacticalPos = 0;
    LoseFlags = 1; // lost all units and buildings
    WinFlags = 3; // destroyed all enemy buildings & FLAG 2

    for (int i=0; i<5; ++i)
    {
        House[i].MaxUnits = 0;
        House[i].Credits = 0;
        House[i].Quota = 0;
        House[i].Brain = 0;
    }

    Bloom.clear();
    Field.clear();

    Starport.clear();
    Teams.clear();
    Units.clear();
    Structures.clear();
    Reinforcements.clear();
}

static QString readMString(QFile &f)
{
    static unsigned char buff[2];
    f.read((char*)buff,2);
    return f.read((buff[0]<<8)|buff[1]);
}

static int readWord(QFile &f)
{
    static unsigned char buff[2];
    f.read((char*)buff,2);
    return (buff[0]<<8)|buff[1];
}

void LoadMission( const QString &filename )
{
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly))
        return;

    unsigned char buff[20];
    Mission.Clear();
    for (;;)
    {
        unsigned char cmd;
        unsigned char subcmd;
        if (!f.read((char*)&cmd,1))
            break;
        f.read((char*)&subcmd,1);
        if (cmd & 0x80)
            break;
        switch(cmd)
        {
            // Settings
            case 0:
                switch(subcmd)
                {
                    // LosePicture
                    case 0:
                        Mission.LosePicture = readMString(f);
                        break;
                    // WinPicture
                    case 1:
                        Mission.WinPicture = readMString(f);
                        break;
                    // BriefPicture
                    case 2:
                        Mission.BriefPicture = readMString(f);
                        break;

                    // TimeOut
                    case 3:
                        Mission.TimeOut = readWord(f);
                        break;
                    // MapScale
                    case 4:
                        Mission.MapScale = readWord(f);
                        break;
                    // CursorPos
                    case 5:
                        Mission.CursorPos = readWord(f);
                        break;
                    // TacticalPos
                    case 6:
                        Mission.TacticalPos = readWord(f);
                        break;
                    // LoseFlags
                    case 7:
                        Mission.LoseFlags = readWord(f);
                        break;
                    // WinFlags
                    case 8:
                        Mission.WinFlags = readWord(f);
                        break;
                }
                break;
            // MAP
            case 1:
                switch(subcmd)
                {
                    // Bloom
                    case 'B':
                        f.read((char*)buff,2);
                        Mission.Bloom.resize((buff[0]<<8)|buff[1]);
                        for (size_t i=0; i<Mission.Bloom.size(); ++i)
                            Mission.Bloom[i] = readWord(f);
                        break;

                    // Field
                    case 'F':
                        f.read((char*)buff,2);
                        Mission.Field.resize((buff[0]<<8)|buff[1]);
                        for (size_t i=0; i<Mission.Field.size(); ++i)
                            Mission.Field[i] = readWord(f);
                        break;

                    case 'S':
                        Mission.MapSeed = readWord(f);
                        break;
                }
                break;
            // Harkonnen
            case 2:
            // Atreides
            case 3:
            // Ordos
            case 4:
            // Sadaukar
            case 5:
            // Fremen
            case 11:
                if (cmd == 11)
                    cmd = 6;
                switch(subcmd)
                {
                    // Quota
                    case 'Q':
                        Mission.House[cmd-2].Quota = readWord(f);
                        break;
                    // Credits
                    case 'C':
                        Mission.House[cmd-2].Credits = readWord(f);
                        break;
                    // Brain
                    case 'B':
                        Mission.House[cmd-2].Brain = readWord(f);
                        break;
                    // MaxUnits
                    case 'M':
                        Mission.House[cmd-2].MaxUnits = readWord(f);
                        break;
                }
                break;
            // Starport (subcmd = unit)
            case 6:
            {
                DuneMission::StarportEntry se;
                se.unit = subcmd;
                se.count = readWord(f);
                Mission.Starport.push_back(se);
            }
                break;
            // Teams ( subcmd = team id )
            case 7:
            {
                DuneMission::Team team;
                team.house = readWord(f);
                team.ai    = readWord(f);
                team.type  = readWord(f);
                team.min   = readWord(f);
                team.max   = readWord(f);
                Mission.Teams.push_back(team);
            }
                break;
            // Units ( subcmd = unk )
            case 8:
            {
                f.read((char*)buff,6*2);

                DuneMission::Unit unit;
                unit.house = (buff[ 0]<<8) | buff[ 1];
                unit.id    = (buff[ 2]<<8) | buff[ 3];
                unit.life  = (buff[ 4]<<8) | buff[ 5];
                unit.pos   = (buff[ 6]<<8) | buff[ 7];
                unit.angle = (buff[ 8]<<8) | buff[ 9];
                unit.ai    = (buff[10]<<8) | buff[11];
                Mission.Units.push_back(unit);
            }
                break;
            // Structures ( subcmd = unk )
            case 9:
            {
                DuneMission::Structure structure;
                if (subcmd == 'G')
                {
                    f.read((char*)buff,3*2);
                    structure.pos   = (buff[0]<<8) | buff[1];
                    structure.house = (buff[2]<<8) | buff[3];
                    structure.id    = (buff[4]<<8) | buff[5];
                }
                else
                {
                    f.read((char*)buff,5*2);
                    structure.flag  = (buff[0]<<8) | buff[1];
                    structure.house = (buff[2]<<8) | buff[3];
                    structure.id    = (buff[4]<<8) | buff[5];
                    structure.life  = (buff[6]<<8) | buff[7];
                    structure.pos   = (buff[8]<<8) | buff[9];
                }
                Mission.Structures.push_back(structure);
            }
                break;
            // Reinforcements
            case 10:
            {
                f.read((char*)buff,4*2);

                DuneMission::Reinforcement r;
                r.house  = (buff[0]<<8) | buff[1];
                r.id     = (buff[2]<<8) | buff[3];
                r.pos    = (buff[4]<<8) | buff[5];
                r.delay  = buff[6];
                r.repeat = buff[7];
                Mission.Reinforcements.push_back(r);
            }
                break;
        }
    }
    f.close();
}

static void writecmd(QFile &f, int cmd, int subcmd)
{
    static char buff[2];
    buff[0] = cmd;
    buff[1] = subcmd;
    f.write(buff,2);
}

static void writeWord(QFile &f, int value)
{
    static char buff[2];
    buff[0] = value >> 8;
    buff[1] = value;
    f.write(buff,2);
}

static void writeString(QFile &f, QString string)
{
    int len = string.length();
    if (!(len&1))
        writeWord(f,len+2);
    else
        writeWord(f,len+1);
    for (int i=0; i<string.length(); ++i)
        f.putChar(string[i].unicode());
    f.putChar(0);
    if (!(len&1))
        f.putChar(0);
}

void SaveMission( const QString &filename )
{
    QFile f(filename);
    if (!f.open(QIODevice::WriteOnly))
        return;

    writecmd(f,0,0);
    writeString(f,Mission.LosePicture);
    writecmd(f,0,1);
    writeString(f,Mission.WinPicture);
    writecmd(f,0,2);
    writeString(f,Mission.BriefPicture);
    writecmd(f,0,3);
    writeWord(f,Mission.TimeOut);
    writecmd(f,0,4);
    writeWord(f,Mission.MapScale);
    writecmd(f,0,5);
    writeWord(f,Mission.CursorPos);
    writecmd(f,0,6);
    writeWord(f,Mission.TacticalPos);
    writecmd(f,0,7);
    writeWord(f,Mission.LoseFlags);
    writecmd(f,0,8);
    writeWord(f,Mission.WinFlags);

    if (Mission.Bloom.size())
    {
        writecmd(f,1,'B');
        writeWord(f,Mission.Bloom.size());
        for (size_t i=0; i<Mission.Bloom.size(); ++i)
            writeWord(f,Mission.Bloom[i]);
    }

    if (Mission.Field.size())
    {
        writecmd(f,1,'F');
        writeWord(f,Mission.Field.size());
        for (size_t i=0; i<Mission.Field.size(); ++i)
            writeWord(f,Mission.Field[i]);
    }

    writecmd(f,1,'S');
    writeWord(f,Mission.MapSeed);

    for (int i=0; i<5; ++i)
    if (Mission.House[i].Brain != 0)
    {
        int cmd = i+2;
        if (i == 4)
            cmd = 11; // Fremen
        writecmd(f,cmd,'Q');
        writeWord(f,Mission.House[i].Quota);
        writecmd(f,cmd,'C');
        writeWord(f,Mission.House[i].Credits);
        writecmd(f,cmd,'B');
        writeWord(f,Mission.House[i].Brain);
        writecmd(f,cmd,'M');
        writeWord(f,Mission.House[i].MaxUnits);
    }

    for (size_t i=0; i<Mission.Starport.size(); ++i)
    {
        writecmd(f,6,Mission.Starport[i].unit);
        writeWord(f,Mission.Starport[i].count);
    }

    for (size_t i=0; i<Mission.Teams.size(); ++i)
    {
        DuneMission::Team &team = Mission.Teams[i];
        writecmd(f,7,i+1);
        writeWord(f,team.house);
        writeWord(f,team.ai);
        writeWord(f,team.type);
        writeWord(f,team.min);
        writeWord(f,team.max);
    }

    for (size_t i=0; i<Mission.Units.size(); ++i)
    {
        DuneMission::Unit &unit = Mission.Units[i];
        writecmd(f,8,i+1);
        writeWord(f,unit.house);
        writeWord(f,unit.id);
        writeWord(f,unit.life);
        writeWord(f,unit.pos);
        writeWord(f,unit.angle);
        writeWord(f,unit.ai);
    }

    for (size_t i=0; i<Mission.Structures.size(); ++i)
    {
        DuneMission::Structure &structure = Mission.Structures[i];
        if (structure.id == 0 // concrate
         || structure.id == 0xE) // wall
        {
            writecmd(f,9,'G');
            writeWord(f,structure.pos);
            writeWord(f,structure.house);
            writeWord(f,structure.id);
        }
        else
        {
            writecmd(f,9,'I');
            writeWord(f,structure.flag);
            writeWord(f,structure.house);
            writeWord(f,structure.id);
            writeWord(f,structure.life);
            writeWord(f,structure.pos);
        }
    }

    for (size_t i=0; i<Mission.Reinforcements.size(); ++i)
    {
        DuneMission::Reinforcement &r = Mission.Reinforcements[i];
        writecmd(f,10,i+1);
        writeWord(f,r.house);
        writeWord(f,r.id);
        writeWord(f,r.pos);
        f.putChar(r.delay);
        f.putChar(r.repeat);
    }

    // END
    writeWord(f,0xFFFF);

    f.close();
}

