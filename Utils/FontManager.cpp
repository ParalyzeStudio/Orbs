#include "OrbsPluginPCH.h"
#include "FontManager.h"


void FontManager::loadFonts()
{
	Vision::Fonts.LoadFont("GUI\\Fonts\\NeuropolXFree15.fnt");
	Vision::Fonts.LoadFont("GUI\\Fonts\\NeuropolXFree30.fnt");
	Vision::Fonts.LoadFont("GUI\\Fonts\\NeuropolXFree50.fnt");
	Vision::Fonts.LoadFont("GUI\\Fonts\\Digicity15.fnt");
	Vision::Fonts.LoadFont("GUI\\Fonts\\Digicity30.fnt");
	Vision::Fonts.LoadFont("GUI\\Fonts\\Digicity50.fnt");
}

VisFont_cl* FontManager::getFontForNameAndSize(int fontID, float fontHeight)
{
	int fontSizes[3] = {15,30,50};
	if (fontID == FONT_NEUROPOL)
	{
		if (fontHeight <= 1.2f * fontSizes[0])
			return Vision::Fonts.FindFont("NeuropolX15");
		else if (fontHeight <= 1.2f * fontSizes[1])
			return Vision::Fonts.FindFont("NeuropolX30");
		else
			return Vision::Fonts.FindFont("NeuropolX50");
	}
	else if (fontID == FONT_DRAKO_HEART)
	{
		if (fontHeight <= 1.2f * fontSizes[0])
			return Vision::Fonts.FindFont("DrakoHeart15");
		else if (fontHeight <= 1.2f * fontSizes[1])
			return Vision::Fonts.FindFont("DrakoHeart30");
		else
			return Vision::Fonts.FindFont("DrakoHeart50");
	}
	else if (fontID == FONT_DIGICITY)
	{
		if (fontHeight <= 1.2f * fontSizes[0])
			return Vision::Fonts.FindFont("Digicity15");
		else if (fontHeight <= 1.2f * fontSizes[1])
			return Vision::Fonts.FindFont("Digicity30");
		else
			return Vision::Fonts.FindFont("Digicity50");
	}
	else if (fontID == FONT_COURIER)
	{
		return Vision::Fonts.FindFont("courier_new");
	}


	return NULL;
}