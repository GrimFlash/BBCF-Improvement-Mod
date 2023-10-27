#pragma once
#include <stdint.h>
class ReplayFile
{
public:
	//metadata
	char pad_0000[0x38]; //0x00
	char date1[0x18]; //0x38
	char pad_0050[0x28]; //0x50
	char date2[0x18]; //0x78
	char pad_0090[0x14]; //0x90
	//char p1_name[0x24]; //0xa4 its in utf-16
	wchar_t p1_name[0x12];
	char pad_0xc8[0x9E]; //0xc8
	char Y4[0x8]; //0x167
	//char p2_name[0x24]; //0x16E its in utf-16
	wchar_t p2_name[0x12];
	char pad_192[0x9e]; //0x192
	uint32_t p1_toon;
	uint32_t p2_toon;
	char Y4_2[0x8]; //0x238
	char recorder_name[0x24]; //0x240
	//padding until replay data
	//replay data
	char pad_all[0xFDC0]; 
};