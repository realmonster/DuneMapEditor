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

#ifndef DUNEGROUND_H
#define DUNEGROUND_H

struct DuneGround
{
    enum Types {Dust, Ground, SpiceLow, SpiceHigh, Dune};
    int width;
    int height;
    unsigned char *data;
    unsigned char *types;
    DuneGround(int _width, int _height)
    {
        width = _width;
        height = _height;
        data = (unsigned char *)malloc(width*height*sizeof(unsigned char));
        types = (unsigned char *)malloc(twidth()*theight()*sizeof(unsigned char));
        Clear();
    }

    ~DuneGround()
    {
        if (data)
            free(data);
        if (types)
            free(types);
    }

    DuneGround(const DuneGround & g)
    {
        data = 0;
        types = 0;
        width = 0;
        height = 0;
        (*this) = g;
    }

    void resize(int _width, int _height)
    {
        DuneGround n(_width, _height);

        for (int x=0; x<n.width; ++x)
            for (int y=0; y<n.height; ++y)
            {
                unsigned char v = 0x7F;
                if (in(x,y))
                     v = (*this)[x][y];
                n[x][y] = v;
            }

        free(data);
        data = n.data;
        free(types);
        types = n.types;

        width = _width;
        height = _height;

        n.data = 0;
        n.types = 0;
    }

    const DuneGround & operator = (const DuneGround &g)
    {
        if (width != g.width
         || height != g.height)
        {
            if (data)
                free(data);
            if (types)
                free(types);
            data = (unsigned char *)malloc(g.width*g.height*sizeof(unsigned char));
            types = (unsigned char *)malloc(g.twidth()*g.theight()*sizeof(unsigned char));
        }
        width = g.width;
        height = g.height;
        memcpy(data,g.data,width*height*sizeof(unsigned char));
        memcpy(types,g.types,twidth()*theight()*sizeof(unsigned char));
        return (*this);
    }

    void Clear()
    {
        for (int x=0; x<width; ++x)
            for (int y=0; y<height; ++y)
                (*this)[x][y] = 0x7F;
        for (int x=0; x<twidth(); ++x)
            for (int y=0; y<theight(); ++y)
                this->t(x,y) = Dust;
    }

    inline int twidth() const
    {
        return width*2+1;
    }

    inline int theight() const
    {
        return height*2+1;
    }

    inline bool tin(int x, int y) const
    {
        return (x>=0 && y>=0 && x<twidth() && y<theight());
    }

    inline bool in(int x, int y) const
    {
        return (x>=0 && y>=0 && x<width && y<height);
    }

    // get tile type based on mask values.
    int getTileType(int x, int y)
    {
        int mask = 0;

        x *= 2; y *= 2; // convert into mask coords

        // all except corners
        mask |= (1 << t(1 + x,     y));
        mask |= (1 << t(    x, 1 + y));
        mask |= (1 << t(1 + x, 1 + y));
        mask |= (1 << t(2 + x, 1 + y));
        mask |= (1 << t(1 + x, 2 + y));

        // corners
        if (t(x,     y)     == Dune
         || t(x + 2, y)     == Dune
         || t(x,     y + 2) == Dune
         || t(x + 2, y + 2) == Dune)
            mask |= (1 << Dune);

        switch (mask)
        {
            case (1 << Ground):
            case (1 << Ground) | (1 << Dust): return Ground;
            case (1 << SpiceLow): return SpiceHigh;
            case (1 << SpiceLow)|(1 << Dust): return SpiceLow;
            case (1 << Dune): return Dune;
            case (1 << Dune)|(1 << Dust): return Dune;
            case (1 << SpiceHigh): return SpiceHigh;
            case (1 << SpiceHigh)|(1 << SpiceLow): return SpiceHigh;
            case (1 << SpiceHigh)|(1 << SpiceLow)|(1 << Dust): return SpiceLow;
            case (1 << SpiceHigh)|(1 << Dust): return SpiceLow;
            default: return Dust;
        }
    }

