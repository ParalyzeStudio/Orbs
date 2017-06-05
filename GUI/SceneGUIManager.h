#pragma once

class Touchable;
class Clickable;
class Panel;
class SpriteScreenMask;
class ButtonScreenMask;
class PrintTextLabel;
class ScreenMaskPanel;
class ScrollingList;
class ItemList;
class CallFuncObject;
class OrbFountain;
class YesNoDialogPanel;
class PopupButton;

//super class that holds common GUI manager functions
class SceneGUIManager : public VBaseObject, public IVisCallbackHandler_cl
{
public:
	SceneGUIManager() : m_GUIContext(NULL), scale(0) {}

	virtual void initGUIContext();
	virtual void deInitGUIContext();

	inline hkvVec2 getScreenSize()
	{
		return hkvVec2(m_GUIContext->GetClientRect().GetSize().x, m_GUIContext->GetClientRect().GetSize().y);
	}

	virtual void defineScale() = 0;
	virtual bool clickContainedInGUI(hkvVec2 clickCoords) = 0; //has the click to be intercepted and swallowed by GUI
	virtual bool processPointerEvent(hkvVec2 touchLocation, int eventType){return false;};

	virtual vector<PrintTextLabel*> retrieveAllTextLabels() = 0;

	virtual void OnHandleCallback(IVisCallbackDataObject_cl *pData);
	virtual void update(float dt) {};

	VSmartPtr<VGUIMainContext> getGUIContext(){return this->m_GUIContext;};
	bool isContextActive(){if (!m_GUIContext) return false; return this->m_GUIContext->IsActive();};
	float getScale(){return this->scale;};

protected:
	VSmartPtr<VGUIMainContext> m_GUIContext;
	float scale;
};

class Anchored2DObject : public VTypedObject, public IVisCallbackHandler_cl
{
public:
	Anchored2DObject();
	~Anchored2DObject();

	V_DECLARE_DYNCREATE(Anchored2DObject);

	virtual void init(hkvVec2 position, float angle = 0, hkvVec2 scale = hkvVec2(1.0f, 1.0f), float opacity = 1.0f, hkvVec2 anchorPoint = hkvVec2(0.5f, 0.5f));
	
	virtual void SetScale(hkvVec2 scale);
	virtual void SetPosition(hkvVec2 position);
	virtual void SetAngle(float angle);
	virtual void SetOpacity(float opacity);
	virtual void IncScale(hkvVec2 &deltaScale);
	virtual void IncPosition(hkvVec2 deltaPosition);
	virtual void IncRotation(float deltaAngle);
	virtual void IncOpacity(float deltaOpacity);

	void updatePosScaleAndOpacityFromParent();

	hkvVec2 GetScale(){return this->currentScale;};
	hkvVec2 GetPosition(){return this->currentPosition;};
	virtual hkvVec2 GetAbsPosition();
	float GetAngle(){return this->currentAngle;};
	float GetOpacity(){return this->currentOpacity;};
	void SetAnchorPoint(hkvVec2 anchorPoint){this->anchorPoint = anchorPoint;};

	hkvVec2 scaleLinearVariation(float dt);
	float angleLinearVariation(float dt);
	hkvVec2 positionLinearVariation(float dt);
	float opacityLinearVariation(float dt);

	void scaleTo(hkvVec2 toScale, float duration, float delay);
	void rotateTo(float toAngle, float duration, float delay);
	void rotateBy(float byAngle);
	void translateTo(hkvVec2 toPosition, float duration, float delay);
	void fadeTo(float toOpacity, float duration, float delay);

	virtual void update(float dt);
	virtual void OnHandleCallback(IVisCallbackDataObject_cl *pData);

	//virtual bool isClickable(){return false;};

	hkvVec2 getAnchorPoint(){return this->anchorPoint;};

	ScreenMaskPanel* getParentPanel(){return this->parentPanel;};
	void setParentPanel(ScreenMaskPanel* parentPanel){this->parentPanel = parentPanel; if (this->parentPanel) this->updatePosScaleAndOpacityFromParent();};

	int m_iID; //the unique id of the screen mask if set, else 0

protected:
	hkvVec2 anchorPoint; //position of the anchor point inside the screen mask [0-1] * width, [0-1] * height
	bool scaling;
	bool translating;
	bool rotating;
	bool fading;

	hkvVec2 currentScale;
	hkvVec2 currentPosition;
	float currentAngle;
	float currentOpacity;

	hkvVec2 startScale;
	hkvVec2 startPosition;
	float startAngle;
	float startOpacity;
	hkvVec2 toScale;
	hkvVec2 toPosition;
	float toAngle;
	float toOpacity;
	
