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

#ifndef _DUNES_H_
#define _DUNES_H_

int dunemask[256];

#define make_dunemask(a,b,c,d,e,f,g,h,i) ((a)*1 +(b)*2  +(c)*4+ (d)*8 +(e)*16 +(f)*32+  (g)*64+(h)*128+(i)*256)
#define DUNE_MASK     (make_dunemask(1,0,1,1,0,1,1,0,1))
#define DUNE_MASKMID  (make_dunemask(1,0,1,1,1,1,1,0,1))

int DunemaskDist(int a,int b)
{
	a = a^b;
	int r = 0;
	for (int i=0; i<9; ++i)
		if (a&(1<<i))
			++r;
	return r;
}

void DunemaskInit()
{
	memset(dunemask,-1,sizeof(dunemask));
	dunemask[0x7F]=make_dunemask(0,0,0,0,0,0,0,0,0);
	dunemask[0x50]=make_dunemask(1,0,1,1,0,0,0,0,0);
	dunemask[0x51]=make_dunemask(0,0,1,0,0,1,1,0,1);
	dunemask[0x52]=make_dunemask(0,0,1,0,1,0,0,0,0);
	dunemask[0x53]=make_dunemask(1,0,1,1,0,1,1,0,0);
	dunemask[0x54]=make_dunemask(1,0,1,1,0,0,1,0,0);
	dunemask[0x55]=make_dunemask(1,0,1,0,1,0,0,0,0);
	dunemask[0x56]=make_dunemask(0,0,0,0,1,0,0,0,0);
	dunemask[0x57]=make_dunemask(1,0,0,0,1,0,0,0,0);
	dunemask[0x58]=make_dunemask(0,0,1,0,0,0,0,0,0);
	dunemask[0x59]=make_dunemask(1,0,1,0,0,0,0,0,0);
	dunemask[0x5A]=make_dunemask(0,0,0,0,0,1,1,0,1);
	dunemask[0x5B]=make_dunemask(1,0,1,1,0,0,1,0,1);
	dunemask[0x5C]=make_dunemask(1,0,0,1,0,1,1,0,1);
	dunemask[0x5D]=make_dunemask(0,0,0,0,1,0,1,0,0);
	dunemask[0x5E]=make_dunemask(0,0,0,0,1,0,1,0,1);
	dunemask[0x5F]=make_dunemask(0,0,0,0,1,0,0,0,1);
	dunemask[0x60]=make_dunemask(1,0,1,1,1,0,0,0,0);
	dunemask[0x61]=make_dunemask(0,0,1,0,1,1,0,0,1);
	dunemask[0x62]=make_dunemask(0,0,0,0,1,1,1,0,1);
	dunemask[0x63]=make_dunemask(1,0,0,0,0,0,1,0,0);
	dunemask[0x65]=make_dunemask(0,0,1,0,0,1,0,0,0);
	dunemask[0x66]=make_dunemask(0,0,0,0,1,0,0,0,0);
	dunemask[0x90]=make_dunemask(0,0,0,0,0,1,0,0,1);
	dunemask[0x91]=make_dunemask(0,0,1,0,0,1,0,0,1);
	dunemask[0x92]=make_dunemask(1,0,1,0,0,1,0,0,1);
	dunemask[0x93]=make_dunemask(0,0,0,0,0,0,0,0,1);
	dunemask[0x94]=make_dunemask(0,0,0,0,0,0,1,0,0);
	dunemask[0x95]=make_dunemask(0,0,0,0,0,0,1,0,1);
	dunemask[0x96]=make_dunemask(0,0,1,1,0,1,1,0,1);
	dunemask[0x97]=make_dunemask(1,0,0,1,0,0,1,0,0);
	dunemask[0x98]=make_dunemask(1,0,0,1,0,0,0,0,0);
	dunemask[0x99]=make_dunemask(1,0,0,0,0,0,0,0,0);
	dunemask[0x9A]=make_dunemask(1,0,1,1,1,1,1,0,0);
	dunemask[0x9B]=make_dunemask(1,0,0,1,1,0,0,0,0);
	dunemask[0x9C]=make_dunemask(1,0,1,1,1,0,0,0,0);
	dunemask[0x9D]=make_dunemask(1,0,0,1,0,0,1,0,1);
	dunemask[0x9E]=make_dunemask(1,0,1,1,0,1,1,0,1);
	dunemask[0x9F]=make_dunemask(1,1,1,1,1,1,1,1,1);
	dunemask[0xA8]=make_dunemask(0,0,0,1,0,0,1,0,0);
	dunemask[0xAC]=make_dunemask(1,0,0,0,1,0,1,0,0);
}

#endif // _DUNES_H_