    // check is tile may use high spice.
    // tile can use high spice if each of four neighbors are spice.
    bool isHighSpice(int x, int y)
    {
        if (getTileType(x,y) != SpiceHigh)
            return false;
        const int ok_mask = (1 << SpiceLow) | (1 << SpiceHigh);
        if ((!in(x,y-1) || (1 << getTileType(x,y-1)) & ok_mask)
         && (!in(x+1,y) || (1 << getTileType(x+1,y)) & ok_mask)
         && (!in(x,y+1) || (1 << getTileType(x,y+1)) & ok_mask)
         && (!in(x-1,y) || (1 << getTileType(x-1,y)) & ok_mask))
            return true;
        return false;
    }

    // tile is Dune or outside map
    // shortcut method, use with care
    bool isDuneTile(int x, int y)
    {
        return (!in(x, y) || getTileType(x, y) == Dune);
    }

    // check is mask position is valid dune corner.
    // it return false if you need to remove corner to resolve conflict.
    // it assume that t(x, y) == Dune, and all four tiles around has type Dune.
    bool isDuneCorner(int x, int y)
    {
        if (y < 2)
            return true;

        int a1 = -1, a2 = -1;
        if (t(x, y - 2) == Dune)
        {
            if (in(x/2, y/2 - 2))
                a2 = getTileType(x/2, y/2 - 2);
            else
                a2 = Dune;
            if (in(x/2 - 1, y/2 - 2))
                a1 = getTileType(x/2 - 1, y/2 - 2);
            else
                a1 = Dune;
            if (a1 == Dune && a2 == Dune && isDuneCorner(x, y - 2))
                return true;
        }

        if (x >= 2 && t(x - 2, y - 2) == Dune)
        {
            if (a1 < 0)
            {
                if (in(x/2 - 1, y/2 - 2))
                    a1 = getTileType(x/2 - 1, y/2 - 2);
                else
                    a1 = Dune;
            }
            if (a1 == Dune
             && isDuneTile(x/2 - 2, y/2 - 2)
             && isDuneTile(x/2 - 2, y/2 - 1)
             && isDuneCorner(x - 2, y - 2))
            {
                if (!in(x/2 - 2, y/2) || getTileType(x/2 - 2, y/2) == Dune)
                {
                    if (t(x - 2, y) != Dune || !isDuneCorner(x - 2, y))
                        return false;
                }
                else
                    return false;
            }
        }

        if (tin(x + 2, y - 2) && t(x + 2, y - 2) == Dune)
        {
            if (a2 < 0)
            {
                if (in(x/2 , y/2 - 2))
                    a2 = getTileType(x/2, y/2 - 2);
                else
                    a2 = Dune;
            }
            if (a2 == Dune
             && isDuneTile(x/2 + 1, y/2 - 2)
             && isDuneTile(x/2 + 1, y/2 - 1)
             && isDuneCorner(x + 2, y - 2))
            {
                if (!in(x/2 + 1, y/2) || getTileType(x/2 + 1, y/2) == Dune)
                {
                    if (t(x + 2, y) != Dune || !isDuneCorner(x + 2, y))
                        return false;
                }
                else
                    return false;
            }
        }
        return true;
    }

    void Draw(int x, int y, unsigned char type)
    {
        //   0   1   2
        // 0 1 2 3 4 5 6
        this->t(x,y) = type;
        if (in(x/2, y/2))
            Correct(x/2, y/2, type);
        if (in(x/2-1, y/2))
            Correct(x/2-1,y/2,type);
        if (in(x/2, y/2-1))
            Correct(x/2,y/2-1,type);
        if (in(x/2-1, y/2-1))
            Correct(x/2-1,y/2-1,type);
    }