	float scaleDuration;
	float rotationDuration;
	float translationDuration;
	float fadeDuration;
	float scaleDelay;
	float rotationDelay;
	float translationDelay;
	float fadeDelay;
	float scaleElapsedTime;
	float rotationElapsedTime;
	float translationElapsedTime;
	float fadeElapsedTime;

	ScreenMaskPanel* parentPanel;
};

class Touchable
{
public:
	Touchable() : selected(false), lastTouchLocation(hkvVec2(0,0)), deltaTouchLocation(hkvVec2(0,0)) {};

	virtual bool containsTouch(hkvVec2 touchLocation) = 0;

	virtual bool trigger(hkvVec2 touchLocation, int eventType); //trigger the item (press, release or move)
	virtual void onPointerDown(hkvVec2 touchLocation);
	virtual bool onPointerMove(hkvVec2 touchLocation);
	virtual void onPointerUp(hkvVec2 touchLocation);
	
	virtual void select();
	virtual void deselect();

protected:
	bool selected;
	hkvVec2 lastTouchLocation;
	hkvVec2 deltaTouchLocation;
};

class Clickable : public Touchable
{
public:
	Clickable() : enabled(false) {};

	virtual bool trigger(hkvVec2 touchLocation, int eventType); //trigger the button (press, release or move)
	virtual void disable();
	virtual void enable();
	virtual void onPointerUp(hkvVec2 touchLocation);
	virtual void onClick() = 0;

protected:
	bool enabled;
};

class SpriteScreenMask : public VisScreenMask_cl, public Anchored2DObject, public Touchable
{
public:
	SpriteScreenMask();
	~SpriteScreenMask();

	V_DECLARE_DYNCREATE(SpriteScreenMask);

	virtual void init(hkvVec2 position, hkvVec2 unscaledTargetSize, VTextureObject* textureObj, hkvVec2 scale = hkvVec2(1.0f, 1.0f), float angle = 0, float opacity = 1.0f, hkvVec2 anchorPoint = hkvVec2(0.5f, 0.5f));
	virtual void init(hkvVec2 position, hkvVec2 unscaledTargetSize, hkvVec4 textureRange, VTextureObject* textureObj, bool repeatTexture, hkvVec2 scale = hkvVec2(1.0f, 1.0f), float angle = 0, float opacity = 1.0f, hkvVec2 anchorPoint = hkvVec2(0.5f, 0.5f));

	virtual void SetUnscaledTargetSize(hkvVec2 size);
	virtual void IncScale(hkvVec2 &deltaScale);
	virtual void IncPosition(hkvVec2 deltaPosition);
	virtual void IncRotation(float deltaAngle);
	virtual void IncOpacity(float deltaOpacity);

	void updateTexture(VTextureObject* newTexture);

	virtual bool containsTouch(hkvVec2 touchLocation);
	virtual bool trigger(hkvVec2 touchLocation, int eventType){return false;}; //sprites are not triggerable

	hkvVec2 GetUnscaledTargetSize(){return this->unscaledTargetSize;};
	void setFixedRotation(bool fixedRotation){this->fixedRotation = fixedRotation;};
	bool isFixedRotation(){return this->fixedRotation;};

protected:
	hkvVec2 unscaledTargetSize; //unscaled target size of the screenmask
	bool usedAsButton; //is this screenMask used as a button
	bool fixedRotation; //does this mask rotates (for instance when its parent panel rotates)
};

class RotatingSpriteScreenMask : public SpriteScreenMask
{
public:
	RotatingSpriteScreenMask() : SpriteScreenMask(), rotationSpeed(0) {};
	virtual void init(hkvVec2 position, hkvVec2 unscaledTargetSize, VTextureObject* textureObj, float rotationSpeed, hkvVec2 scale = hkvVec2(1.0f, 1.0f), float angle = 0);

	virtual void update(float dt);

private:
	float rotationSpeed; // circle's rotation speed in degrees/sec
};

class ButtonScreenMask : public SpriteScreenMask, public Clickable
{
public:
	ButtonScreenMask();

	V_DECLARE_DYNCREATE(ButtonScreenMask);

	virtual void init(hkvVec2 position, hkvVec2 unscaledTargetSize, VTextureObject* textureObj, hkvVec2 scale = hkvVec2(1.0f, 1.0f), float angle = 0, float opacity = 1.0f, VColorRef enabledColor = V_RGBA_WHITE, VColorRef disabledColor = V_RGBA_WHITE, VColorRef selectedColor = V_RGBA_WHITE);

