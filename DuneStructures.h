#ifndef _DUNES_STRUCTURES_H_
#define _DUNES_STRUCTURES_H_

struct DrawInfos
{
	int x,y,width,height;
} StructureDrawInfos[] = {
// 0: Concrete
{448,0,32,32},
// 1: Concrete 2x2
{448,0,64,64},
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
{288,64,32,32},
// F: Turret
{320,64,32,32},
// 10: Rocket Turret
{352,64,32,32},
// 11: Silos
{384,64,64,64},
// 12: Outpost
{448,64,64,64}
};

#endif // _DUNES_STRUCTURES_H_