    void Correct(int x, int y, unsigned char type_draw)
    {
        int k = 0;
        if (type_draw != Dust)
            k = GetK(x,y,type_draw);
        int allowed = 0;
        int replace = Dust;
        if (type_draw == Ground)
            allowed = ((1<<Ground) | (1<<Dust));
        if (type_draw == SpiceLow)
            allowed = ((1<<SpiceLow) | (1<<SpiceHigh) | (1<<Dust));
        if (type_draw == SpiceHigh)
        {
            allowed = ((1<<SpiceLow) | (1<<SpiceHigh));
            replace = SpiceLow;
        }
        int fmask = make_dunemask(0,1,0,1,1,1,0,1,0);
        if (type_draw != Dune && k != 0)
        {
            for (int i=0; i<3; ++i)
                for (int j=0; j<3; ++j)
                    if ((~allowed)&(1<<t(i+x*2,j+y*2))
                    && (((fmask & (1<<(i+j*3))) || type_draw == SpiceHigh)
                    || t(i+x*2,j+y*2) == Dune) )
                        Draw(i+x*2,j+y*2,replace);
        }
        if (type_draw == Dune && k != 0)
        {
            for (int i=0; i<3; ++i)
                for (int j=0; j<3; ++j)
                    if ((fmask & (1<<(i+j*3)))
                     &&	t(i+x*2,j+y*2) != Dune
                     && t(i+x*2,j+y*2) != Dust)
                        Draw(i+x*2,j+y*2,Dust);
            /*int k = GetK(x,y,Dune);
            int mask = dunemask[k];
            for (int i=0; i<3; ++i)
                for (int j=0; j<3; ++j)
                {
                    if ((mask & (1<<(i+j*3)))
                     && t(i+x*2,j+y*2) != Dune)
                        Draw(i+x*2,j+y*2,Dune);
                    if (!(mask & (1<<(i+j*3)))
                     && t(i+x*2,j+y*2) == Dune)
                        Draw(i+x*2,j+y*2,Dust);
                }*/
        }
        for (int i=-1; i<2; ++i)
            for (int j=-1; j<2; ++j)
                if (in(x+i,y+j))
                    Update(x+i, y+j);
    }

    int GetK(int x, int y, unsigned char type)
    {
        int mask = 0;
        int k = 0;
        for (int i=0; i<3; ++i)
            for (int j=0; j<3; ++j)
                if (t(i+x*2,j+y*2) == type
                || (type==SpiceLow && t(i+x*2,j+y*2) == SpiceHigh))
                    mask |= 1<<(i+j*3);
        //1    2   4
        //8   16  32
        //64 128 256
        if ( type == Dune )
        {
            k = 0;
            int min = 10;
            for (int i=0; i<256; ++i)
                if (dunemask[i] != -1)
                {
                    int d = DunemaskDist((dunemask[i] & DUNE_MASK), (mask & DUNE_MASK));
                    if (((dunemask[i] & DUNE_MASK) & (mask & DUNE_MASK)) == (mask & DUNE_MASK)
                     && d < min)
                    {
                        k = i;
                        min = d;
                    }
                }
            for (int i=0; i<256; ++i)
                if (dunemask[i] != -1)
                {
                    if (((dunemask[k] & DUNE_MASK)|(mask&16)) == (dunemask[i] & DUNE_MASKMID))
                        k = i;
                }
            if (k == 0x9C && (mask & 2))
                k = 0x60;
            if (dunemask[k] == 0)
                k = 0;
        }
        else
        {
            if ( (mask&(1+2+4)) == (1+2+4))
                k++;
            if ( (mask&(4+32+256)) == (4+32+256))
                k |= 2;
            if ( (mask&(64+128+256)) == (64+128+256))
                k |= 4;
            if ( (mask&(1+8+64)) == (1+8+64))
                k |= 8;

            if ( type == Ground && k == 0 && (mask & 16)) // point
                k = 16;
            if ( k == 3 && !(mask & 16))
                k = 17;
            if ( k == 6 && !(mask & 16))
                k = 18;
            if ( k == 12 && !(mask & 16))
                k = 20;
            if ( k == 9 && !(mask & 16))
                k = 19;
        }
        return k;
    }

