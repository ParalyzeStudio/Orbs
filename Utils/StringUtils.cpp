#include "OrbsPluginPCH.h"
#include "StringUtils.h"

StringsXmlHelper StringsXmlHelper::instance;

StringsXmlHelper::StringsXmlHelper()
{
	 
}

StringsXmlHelper::~StringsXmlHelper()
{

}

void StringsXmlHelper::preloadStringsFiles()
{
	if (!stringsFilesLoaded)
	{
		string stringsFile = GetStringsFolder() + string("strings.xml");
		string creditsFile = GetStringsFolder() + string("credits.xml");
		stringsDoc.LoadFile(stringsFile.c_str());
		creditsDoc.LoadFile(creditsFile.c_str());
		ResourcePreloader::preloadStringsFiles();
	}
}

string StringsXmlHelper::getStringForTag(const char* tag)
{
	 for (TiXmlElement *pNode = stringsDoc.RootElement()->FirstChildElement("string"); pNode; pNode=pNode->NextSiblingElement("string"))
	 {
		 const char *szTag = XMLHelper::Exchange_String(pNode,"tag",NULL,false);
		 if (!strcmp(szTag, tag))
		 {
			 return XMLHelper::Exchange_String(pNode,"text",NULL,false);
		 }
	 }

	 return "";
}

int StringsXmlHelper::getObjectivesCountForLevel(int level)
{
	int objectivesCount = 0;
	string levelNameString = (string("level") + stringFromInt(level));
	const char* levelName = levelNameString.c_str();
	for (TiXmlElement *pNode = stringsDoc.RootElement()->FirstChildElement("objectives_count"); pNode; pNode=pNode->NextSiblingElement("objectives_count"))
	{
		const char *szTag = XMLHelper::Exchange_String(pNode,"tag",NULL,false);
		if (!strcmp(szTag, levelName))
		{
			XMLHelper::Exchange_Int(pNode, "count", objectivesCount, false);
			break;
		}
	}

	return objectivesCount;
}

vector<string> StringsXmlHelper::getObjectivesStringsForLevel(int level)
{
	int objectivesCount = getObjectivesCountForLevel(level);
	vector<string> objectives (objectivesCount, "");
	for (int objectiveIndex = 0; objectiveIndex != objectivesCount; objectiveIndex++)
	{
		string tagName = string("level") + stringFromInt(level) + string("_objective") + stringFromInt(objectiveIndex + 1);
		string objectiveString = getStringForTag(tagName.c_str());
		objectives[objectiveIndex] = objectiveString;
	}

	return objectives;
}


int StringsXmlHelper::splitStringAndCreateLines(string &stringWithLineSeparators, string str, int maxCharByLine)
{
	int numberOfLines = 1;
    istringstream iss(str);
	int lineCharacterCount = 0;
	istream_iterator<string> it;
    for (it = istream_iterator<string>(iss); it != istream_iterator<string>(); ++it)
	{
        string word = *it;
		if (word.size() == 0) //no more words
			break;
		if (lineCharacterCount == 0) //first word of the line
		{
			stringWithLineSeparators += word;
			lineCharacterCount += word.size();
		}
		else
		{
			lineCharacterCount += 1; //add a space
			lineCharacterCount += word.size(); //add the number of characters contained in this word
			if (lineCharacterCount <= maxCharByLine) //add the word on this line
				stringWithLineSeparators = stringWithLineSeparators + string(" ") + word;
			else //create a new line
			{
				numberOfLines++;
				stringWithLineSeparators += string(" \n"); //line separator
				stringWithLineSeparators += word; //add the word
				lineCharacterCount = word.size();
				continue;
			}
		}
	}
	return numberOfLines;
}

int StringsXmlHelper::splitStringInArray(vector<string> &stringsArray, string str, int maxCharByLine)
{
	int numberOfLines = 1;
    istringstream iss(str);
	int lineCharacterCount = 0;
	istream_iterator<string> it;
	string currentString = "";
    for (it = istream_iterator<string>(iss); it != istream_iterator<string>(); ++it)
	{
        string word = *it;
		if (word.size() == 0) //no more words
			break;
		if (lineCharacterCount == 0) //first word of the line
		{
			currentString += word;
			lineCharacterCount += word.size();
		}
		else
		{
			lineCharacterCount += 1; //add a space
			lineCharacterCount += word.size(); //add the number of characters contained in this word
			if (lineCharacterCount <= maxCharByLine) //add the word on this line
				currentString = currentString + string(" ") + word;
			else //create a new line
			{
				stringsArray.push_back(currentString);
				currentString = "";
				numberOfLines++;
				currentString += word; //add the word
				lineCharacterCount = word.size();
				continue;
			}
		}
	}
	stringsArray.push_back(currentString);
	return numberOfLines;
}

vector<CreditsLine> StringsXmlHelper::getCredits()
{
	vector<CreditsLine> creditsStrings;

	for (TiXmlElement *pNode = creditsDoc.RootElement()->FirstChildElement("string"); pNode; pNode=pNode->NextSiblingElement("string"))
	{
		CreditsLine creditsLine;
		creditsLine.type = string(XMLHelper::Exchange_String(pNode,"type",NULL,false));
		if (!creditsLine.type.compare(CREDITS_LINE_TYPE_IMG)) //img
		{
			creditsLine.path = string(XMLHelper::Exchange_String(pNode,"path",NULL,false));
			XMLHelper::Exchange_Float(pNode, "width", creditsLine.width, false);
			XMLHelper::Exchange_Float(pNode, "height", creditsLine.height, false);
		}
		else if(!creditsLine.type.compare(CREDITS_LINE_TYPE_TEXT)) //text
		{
			creditsLine.text = string(XMLHelper::Exchange_String(pNode,"text",NULL,false));
			XMLHelper::Exchange_Int(pNode, "dropLines", creditsLine.dropLines, false);
			const char* fontName = XMLHelper::Exchange_String(pNode, "font", NULL, false);
			creditsLine.fontID = getFontIDForFontName(fontName);
			XMLHelper::Exchange_Float(pNode, "fontSize", creditsLine.fontSize, false);
			XMLHelper::Exchange_Color(pNode, "color", creditsLine.color, false);
		}

		creditsStrings.push_back(creditsLine);
	}

	return creditsStrings;
}