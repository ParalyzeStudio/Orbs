#pragma once

#define FONT_NEUROPOL 1 
#define FONT_DRAKO_HEART 2
#define FONT_DIGICITY 3
#define FONT_COURIER 4

class FontManager
{
public:
	static void loadFonts();
	static VisFont_cl* getFontForNameAndSize(int fontID, float fontHeight);
};

