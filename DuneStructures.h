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

#ifndef _DUNES_STRUCTURES_H_
#define _DUNES_STRUCTURES_H_

struct DrawInfos
{
    int x,y,width,height,colored;
} StructureDrawInfos[] = {
// 0: Concrete
{448,0,32,32,1},
// 1: Concrete 2x2
{448,0,64,64,1},
// 2: Palace
{0,0,96,96},
// 3: Vehicle
{96,0,96,64},
// 4: Vehicle
{96,0,96,64},
// 5: Hi-Tech
{192,0,64,64},
// 6: Barracks
{256,0,64,64},
// 7: Barracks
{256,0,64,64},
// 8: Construction Yard
{320,0,64,64},
// 9: Wind Trap
{384,0,64,64},
// A: Barracks
{256,0,64,64},
// B: Starport
{0,96,96,96},
// C: Refinery
{96,64,96,64},
// D: Repair
{192,64,96,64},
// E: Wall
{128,160,32,32,1},
// F: Turret
{224,128,32,32,1},
// 10: Rocket Turret
{256,128,32,32,1},
// 11: Silos
{384,64,64,64},
// 12: Outpost
{448,64,64,64}
};

#endif // _DUNES_STRUCTURES_H_
