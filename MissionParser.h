#ifndef MISSIONPARSER_H
#define MISSIONPARSER_H

struct DuneMission
{
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

    void Clear()
    {
        LosePicture = "LOSTBILD.WSA";
        WinPicture = "WIN1.WSA";
        BriefPicture = "QUAD.WSA";
        MapSeed = 0;
        TimeOut = 0;
        MapScale = 0;
        CursorPos = 0;
        TacticalPos = 0;
        LoseFlags = 0;
        WinFlags = 0;

        for (int i=0; i<5; ++i)
        {
            House[i].MaxUnits = 0;
            House[i].Credits = 0;
            House[i].Quota = 0;
            House[i].Brain = 'C';
        }

        Bloom.clear();
        Field.clear();

        Starport.clear();
        Teams.clear();
        Units.clear();
        Structures.clear();
        Reinforcements.clear();
    }
}

Mission;

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

void LoadMission( QString filename )
{
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly))
        return;
    //FILE *log = fopen("log.txt","w");
    unsigned char buff[20];
    Mission.Clear();
    DuneMission::Structure structure;
    for (;;)
    {
        unsigned char cmd;
        unsigned char subcmd;
        if (!f.read((char*)&cmd,1))
            break;
        f.read((char*)&subcmd,1);
        //fprintf(log,"cmd = %d, subcmd = %d, offset = %X\n",cmd,subcmd,ftell(f)-2);
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
                        for (int i=0; i<Mission.Bloom.size(); ++i)
                            Mission.Bloom[i] = readWord(f);
                        break;

                    // Field
                    case 'F':
                        f.read((char*)buff,2);
                        Mission.Field.resize((buff[0]<<8)|buff[1]);
                        for (int i=0; i<Mission.Field.size(); ++i)
                            Mission.Field[i] = readWord(f);
                        break;

                    case 'S':
                        Mission.MapSeed = readWord(f);
                        //sprintf(buff,"%d",((buff[0]<<8)|buff[1]));
                        //MessageBox(NULL,buff,"Seed",MB_OK);
                        break;
                }
                break;
            // Harkonnen
            case 2:
            // Atreides
            case 3:
            // Ordos
            case 4:
            // Fremen
            case 5:
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
                        //sprintf(buff,"%d(%X)%c",cmd,ftell(f),subcmd);
                        //MessageBox(NULL,buff,"House",MB_OK);
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
                unit.ai    = (buff[10]<<8) | buff[12];
                Mission.Units.push_back(unit);
            }
                break;
            // Structures ( subcmd = unk )
            case 9:
                if (subcmd == 'G')
                {
                    f.read((char*)buff,3*2);
                    structure.pos   = (buff[ 0]<<8) | buff[ 1];
                    structure.house = (buff[ 2]<<8) | buff[ 3];
                    structure.id    = (buff[ 4]<<8) | buff[ 5];
                }
                else
                {
                    f.read((char*)buff,5*2);
                    structure.flag  = (buff[ 0]<<8) | buff[ 1];
                    structure.house = (buff[ 2]<<8) | buff[ 3];
                    structure.id    = (buff[ 4]<<8) | buff[ 5];
                    structure.life  = (buff[ 6]<<8) | buff[ 7];
                    structure.pos   = (buff[ 8]<<8) | buff[ 9];
                }
                Mission.Structures.push_back(structure);
                break;
            // Reinforcements
            case 10:
            {
                f.read((char*)buff,4*2);

                DuneMission::Reinforcement r;
                r.house  = (buff[ 0]<<8) | buff[ 1];
                r.id     = (buff[ 2]<<8) | buff[ 3];
                r.pos    = (buff[ 4]<<8) | buff[ 5];
                r.delay  = buff[6];
                r.repeat = buff[7];
                Mission.Reinforcements.push_back(r);
            }
                break;
        }
    }
    f.close();
    //fclose(log);
    //sprintf((char*)buff,"%d",Units.size());
    //MessageBox(NULL,(char*)buff,"Units.size()",MB_OK);
}

#endif // MISSIONPARSER_H
