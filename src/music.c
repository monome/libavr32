#include "types.h"

// equal temp tuning table for 1v/oct on 14bit dac with 10v range
// for i in range(0,128):
//     print '%.f, ' % (i * 16384.0 / 120.0)
const uint16_t ET[128] = {
	0,     137,   273,   410,   546,   683,   819,   956,   1092,  1229,  1365,  1502,
	1638,  1775,  1911,  2048,  2185,  2321,  2458,  2594,  2731,  2867,  3004,  3140,
	3277,  3413,  3550,  3686,  3823,  3959,  4096,  4233,  4369,  4506,  4642,  4779,
	4915,  5052,  5188,  5325,  5461,  5598,  5734,  5871,  6007,  6144,  6281,  6417,
	6554,  6690,  6827,  6963,  7100,  7236,  7373,  7509,  7646,  7782,  7919,  8055,
	8192,  8329,  8465,  8602,  8738,  8875,  9011,  9148,  9284,  9421,  9557,  9694,
	9830,  9967,  10103, 10240, 10377, 10513, 10650, 10786, 10923, 11059, 11196, 11332,
	11469, 11605, 11742, 11878, 12015, 12151, 12288, 12425, 12561, 12698, 12834, 12971,
	13107, 13244, 13380, 13517, 13653, 13790, 13926, 14063, 14199, 14336, 14473, 14609,
	14746, 14882, 15019, 15155, 15292, 15428, 15565, 15701, 15838, 15974, 16111, 16247,
	16384, 16521, 16657, 16794, 16930, 17067, 17203, 17340,
};

// scale mode intervals
const uint8_t SCALE_INT[7][7] = {
	{2, 2, 1, 2, 2, 2, 1},	// ionian
	{2, 1, 2, 2, 2, 1, 2},	// dorian
	{1, 2, 2, 2, 1, 2, 2},	// phyrgian
	{2, 2, 2, 1, 2, 2, 1},	// lydian
	{2, 2, 1, 2, 2, 1, 2},	// mixolydian
	{2, 1, 2, 2, 1, 2, 2},	// aeolian
	{1, 2, 2, 1, 2, 2, 2},	// locrian
};