    // change tile id according to mask
    void Update(int x, int y)
    {
        int here = getTileType(x, y);
        if (here == Dust)
        {
            (*this)[x][y] = 0x7F;
            return;
        }
        if (here == Dune)
        {
            int mask = 0;
            for (int i = 0; i < 3; ++i)
                for (int j = 0; j < 3; ++j)
                    if (t(i + x*2, j + y*2) == Dune)
                        mask |= 1 << (i + j*3);

            // remove side if neighbor not dune
            if ((mask & (1+2+4)) && !isDuneTile(x, y - 1))
                mask &= ~(1+2+4);
            if ((mask & (4+32+256)) && !isDuneTile(x + 1, y))
                mask &= ~(4+32+256);
            if ((mask & (64+128+256)) && !isDuneTile(x, y + 1))
                mask &= ~(64+128+256);
            if ((mask & (1+8+64)) && !isDuneTile(x - 1, y))
                mask &= ~(1+8+64);

            // remove corner if neighbor by corner is not dune or bad corner
            if ((mask & 1) && (!isDuneTile(x - 1, y - 1) || !isDuneCorner(x*2, y*2)))
                mask ^= 1;
            if ((mask & 4) && (!isDuneTile(x + 1, y - 1) || !isDuneCorner(x*2 + 2, y*2)))
                mask ^= 4;
            if ((mask & 256) && (!isDuneTile(x + 1, y + 1) || !isDuneCorner(x*2 + 2, y*2 + 2)))
                mask ^= 256;
            if ((mask & 64) && (!isDuneTile(x - 1, y + 1) || !isDuneCorner(x*2, y*2 + 2)))
                mask ^= 64;

            if (!(mask & (1+64)))
                mask &= ~8;
            if (!(mask & (4+256)))
                mask &= ~32;

            switch(mask & make_dunemask(1,0,1,0,0,0,1,0,1))
            {
            case make_dunemask(0,0,0,0,0,0,0,0,0):
            case make_dunemask(0,0,0,0,0,0,1,0,0):
                break;

            case make_dunemask(1,0,1,0,0,0,0,0,0):
                mask &= ~32;
                // fall down !
            case make_dunemask(1,0,0,0,0,0,0,0,0):
                if (!(mask & 8) || x == 0)
                    break;
                if (t(x*2 - 2, y*2) == Dune
                 && isDuneTile(x - 2, y)
                 && isDuneTile(x - 2, y - 1)
                 && isDuneCorner(x*2 - 2, y*2))
                {
                    if (t(x*2 - 2, y*2 + 2) == Dune
                     && isDuneTile(x - 2, y)
                     && isDuneTile(x - 1, y + 1)
                     && isDuneTile(x - 2, y + 1)
                     && isDuneCorner(x*2 - 2, y*2 + 2))
                    {
                    }
                    else
                        mask &= ~8;
                }
                break;

            case make_dunemask(1,0,1,0,0,0,1,0,0):
                mask |= 8;
                // fall down !
            case make_dunemask(0,0,1,0,0,0,0,0,0):
                if (!(mask & 32) || !in(x + 1, y))
                    break;
                if (t(x*2 + 4, y*2) == Dune
                 && isDuneTile(x + 2, y)
                 && isDuneTile(x + 2, y - 1)
                 && isDuneCorner(x*2 + 4, y*2))
                {
                    if (t(x*2 + 4, y*2 + 2) == Dune
                     && isDuneTile(x + 2, y)
                     && isDuneTile(x + 1, y + 1)
                     && isDuneTile(x + 2, y + 1)
                     && isDuneCorner(x*2 + 4, y*2 + 2))
                        mask &= ~32;
                }
                break;

            case make_dunemask(1,0,0,0,0,0,1,0,1):
            case make_dunemask(0,0,0,0,0,0,1,0,1):
                if (mask & 1)
                    mask |= 8;
                else
                    mask &= ~8;
                // fall down !
            case make_dunemask(0,0,0,0,0,0,0,0,1):
                if (!(mask & 32) || !in(x + 1, y))
                    break;
                if (t(x*2 + 4, y*2 + 2) == Dune
                 && isDuneTile(x + 2, y)
                 && isDuneTile(x + 2, y + 1)
                 && isDuneCorner(x*2 + 4, y*2 + 2))
                {
                    if (t(x*2 + 4, y*2) == Dune
                     && isDuneTile(x + 2, y)
                     && isDuneTile(x + 1, y - 1)
                     && isDuneTile(x + 2, y - 1)
                     && isDuneCorner(x*2 + 4, y*2))
                    {
                    }
                    else
                        mask &= ~32;
                }
                break;

            case make_dunemask(1,0,0,0,0,0,1,0,0):
                if ((mask & 8) || x == 0)
                    break;
                if (t(x*2 - 2, y*2) == Dune
                 && isDuneTile(x - 2, y)
                 && isDuneTile(x - 2, y - 1)
                 && isDuneCorner(x*2 - 2, y*2))
                {
                    if (t(x*2 - 2, y*2 + 2) == Dune
                     && isDuneTile(x - 2, y)
                     && isDuneTile(x - 2, y + 1)
                     && isDuneCorner(x*2 + 2, y*2 + 2))
                    {
                    }
                    else
                        mask |= 8;
                }
                else
                    mask |= 8;
                break;

            case make_dunemask(0,0,1,0,0,0,0,0,1):
            case make_dunemask(0,0,1,0,0,0,1,0,1):
                mask |= 32;
                break;

            case make_dunemask(1,0,1,0,0,0,0,0,1):
                mask = make_dunemask(1,0,1,0,0,1,0,0,1);
                break;

            case make_dunemask(1,0,1,0,0,0,1,0,1):
                mask |= 8;
                if ((mask & 32) || !in(x + 1, y))
                    break;
                if (t(x*2 + 4, y*2 + 2) == Dune
                 && isDuneTile(x + 2, y)
                 && isDuneTile(x + 2, y + 1)
                 && isDuneCorner(x*2 + 4, y*2 + 2))
                {
                    mask |= 32;
                    break;
                }
                if (t(x*2 + 4, y*2) == Dune
                 && isDuneTile(x + 2, y)
                 && isDuneTile(x + 2, y - 1)
                 && isDuneCorner(x*2 + 4, y*2))
                    mask |= 32;
                break;
            }

            int k = 0;
            for (int i = 0; i < 256; ++i)
                if (dunemask[i] != -1 && (dunemask[i] & DUNE_MASK) == (mask & DUNE_MASK))
                    k = i;
            for (int i = 0; i < 256; ++i)
                if (dunemask[i] != -1 && (dunemask[i] & DUNE_MASKMID) == (mask & DUNE_MASKMID))
                    k = i;
            if (k == 0x9C && (mask & 2))
                k = 0x60;
            (*this)[x][y] = k;
        }
        else
        {
            int k = 0;
            int ok_mask, here_mask;
            if (here == Ground)
            {
                ok_mask = 1 << Ground;
                here_mask = ok_mask;
            }
            else
            {
                ok_mask = (1 << SpiceLow) | (1 << SpiceHigh);
                if (here == SpiceHigh)
                {
                    if ((!in(x, y - 1) || (1 << getTileType(x, y - 1)) & ok_mask)
                     && (!in(x + 1, y) || (1 << getTileType(x + 1, y)) & ok_mask)
                     && (!in(x, y + 1) || (1 << getTileType(x, y + 1)) & ok_mask)
                     && (!in(x - 1, y) || (1 << getTileType(x - 1, y)) & ok_mask))
                    {
                        here_mask = (1 << SpiceHigh);
                    }
                    else
                    {
                        here = SpiceLow;
                        here_mask = ok_mask;
                    }
                }
                else
                    here_mask = ok_mask;
            }
            if (here == SpiceHigh)
            {
                if (t(x*2 + 1, y*2)     == SpiceHigh && (!in(x, y - 1) || isHighSpice(x, y - 1)))
                    k++;
                if (t(x*2 + 2, y*2 + 1) == SpiceHigh && (!in(x + 1, y) || isHighSpice(x + 1, y)))
                    k |= 2;
                if (t(x*2 + 1, y*2 + 2) == SpiceHigh && (!in(x, y + 1) || isHighSpice(x, y + 1)))
                    k |= 4;
                if (t(x*2,     y*2 + 1) == SpiceHigh && (!in(x - 1, y) || isHighSpice(x - 1, y)))
                    k |= 8;
            }
            else
            {
                if (((1 << t(x*2 + 1, y*2))     & here_mask) && (!in(x, y - 1) || ((1 << getTileType(x, y - 1)) & ok_mask)))
                    k++;
                if (((1 << t(x*2 + 2, y*2 + 1)) & here_mask) && (!in(x + 1, y) || ((1 << getTileType(x + 1, y)) & ok_mask)))
                    k |= 2;
                if (((1 << t(x*2 + 1, y*2 + 2)) & here_mask) && (!in(x, y + 1) || ((1 << getTileType(x, y + 1)) & ok_mask)))
                    k |= 4;
                if (((1 << t(x*2,     y*2 + 1)) & here_mask) && (!in(x - 1, y) || ((1 << getTileType(x - 1, y)) & ok_mask)))
                    k |= 8;
            }
            if (k == 0 && ((1 << t(x*2 + 1, y*2 + 1)) & here_mask)) // point
                k = 16;
            else if (k == 3 && !((1 << t(x*2 + 1, y*2 + 1)) & here_mask))
                k = 17;
            else if (k == 6 && !((1 << t(x*2 + 1, y*2 + 1)) & here_mask))
                k = 18;
            else if (k == 12 && !((1 << t(x*2 + 1, y*2 + 1)) & here_mask))
                k = 20;
            else if (k == 9 && !((1 << t(x*2 + 1, y*2 + 1)) & here_mask))
                k = 19;

            int id = 0x7F;
            if (here == Ground)
            {
                if (k == 15)
                {
                    // avoid changing cliffs
                    int tmp = (*this)[x][y];
                    if (tmp >= 0xA0 && tmp <= 0xAF && tmp != 0xA8 && tmp != 0xAC || tmp == 0x64)
                        return;
                }
                id = 0x80 + k;
                if (k == 0)
                    id = 0x7F;
                if ( k == 16) // point
                    id = 0x80;
                if ( k == 17)
                    id = 0x3C;
                if ( k == 18)
                    id = 0x3D;
                if ( k == 20)
                    id = 0x3F;
                if ( k == 19)
                    id = 0x3E;
            }
            else if (here == SpiceLow)
            {
                id = 0xB0 + k;
                if (k == 0)
                    id = 0x7F;
                if (k == 16)
                    id = 0xB0;
                if (k == 17)
                    id = 0x40;
                if (k == 18)
                    id = 0x41;
                if (k == 20)
                    id = 0x43;
                if (k == 19)
                    id = 0x42;
            }
            else if (here == SpiceHigh)
            {
                id = 0xC0 + k;
                if (k == 0)
                    id = 0xBF;
                if (k == 16)
                    id = 0xC0;
                if (k == 17)
                    id = 0x44;
                if (k == 18)
                    id = 0x45;
                if (k == 20)
                    id = 0x47;
                if (k == 19)
                    id = 0x46;
            }
            (*this)[x][y] = id;
        }
    }

