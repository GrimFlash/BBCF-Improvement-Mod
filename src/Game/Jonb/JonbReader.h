#pragma once
#include "../CharData.h"
#include "JonbEntry.h"

#include <vector>

class JonbReader
{
public:
	static std::vector<hitbox> getJonbEntries(const CharData* charObj);
};
