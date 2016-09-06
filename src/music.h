// equal temp tuning table for 1v/oct on 12bit dac with 10v range

// for i in range(0,120):
// print '%.f, ' % (i * 4092.0 / 120.0)
const uint16_t ET[120] = {
	0, 34, 68, 102, 136, 170, 205, 239, 273, 307, 341, 375, 409, 443, 477, 511, 545, 580, 614, 648, 682, 716, 750, 784, 818, 852, 886,
	920, 955, 989, 1023, 1057, 1091, 1125, 1159, 1193, 1227, 1261, 1295, 1330, 1364, 1398, 1432, 1466, 1500, 1534, 1568, 1602, 1636,
	1670, 1705, 1739, 1773, 1807, 1841, 1875, 1909, 1943, 1977, 2011, 2046, 2080, 2114, 2148, 2182, 2216, 2250, 2284, 2318, 2352,
	2386, 2421, 2455, 2489, 2523, 2557, 2591, 2625, 2659, 2693, 2727, 2761, 2796, 2830, 2864, 2898, 2932, 2966, 3000, 3034, 3068,
	3102, 3136, 3171, 3205, 3239, 3273, 3307, 3341, 3375, 3409, 3443, 3477, 3511, 3546, 3580, 3614, 3648, 3682, 3716, 3750, 3784,
	3818, 3852, 3886, 3921, 3955, 3989, 4023, 4057
};

// scale mode intervals
const u8 SCALE_INT[7][7] = {
	{2, 2, 1, 2, 2, 2, 1},	// ionian
	{2, 1, 2, 2, 2, 1, 2},	// dorian
	{1, 2, 2, 2, 1, 2, 2},	// phyrgian
	{2, 2, 2, 1, 2, 2, 1},	// lydian
	{2, 2, 1, 2, 2, 1, 2},	// mixolydian
 	{2, 1, 2, 2, 1, 2, 2},	// aeolian
	{1, 2, 2, 1, 2, 2, 2} 	// locrian
};