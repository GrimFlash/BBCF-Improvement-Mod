#pragma once

class hitbox {
public:
	enum class JonbEntryType_ : int
	{
		hurt = 0,
		hit
	};

	JonbEntryType_ type;
	float offsetX;
	float offsetY;
	float width;
	float height;
};