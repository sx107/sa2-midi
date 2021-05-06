/*
 * sa2_lut.h
 *
 *  Created on: 24 мар. 2021 г.
 *      Author: Sx107
 */

#ifndef SA2_LUT_H_
#define SA2_LUT_H_

// Fulltone pitchbend LUT is provided for a 15-bit 2-fulltone pitchbend, because modulation also changes the pitch.
// And I don't want to truncate the modulation

// To compute the LUT:
// lut[LUT_INT(x)] + LUT_POS(x) * (lut[LUT_INT(x) + 1] - lut[LUT_INT(x)]) / LUT_INTLEN

// Original multisynth divisor is 16
// Pitchbend divisor is D = 16 * (2^(1/12))^((pb-0.5) * 4), where pb is pitchbend from 0 to 1
// This LUT represents value D-12 as a fixed point number, first 3 bits are integer, the rest is 13-bit fractional
// LUT = (D-12) * 65536 / 8 - 5728, 5728 is subtracted since otherwise the number won't fit in 16 bits
// The number of LUT bits is log2(LUT_LENGTH - 1)

// LUT is inverted, because max pitchbend = min multisynth divisor. It complicates a bit the binary calculations, but oh well
// const uint16_t _sa2_pb_lut_fulltone[] = {61108, 51840, 43091, 34834, 27040, 19683, 12740, 6186, 0}; // 3-bit LUT, 9 values

// Considering that most MIDI devices output only 7-bit pitchbend, a 7-bit LUT will be more than enough
const uint16_t _sa2_pb_lut_fulltone[] = {	61108, 60513, 59920, 59329, 58741, 58154, 57570, 56987,
											56407, 55829, 55253, 54679, 54107, 53537, 52969, 52403,
											51840, 51278, 50718, 50161, 49605, 49051, 48500, 47950,
											47402, 46857, 46313, 45771, 45231, 44693, 44157, 43623,
											43091, 42561, 42033, 41507, 40982, 40459, 39939, 39420,
											38903, 38388, 37875, 37363, 36854, 36346, 35840, 35336,
											34834, 34334, 33835, 33338, 32843, 32350, 31858, 31369,
											30881, 30395, 29910, 29427, 28946, 28467, 27990, 27514,
											27040, 26568, 26097, 25628, 25161, 24695, 24231, 23769,
											23309, 22850, 22393, 21937, 21483, 21031, 20580, 20131,
											19683, 19238, 18793, 18351, 17910, 17470, 17033, 16596,
											16162, 15728, 15297, 14867, 14438, 14011, 13586, 13162,
											12740, 12319, 11900, 11482, 11066, 10651, 10238, 9826,
											9416,  9007,  8599,  8194,  7789,  7386,  6985,  6585,
											6186,  5789,  5393,  4999,  4606,  4214,  3824,  3436,
											3048, 2662,	  2278,  1895,  1513,  1133,  754,   376,   0};

// All calculations for converting a 15-bit number into a LUT interval and position in the LUT interval
#define SA2_FULLTONE_LUT_BITS 7																			// Number of LUT intervals in bits
#define SA2_FULLTONE_LUT_INT(X) (X >> (15 - SA2_FULLTONE_LUT_BITS)) 										// Returns the corresponding LUT interval
#define SA2_FULLTONE_LUT_INTLEN ((1 << (15 - SA2_FULLTONE_LUT_BITS)) - 1) 									// Max LUT interval position
#define SA2_FULLTONE_LUT_POS(X) (SA2_FULLTONE_LUT_INTLEN - (X & ((1 << (15 - SA2_FULLTONE_LUT_BITS))-1)))	// Returns the corresponding position in LUT interval, INVERTED!

#define SA2_FULLTONE_LUT_PRECISION 13 	// Number of fractional bits in LUT
#define SA2_FULLTONE_LUT_MINVAL 12		// Minimal integer value
#define SA2_FULLTONE_LUT_SUBTRACT 5728	// Subtracted value from LUT

// Octave pitchbend LUT
// D = 16 * (2^(1/12))^((pb-0.5) * 48), where pb is pitchbend from 0 to 1
// LUT = (D-4) * 65536 / 64
// Everything else - see above
// const uint16_t _sa2_pb_lut_octave[] = {61440, 42245, 28672, 19074, 12288, 7489, 4096, 1697, 0}; // 3-bit LUT, 9 values

// 7-bit lut because see above
const uint16_t _sa2_pb_lut_octave[] = {	61440, 60036, 58661, 57317, 56001, 54713, 53453, 52220,
										51013, 49832, 48677, 47546, 46439, 45356, 44297, 43260,
										42245, 41252, 40280, 39329, 38399, 37488, 36597, 35725,
										34872, 34037, 33220, 32420, 31638, 30872, 30123, 29390,
										28672, 27970, 27283, 26610, 25952, 25309, 24678, 24062,
										23458, 22868, 22290, 21725, 21172, 20630, 20100, 19582,
										19074, 18578, 18092, 17617, 17151, 16696, 16251, 15815,
										15388, 14970, 14562, 14162, 13771, 13388, 13013, 12647,
										12288, 11937, 11593, 11257, 10928, 10606, 10291, 9983,
										9681,  9386,  9097,  8814,  8538,  8267,  8002,  7743,
										7489,  7241,  6998,  6760,  6528,  6300,  6077,  5859,
										5646,  5437,  5233,  5033,  4837,  4646,  4459,  4275,
										4096,  3920,  3749,  3581,  3416,  3255,  3098,  2943,
										2793,  2645,  2501,  2359,  2221,  2086,  1953,  1823,
										1697,  1572,  1451,  1332,  1216,  1102,  991,   882,
										775,   671,   568,   469,   371,   275,   181,   90,    0};

