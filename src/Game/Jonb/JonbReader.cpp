#include "JonbReader.h"

std::vector<hitbox> JonbReader::getJonbEntries(const CharData * charObj)
{
	std::vector<hitbox> jonbEntries;

	const int entriesCount = charObj->hurtboxCount + charObj->hitboxCount;
	hitbox* pEntry = charObj->pJonbEntryBegin;

	for (int i = 0; i < entriesCount; i++)
	{
		jonbEntries.push_back(*pEntry);
		pEntry++;
	}

	return jonbEntries;
}
