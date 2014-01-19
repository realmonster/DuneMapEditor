#ifndef MISSIONPARSER_H
#define MISSIONPARSER_H

struct DuneUnit
{
    short house;
    short id;
    short life;
    short pos;
    short angle;
    short ai;
};

struct DuneStructure
{
    short flag;
    short house;
    short id;
    short life;
    short pos;
};

std::vector<DuneUnit> Units;
std::vector<DuneStructure> Structures;

void LoadMission( const char * filename )
{
    FILE *f = fopen(filename, "rb");
    if (!f)
        return;
    //FILE *log = fopen("log.txt","w");
    unsigned char buff[20];
    Units.clear();
    Structures.clear();
    DuneUnit unit;
    DuneStructure structure;
    for (;;)
    {
        unsigned char cmd;
        unsigned char subcmd;
        if (!fread(&cmd,1,1,f))
            break;
        fread(&subcmd,1,1,f);
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
                    // WinPicture
                    case 1:
                    // BriefPicture
                    case 2:
                        fread(buff,1,2,f);
                        fseek(f,(buff[0]<<8)|buff[1],SEEK_CUR);
                        break;

                    // TimeOut
                    case 3:
                    // MapScale
                    case 4:
                    // CursorPos
                    case 5:
                    // TacticalPos
                    case 6:
                    // LoseFlags
                    case 7:
                    // WinFlags
                    case 8:
                        fread(buff,1,2,f);
                        break;
                }
                break;
            // MAP
            case 1:
                switch(subcmd)
                {
                    // Bloom
                    case 'B':
                    // Field
                    case 'F':
                        fread(buff,1,2,f);
                        fseek(f,((buff[0]<<8)|buff[1])*2,SEEK_CUR);
                        break;
                    case 'S':
                        fread(buff,1,2,f);
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
                    // Credits
                    case 'C':
                    // Brain
                    case 'B':
                    // MaxUnits
                    case 'M':
                        fread(buff,1,2,f);
                        //sprintf(buff,"%d(%X)%c",cmd,ftell(f),subcmd);
                        //MessageBox(NULL,buff,"House",MB_OK);
                        break;
                }
                break;
            // Starport (subcmd = unit)
            case 6:
                fread(buff,1,2,f);
                break;
            // Teams ( subcmd = team id )
            case 7:
                fseek(f,5*2,SEEK_CUR);
                break;
            // Units ( subcmd = unk )
            case 8:
                fread(buff,1,6*2,f);
                unit.house = (buff[ 0]<<8) | buff[ 1];
                unit.id    = (buff[ 2]<<8) | buff[ 3];
                unit.life  = (buff[ 4]<<8) | buff[ 5];
                unit.pos   = (buff[ 6]<<8) | buff[ 7];
                unit.angle = (buff[ 8]<<8) | buff[ 9];
                unit.ai    = (buff[10]<<8) | buff[12];
                Units.push_back(unit);
                break;
            // Structures ( subcmd = unk )
            case 9:
                if (subcmd == 'G')
                {
                    fread(buff,1,3*2,f);
                    structure.pos  = (buff[ 0]<<8) | buff[ 1];
                    structure.house = (buff[ 2]<<8) | buff[ 3];
                    structure.id    = (buff[ 4]<<8) | buff[ 5];
                }
                else
                {
                    fread(buff,1,5*2,f);
                    structure.flag  = (buff[ 0]<<8) | buff[ 1];
                    structure.house = (buff[ 2]<<8) | buff[ 3];
                    structure.id    = (buff[ 4]<<8) | buff[ 5];
                    structure.life  = (buff[ 6]<<8) | buff[ 7];
                    structure.pos   = (buff[ 8]<<8) | buff[ 9];
                }
                Structures.push_back(structure);
                break;
            // Reinforcements
            case 10:
                fread(buff,1,4*2,f);
                break;
        }
    }
    fclose(f);
    //fclose(log);
    //sprintf((char*)buff,"%d",Units.size());
    //MessageBox(NULL,(char*)buff,"Units.size()",MB_OK);
}

#endif // MISSIONPARSER_H