	//bool trigger(hkvVec2 touchLocation, int eventType); //trigger the button (press, release or move)
	void disable();
	void enable();
	void select();
	void deselect();

	virtual bool trigger(hkvVec2 touchLocation, int eventType);
	virtual bool containsTouch(hkvVec2 touchLocation);
	//virtual bool isClickable(){return true;};
	virtual void onClick(){};

	void setEnabledColor(VColorRef enabledColor){this->enabledColor = enabledColor;};
	void setDisabledColor(VColorRef disabledColor){this->disabledColor = disabledColor;};
	void setSelectedColor(VColorRef selectedColor){this->selectedColor = selectedColor;};

private:
	VColorRef enabledColor;
	VColorRef disabledColor;
	VColorRef selectedColor;
};

class PrintTextLabel : public Anchored2DObject, public VRefCounter
{
public:
	PrintTextLabel();
	~PrintTextLabel();

	void init(hkvVec2 position, string text, VisFont_cl* font, float unscaledFontHeight, VColorRef iColor);

	virtual void IncScale(hkvVec2 &deltaScale);
	virtual void IncPosition(hkvVec2 deltaPosition);
	virtual void IncOpacity(float deltaOpacity);
	void invalidatePos();

	virtual void renderItself();
	void OnHandleCallback(IVisCallbackDataObject_cl *pData);

	string getText(){return this->text;};
	hkvVec2 getAbsPos(){return this->absPos;};
	VisFont_cl* getFont(){return this->font;};
	float getFontHeight(){return this->fontHeight;};
	VColorRef getFontColor(){return this->iColor;};
	hkvVec2 getSize(){return this->size;};
	void setText(string text);
	void setAbsPos(hkvVec2 pos);
	void setFont(VisFont_cl* font){this->font = font;};
	void setFontHeight(){this->fontHeight = fontHeight;};
	void setFontColor(VColorRef iColor){this->iColor = iColor;};
	void setRenderedBehindScreenMasks(bool renderedBehindScreenMasks){this->renderedBehindScreenMasks = renderedBehindScreenMasks;};
	bool isRenderedBehindScreenMasks(){return this->renderedBehindScreenMasks;};
	void setRenderedBehindVeils(bool renderedBehindVeils){this->renderedBehindVeils = renderedBehindVeils;};
	bool isRenderedBehindVeils(){return this->renderedBehindVeils;};

protected:
	string text;
	hkvVec2 absPos;
	hkvVec2 size;
	VisFont_cl* font;
	float unscaledFontHeight;
	float fontHeight;
	VColorRef iColor;

	bool renderedBehindScreenMasks;
	bool renderedBehindVeils;
};

class IncrementalNumberTextLabel : public PrintTextLabel
{
public:
	IncrementalNumberTextLabel();

	void init(hkvVec2 position, VisFont_cl* font, float unscaledFontHeight, VColorRef iColor, int startNumber, int endNumber, float speed);
	void startIncrementing();
	int getNumberAt(float time);

	virtual void update(float dt);

private:
	bool incrementing;
	int startNumber;
	int endNumber;
	float speed;

	float incrementingElapsedTime;
	float incrementingDuration;
};

class ScreenMaskPanel : public VRefCounter, public Anchored2DObject
{
public:
	ScreenMaskPanel() : Anchored2DObject() {}; //panel is always anchored in its center
	~ScreenMaskPanel();

	V_DECLARE_DYNCREATE(ScreenMaskPanel);

	void init(hkvVec2 position, hkvVec2 scale = hkvVec2(1.0f, 1.0f), float opacity = 1.0f);

	void addScreenMask(SpriteScreenMask* screenMask);
	void addTextLabel(PrintTextLabel* textLabel);
	void addPanel(ScreenMaskPanel* panel);
	void addList(ScrollingList* list);
	void addFountain(OrbFountain* fountain);
	void removeAllChildren();
	void removeScreenMask(SpriteScreenMask* screenMask);
	void removeTextLabel(PrintTextLabel* textLabel);
	void removePanel(ScreenMaskPanel* panel);
	void removeList(ScrollingList* list);
	void removeFountain(OrbFountain* fountain);

	Anchored2DObject* findElementById(const char* id);

	//virtual void SetScale(hkvVec2 scale);
	//virtual void SetAngle(float angle);
	//virtual void SetOpacity(float opacity);
	virtual void IncScale(hkvVec2 &deltaScale);
	virtual void IncChildsScale(hkvVec2 deltaPosition);
	virtual void IncPosition(hkvVec2 deltaPosition);
	virtual void IncChildsPosition(hkvVec2 deltaPosition);
	virtual void IncRotation(float deltaAngle);
	virtual void IncChildsRotation(float deltaAngle);
	virtual void IncOpacity(float deltaOpacity);
	virtual void IncChildsOpacity(float deltaOpacity);

