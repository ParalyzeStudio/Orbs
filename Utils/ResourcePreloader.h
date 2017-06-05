#pragma once

class ResourcePreloader : public VBaseObject
{
public:
	ResourcePreloader(void) : stringsFilesLoaded(false) {};
	~ResourcePreloader(void) {};

	virtual void preloadStringsFiles(){this->stringsFilesLoaded = true;};

	static void preloadMenusTextures();
	static void preloadGameTextures();

protected:
	bool stringsFilesLoaded;
};

