#pragma once


#include <sstream>
#include <iterator>

#define CREDITS_LINE_TYPE_TEXT "text"
#define CREDITS_LINE_TYPE_IMG "img"

struct CreditsLine
{
	string type;

	//text related fields
	string text;
	int dropLines;
	int fontID;
	float fontSize;
	VColorRef color;

	//img related fields
	string path;
	float width;
	float height;
};

inline string stringFromInt(int number)
{
	stringstream st;
	st << number;
	return st.str(); 
};

inline string stringFromFloat(float number)
{
	stringstream st;
	st << number;
	return st.str(); 
};

inline void GetLocale(VString& locale)
{
#if defined(_VISION_ANDROID)
	AConfiguration* appConfig=AndroidApplication->config;
	char language[2], country[2];
	AConfiguration_getLanguage(appConfig, language);
	AConfiguration_getCountry(appConfig, country);
	locale.Format("%c%c_%c%c",language[0],language[1],country[0],country[1]);
#else
	locale = "en_US";				//non Android failsafe, give locale as default value
#endif


	//tmp
	locale = "fr_FR";
};

inline void GetLanguage(VString& language)
{
	VString locale;
	GetLocale(locale);
	if (locale.GetLen() > 1)
	{
		language.Left(locale, 2);
	}
	else
		language = "en";
}

inline string GetStringsFolder()
{
	VString language;
	GetLanguage(language);
	if (language.operator==("fr"))
		return "Strings\\fr\\";
	else
		return "Strings\\en\\";
};

inline int getFontIDForFontName(const char* fontName)
{
	string fontNameStr = string(fontName);
	if (!fontNameStr.compare("FONT_NEUROPOL"))
		return FONT_NEUROPOL;
	else if (!fontNameStr.compare("FONT_COURIER"))
		return FONT_COURIER;
	else
		return FONT_NEUROPOL;
};

class StringsXmlHelper : public ResourcePreloader
{
public:
	~StringsXmlHelper();
	static StringsXmlHelper& sharedInstance() {return instance;};
	void preloadStringsFiles();

	string getStringForTag(const char* tag);
	int getObjectivesCountForLevel(int level);
	vector<string> getObjectivesStringsForLevel(int level);
	int splitStringAndCreateLines(string &stringWithLineSeparators, string str, int maxCharByLine);
	int splitStringInArray(vector<string> &stringsArray, string str, int maxCharByLine);

	//credits
	vector<CreditsLine> getCredits();

private:
	static StringsXmlHelper instance;
	StringsXmlHelper();

	TiXmlDocument stringsDoc;
	TiXmlDocument creditsDoc;
};