	virtual void SetOrder(int order);

	void disable(); //disable all child buttons and set opactiy to a lower value
	void enable();

	vector<PrintTextLabel*> retrieveAllTextLabels();

	vector<SpriteScreenMask*>& getScreenMasks(){return this->childScreenMasks;};
	vector<PrintTextLabel*>& getTextLabels(){return this->textLabels;};
	vector<ScreenMaskPanel*>& getPanels(){return this->panels;};
	vector<ScrollingList*>& getLists(){return this->lists;};
	vector<OrbFountain*>& getFountains(){return this->fountains;};

protected:
	vector<SpriteScreenMask*> childScreenMasks;
	vector<PrintTextLabel*> textLabels;
	vector<ScreenMaskPanel*> panels;
	vector<ScrollingList*> lists;
	vector<OrbFountain*> fountains;
};


class ClickableScreenMaskPanel : public ScreenMaskPanel, public Clickable
{
public:
	ClickableScreenMaskPanel() : ScreenMaskPanel(), Clickable(), touchArea(hkvVec2(0,0)) {};

	V_DECLARE_DYNCREATE(ClickableScreenMaskPanel);

	void init(hkvVec2 position, hkvVec2 scale = hkvVec2(1.0f, 1.0f), float opacity = 1.0f);
	void init(hkvVec2 position, hkvVec2 touchArea, hkvVec2 scale = hkvVec2(1.0f, 1.0f), float opacity = 1.0f);

	virtual bool containsTouch(hkvVec2 touchLocation);
	virtual void disable();
	virtual void enable();

	virtual void onClick();

protected:
	hkvVec2 touchArea;
};

//VERTICAL scrolling list
class ScrollingList : public VRefCounter, public Anchored2DObject, public Touchable
{
public:
	ScrollingList() : Anchored2DObject(), 
		lastPage(0),
		currentPage(0),
		swiping(false),
		width(0), 
		height(0), 
		totalLength(0),
		pos(hkvVec2(0,0)),
		autoscroll(false),
		autoscrollSpeed(0) {};
	~ScrollingList();

	virtual void init(hkvVec2 position, float width, float height, bool autoscroll, float autoscrollSpeed);

	void addTextEntry(PrintTextLabel* textEntry, hkvVec2 offset);
	void addMaskEntry(SpriteScreenMask* maskEntry, hkvVec2 offset);
	void addPanelEntry(ScreenMaskPanel* panelEntry, hkvVec2 panelSize, int page, hkvVec2 offset);

	virtual void scroll(float dx) = 0;
	virtual void pageSwipe(bool toNextPage);
	virtual void onPageSwipe(int currentPage){};
	void stopSwiping();
	virtual void IncChildsPosition(hkvVec2 deltaPos);

	void renderPrintTextLabels();
	virtual void IncPosition(hkvVec2 deltaPosition);
	virtual void IncOpacity(float deltaOpacity);
	virtual void adjustOpacity(PrintTextLabel* label) {};
	virtual void adjustOpacity(SpriteScreenMask* mask) {};
	virtual void adjustOpacity(ScreenMaskPanel* panel) {};
	virtual void adjustScale(PrintTextLabel* label) {};
	virtual void adjustScale(SpriteScreenMask* mask) {};
	virtual void adjustScale(ScreenMaskPanel* panel) {};

	bool containsTouch(hkvVec2 touchLocation);
	virtual bool onPointerMove(hkvVec2 touchLocation);

	virtual void update(float dt);

	hkvVec2 getSize(){return hkvVec2(width, height);};
	hkvVec2 getPos(){return this->pos;};

	vector<ScreenMaskPanel*> getPanels(){return this->panels;};

protected:
	vector<PrintTextLabel*> texts;
	vector<SpriteScreenMask*> masks;
	vector<ScreenMaskPanel*> panels; //each item is a group of masks, text labels (i.e a ScreenMaskPanel)

	hkvVec2 pos; //position of the top left hand corner
	bool autoscroll; //is the list auto-scrolling
	float autoscrollSpeed;
	float width; //width of the scrolling list
	float height; //height of the scrolling list (clipping rect)
	float totalLength;
	bool swiping;
	int lastPage;
	int currentPage;
};

