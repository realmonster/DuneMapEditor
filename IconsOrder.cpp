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

#include "IconsOrder.h"

int StructuresOrder[] = {
 0, // platform x1
 1, // platform x4
 8, // CY
 9, // Windtrap
12, // Refinery
7,  // Barracks
10, // Barracks
18, // Radar
17, // Spice Silo
 3, // Venchile
 4, // Venchile
14, // Wall
15, // Turret
16, // R-Turret
13, // Repair
 5, // Hi-Tech
11, // Starport
 2, // Palace
-1, // end of list
};

int UnitsOrder[] = {
 4, // Solder
 2, // Infantry
 5, // Trooper
 3, // Troopers
13, // Trike
14, // Raider Trike
15, // Quad
16, // Harvester
17, // MCV
 9, // Tank
10, // Siege Tank
 7, // Launcher
 8, // Deviator
12, // Sonic
11, // Devastator
 6, // ?
 0, // Carryall
 1, // Thopter
25, // Sandworm
-1, // end of list
};

char* GroundName[] = {
    "Dust",
    "Ground",
    "SpiceLow",
    "SpiceHigh",
    "Dune",
    0
};

char *HouseName[] = {
    "Harkonnen",
    "Atreides",
    "Ordos",
    "Fremen",
    "Sadaukar",
    0
};