    // set mask based on tile id
    void SetTileMask(int x, int y)
    {
        int id = (*this)[x][y];
        int type = Dust;
        int mask = 0;
        int k = 0;
        if (id > 0x80 && id <= 0x8F)
        {
            type = Ground; k = id-0x80;
        }
        if (id == 0x80)
        {
            type = Ground; k = 16;
        }
        if (id == 0x3C)
        {
            type = Ground; k = 17;
        }
        if (id == 0x3D)
        {
            type = Ground; k = 18;
        }
        if (id == 0x3F)
        {
            type = Ground; k = 20;
        }
        if (id == 0x3E)
        {
            type = Ground; k = 19;
        }

        if (id > 0xB0 && id <= 0xBF)
        {
            type = SpiceLow; k = id-0xB0;
        }
        if (id == 0xB0)
        {
            type = SpiceLow; k = 16;
        }

        if (id == 0x40)
        {
            type = SpiceLow; k = 17;
        }
        if (id == 0x41)
        {
            type = SpiceLow; k = 18;
        }
        if (id == 0x43)
        {
            type = SpiceLow; k = 20;
        }
        if (id == 0x42)
        {
            type = SpiceLow; k = 19;
        }

        if (id > 0xC0 && id <= 0xCF)
        {
            type = SpiceHigh; k = id-0xC0;
        }
        if (id == 0xC0)
        {
            type = SpiceHigh; k = 16;
        }

        if (id == 0x44)
        {
            type = SpiceHigh; k = 17;
        }
        if (id == 0x45)
        {
            type = SpiceHigh; k = 18;
        }
        if (id == 0x47)
        {
            type = SpiceHigh; k = 20;
        }
        if (id == 0x46)
        {
            type = SpiceHigh; k = 19;
        }
        if (id == 0x64 || id >= 0xA0 && id <= 0xAF && id != 0xA8 && id != 0xAC)
        {
            type = Ground; k = 15;
        }

        if (type != Dust)
        {
            if (k<16)
            {
                mask = 0;
                if (k & 1)
                    mask |= 2;
                if (k & 2)
                    mask |= 32;
                if (k & 4)
                    mask |= 128;
                if (k & 8)
                    mask |= 8;
                if ((k & (1+2)) == 1+2)
                    mask |= 4;
                if ((k & (2+4)) == 2+4)
                    mask |= 256;
                if ((k & (4+8)) == 4+8)
                    mask |= 64;
                if ((k & (8+1)) == 8+1)
                    mask |= 1;
                if (k != 1
                 && k != 2
                 && k != 4
                 && k != 8)
                    mask |= 16;
            }
            else
            {
                if (k == 16)
                    mask = 16;
                if (k == 17)
                    mask = make_dunemask(0,1,1,0,0,1,0,0,0);
                if (k == 18)
                    mask = make_dunemask(0,0,0,0,0,1,0,1,1);
                if (k == 20)
                    mask = make_dunemask(0,0,0,1,0,0,1,1,0);
                if (k == 19)
                    mask = make_dunemask(1,1,0,1,0,0,0,0,0);
            }
        }

        if (dunemask[id] != -1)
        {
            mask = dunemask[id];
            type = Dune;
        }
        int a = type;
        int b = Dust;
        if (type == SpiceHigh)
            b = SpiceLow;
        for (int i=0; i<3; ++i)
            for (int j=0; j<3; ++j)
                if (mask & (1<<(i+j*3)))
                {
                    if (!(type == SpiceLow && t(i+x*2,j+y*2) == SpiceHigh))
                        t(i+x*2,j+y*2) = a;
                }
                else
                {
                    if (b == SpiceLow && t(i+x*2,j+y*2) != SpiceLow)
                        t(i+x*2,j+y*2) = b;
                }
    }

    unsigned char *operator [](int x)
    {
        return data+x*height;
    }

    unsigned char & t(int x,int y)
    {
        return types[y*twidth()+x];
    }
};

#endif // DUNEGROUND_H