#define DEFAULT_STABILIZING_SPEED 200.0f
//List that scrolls item by item
class ItemList : public ScrollingList
{
public:
	ItemList() : ScrollingList(),
		stabilizing(false),
		itemSize(hkvVec2(0,0)),
		stabilizeStartPosition(hkvVec2(0,0)),
		stabilizingTimer(0),
		stabilizingSpeed(0),
		currentItem(NULL),
		currentItemIndex(0)
	{};

	void init(hkvVec2 position, float width, float height, hkvVec2 itemSize);

	virtual void scroll(float dx);
	void scrollUpOneElement();
	void scrollDownOneElement();

	void findSelectedItem();
	void stabilizeList(bool performFindSelectedItem); //stabilize list to the closest element when touch is released

	virtual void onPointerDown(hkvVec2 touchLocation);
	virtual void onPointerUp(hkvVec2 touchLocation);
	virtual bool onPointerMove(hkvVec2 touchLocation);
	virtual void onItemSelected() {};

	virtual void update(float dt);

	ScreenMaskPanel* getCurrentItem(){return this->currentItem;};
	int getCurrentItemIndex(){return this->currentItemIndex;};

protected:
	bool stabilizing;
	hkvVec2 itemSize; //size of an item
	hkvVec2 stabilizeStartPosition;
	float stabilizingTimer;
	float stabilizingSpeed;

	ScreenMaskPanel* currentItem; //item in the center of the list
	int currentItemIndex;
};


class ParticleEffectContainer : public VRefCounter
{
public:
	ParticleEffectContainer();
	~ParticleEffectContainer();

	void init(VisParticleEffectFile_cl* effectFile, hkvVec3 position, bool billboarding = false, hkvMat3 rotationMatrix = hkvMat3(), float scale = 1.0f);

protected:
	VSmartPtr<VisParticleEffect_cl> particleEffect;
	hkvVec3 position;
	bool billboarding;
	hkvMat3 rotationMatrix;
	float scale;
};

class Veil : public SpriteScreenMask
{
public:
	Veil() : SpriteScreenMask() {};

	void init(VColorRef color, float opacity);
	//virtual void SetOpacity(float opacity);
	virtual void IncOpacity(float deltaOpacity);
};

#define DEFAULT_VEIL_FADE_DURATION 0.3f
#define DEFAULT_VEIL_PEAK_DURATION 0.2f

class TransitionVeil : public Veil
{
public:
	TransitionVeil() : Veil(),
	action(NULL),
	peakDuration(0),
	fadeInDuration(0),
	fadeOutDuration(0),
	running(false),
	veilElapsedTime(0){};

	~TransitionVeil(){hkvLog::Error("destroy TransitionVeil");};

	void init(VColorRef color, CallFuncObject* action, float peakDuration = DEFAULT_VEIL_PEAK_DURATION, float fadeInDuration = DEFAULT_VEIL_FADE_DURATION, float fadeOutDuration = DEFAULT_VEIL_FADE_DURATION);
	void launch(){this->running = true;};
	void update(float dt);

private:
	CallFuncObject* action;
	float peakDuration;
	float fadeInDuration;
	float fadeOutDuration;

	bool running;
	float veilElapsedTime;
};

#define ID_YES_BTN "ID_YES_BTN"
#define ID_NO_BTN "ID_NO_BTN"

class YesNoDialogPanel : public ScreenMaskPanel
{
public:
	void init(string text);

	virtual void onClickYes() {};
	virtual void onClickNo() {};
};

class YesNoButton : public ButtonScreenMask
{
public:
	YesNoButton() : parentDialogPanel(NULL){};

	void init(YesNoDialogPanel* parentPopup, int id);

	virtual void onClick();

private:
	YesNoDialogPanel* parentDialogPanel;
};

class Slider : public ScreenMaskPanel, public Touchable
{
public:
	Slider() : horizontal(false), 
		sliderTrack(NULL), 
		sliderKnob(NULL), 
		knobSelected(false), 
		percentage(0){};

	V_DECLARE_DYNCREATE(Slider);

	void init(bool horizontal, hkvVec2 position, hkvVec2 trackSize, hkvVec2 knobSize, VTextureObject* trackTexture, VTextureObject* knobTexture);

	virtual bool containsTouch(hkvVec2 touchLocation);
	virtual void onPointerDown(hkvVec2 touchLocation);
	virtual bool onPointerMove(hkvVec2 touchLocation);
	virtual void onPointerUp(hkvVec2 touchLocation);

protected:
	bool horizontal; //horizontal or vertical slider

	hkvVec2 trackSize;
	hkvVec2 knobSize;

	SpriteScreenMask* sliderTrack;
	SpriteScreenMask* sliderKnob;

	bool knobSelected;

	float percentage;
};