#define SA2_OCTAVE_LUT_BITS 7
#define SA2_OCTAVE_LUT_INT(X) (X >> (15 - SA2_OCTAVE_LUT_BITS))
#define SA2_OCTAVE_LUT_INTLEN ((1 << (15 - SA2_OCTAVE_LUT_BITS)) - 1)
#define SA2_OCTAVE_LUT_POS(X) (SA2_OCTAVE_LUT_INTLEN - (X & ((1 << (15 - SA2_OCTAVE_LUT_BITS))-1)))

#define SA2_OCTAVE_LUT_PRECISION 10
#define SA2_OCTAVE_LUT_MINVAL 4
#define SA2_OCTAVE_LUT_SUBTRACT 0



// Full pitchbend LUT
// Attention! LUT input here is 14-bit, not 15-bit!
// D = 16 * (2^(1/12))^(p * 36), where pb is pitchbend from 0 to 1
// LUT = (D-4) * 65536 / 64
// Everything else - see above
// const uint16_t _sa2_pb_lut_full[] = {57344, 42343, 30776, 21856, 14978, 9675, 5585, 2432, 0}; // 3-bit LUT, 9 values

// 7-bit lut because see above
const uint16_t _sa2_pb_lut_full[] = {	57344, 56288, 55249, 54227, 53221, 52231, 51257, 50299,
										49357, 48430, 47517, 46619, 45736, 44867, 44012, 43171,
										42343, 41529, 40728, 39939, 39164, 38401, 37650, 36911,
										36184, 35469, 34766, 34073, 33392, 32722, 32063, 31414,
										30776, 30148, 29530, 28922, 28324, 27736, 27157, 26587,
										26027, 25475, 24933, 24399, 23874, 23357, 22849, 22349,
										21856, 21372, 20896, 20427, 19966, 19512, 19066, 18626,
										18194, 17769, 17351, 16939, 16534, 16136, 15744, 15358,
										14978, 14605, 14238, 13876, 13521, 13171, 12827, 12488,
										12155, 11827, 11504, 11187, 10874, 10567, 10265, 9968,
										9675,  9387,  9104,  8825,  8551,  8281,  8016,  7754,
										7497,  7245,  6996,  6751,  6510,  6273,  6040,  5811,
										5585,  5363,  5145,  4930,  4718,  4510,  4306,  4104,
										3906,  3711,  3519,  3331,  3145,  2962,  2783,  2606,
										2432,  2261,  2092,  1926,  1763,  1603,  1445,  1290,
										1137,  987,   839,   693,   550,   409,   271,   134,  0};

// Attention! LUT input here is 14-bit, not 15-bit!
#define SA2_FULL_LUT_BITS 7
#define SA2_FULL_LUT_INT(X) (X >> (14 - SA2_FULL_LUT_BITS))
#define SA2_FULL_LUT_INTLEN ((1 << (14 - SA2_FULL_LUT_BITS)) - 1)
#define SA2_FULL_LUT_POS(X) (SA2_FULL_LUT_INTLEN - (X & ((1 << (14 - SA2_FULL_LUT_BITS))-1)))

#define SA2_FULL_LUT_PRECISION 9
#define SA2_FULL_LUT_MINVAL 16
#define SA2_FULL_LUT_SUBTRACT 0



// LUT for the modulation rate
// LFO Rate (freq.) = (72MHz / 2^14 / 2) / Prescaler
// Frequency range: 0.1-12.8Hz (Approx., real one is 0.99-12.77)
// LUT value (prescaler) = (72*10^6 / 2^14 / 2) / (0.1 * 128 ^ (MR / 127)), where MR is the input 7-bit modrate
const uint16_t _sa2_modrate_lut[] = {	21973, 21149, 20356, 19593, 18859, 18152, 17471, 16817,
										16186, 15579, 14995, 14433, 13892, 13372, 12870, 12388,
										11924, 11477, 11046, 10632, 10234, 9850,  9481,  9126,
										8784,  8454,  8137,  7832,  7539,  7256,  6984,  6722,
										6470,  6228,  5994,  5770,  5553,  5345,  5145,  4952,
										4766,  4588,  4416,  4250,  4091,  3938,  3790,  3648,
										3511,  3380,  3253,  3131,  3014,  2901,  2792,  2687,
										2587,  2490,  2396,  2306,  2220,  2137,  2057,  1980,
										1905,  1834,  1765,  1699,  1635,  1574,  1515,  1458,
										1404,  1351,  1300,  1252,  1205,  1160,  1116,  1074,
										1034,  995,   958,   922,   887,   854,   822,   791,
										762,   733,   706,   679,   654,   629,   606,   583,
										561,   540,   520,   500,   482,   464,   446,   429,
										413,   398,   383,   369,   355,   341,   329,   316,
										304,   293,   282,   272,   261,   252,   242,   233,
										224,   216,   208,   200,   193,   185,   178,   172};


#endif /* SA2_LUT_H_ */
