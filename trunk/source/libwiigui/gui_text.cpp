/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_text.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"
#include "html.h"
#include "wstring.hpp"

static int presetSize = 18;
static int presetMaxWidth = 0;
static int presetAlignmentHor = 0;
static int presetAlignmentVert = 0;
static u16 presetStyle = 0;
static GXColor presetColor = (GXColor){255, 255, 255, 255};

#define TEXT_SCROLL_DELAY			5
#define	TEXT_SCROLL_INITIAL_DELAY	8

/**
 * Constructor for the GuiText class.
 */

GuiText::GuiText(const char * t, int s, GXColor c)
{
	text = NULL;
	size = s;
	currentSize = size;
	color = c;
	alpha = c.a;
	style = FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE;
	maxWidth = 0;
	wrapMode = 0;
	font = NULL;
	usespace = true;
	linestodraw = MAX_LINES_TO_DRAW;
	textModel = NORMAL;
	textScrollPos = 0;
	textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;
	textScrollDelay = TEXT_SCROLL_DELAY;

	alignmentHor = ALIGN_CENTRE;
	alignmentVert = ALIGN_MIDDLE;

	if(t)
	{
		text = charToWideChar(t);
		if(!text)
		    return;

		textWidth = fontSystem->getWidth(text, currentSize);
	}
	selectable = true;
}

GuiText::GuiText(const wchar_t * t, int s, GXColor c)
{
	text = NULL;
	size = s;
	currentSize = size;
	color = c;
	alpha = c.a;
	style = FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE;
	maxWidth = 0;
	wrapMode = 0;
	font = NULL;
	usespace=true;
	linestodraw = MAX_LINES_TO_DRAW;
	textModel = NORMAL;
	textScrollPos = 0;
	textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;
	textScrollDelay = TEXT_SCROLL_DELAY;

	alignmentHor = ALIGN_CENTRE;
	alignmentVert = ALIGN_MIDDLE;

	if(t)
	{
		text = new (std::nothrow) wchar_t[wcslen(t)+1];
		if(!text)
            return;

		wcscpy(text, t);

		textWidth = fontSystem->getWidth(text, currentSize);
	}
	selectable = true;
}

/**
 * Constructor for the GuiText class, uses presets
 */
GuiText::GuiText(const char * t)
{
	text = NULL;
	size = presetSize;
	currentSize = size;
	color = presetColor;
	alpha = presetColor.a;
	style = presetStyle;
	maxWidth = presetMaxWidth;
	wrapMode = 0;
	font = NULL;
	usespace=true;
	linestodraw = MAX_LINES_TO_DRAW;
	textModel = NORMAL;
	textScrollPos = 0;
	textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;
	textScrollDelay = TEXT_SCROLL_DELAY;

	alignmentHor = presetAlignmentHor;
	alignmentVert = presetAlignmentVert;

	if(t)
	{
		text = charToWideChar(t);
		if(!text)
		    return;

		textWidth = fontSystem->getWidth(text, currentSize);
	}
	selectable = true;
}


/**
 * Destructor for the GuiText class.
 */
GuiText::~GuiText()
{
	if(text)
		delete [] text;
	text = NULL;

	if(font)
	{
        delete font;
        font = NULL;
    }

    ClearDynamicText();
}

// overloaded new operator
void *GuiText::operator new(size_t size)
{
	void *p = gui_malloc(size);

	if (!p)
	{
		bad_alloc ba;
		throw ba;
	}
	return p;
}

// overloaded delete operator
void GuiText::operator delete(void *p)
{
	gui_free(p);
}

// overloaded new operator for arrays
void *GuiText::operator new[](size_t size)
{
	void *p = gui_malloc(size);

	if (!p)
	{
		bad_alloc ba;
		throw ba;
	}
	return p;
}

// overloaded delete operator for arrays
void GuiText::operator delete[](void *p)
{
	gui_free(p);
}

void GuiText::SetText(const char * t)
{
    LOCK(this);

	if(text)
		delete [] text;
    text = NULL;

    ClearDynamicText();

	textScrollPos = 0;
	textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;

	if(t)
	{
		text = charToWideChar(t);
		if(!text)
		    return;

		textWidth = fontSystem->getWidth(text, currentSize);
	}
}

void GuiText::SetTextf(const char *format, ...)
{
    if(!format)
        SetText((char *) NULL);

	char *tmp=0;
	va_list va;
	va_start(va, format);
	if((vasprintf(&tmp, format, va)>=0) && tmp)
	{
		SetText(tmp);
	}
	va_end(va);

	if(tmp)
        free(tmp);
}


void GuiText::SetText(const wchar_t * t)
{
    LOCK(this);

	if(text)
		delete [] text;
    text = NULL;

    ClearDynamicText();

	textScrollPos = 0;
	textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;

	if(t)
	{
		text = new (std::nothrow) wchar_t[wcslen(t)+1];
		if(!text)
            return;

		wcscpy(text, t);

		textWidth = fontSystem->getWidth(text, currentSize);
	}
}

void GuiText::ClearDynamicText()
{
    for(u32 i = 0; i < textDyn.size(); i++)
    {
        if(textDyn[i])
            delete [] textDyn[i];
    }
    textDyn.clear();
}

void GuiText::SetPresets(int sz, GXColor c, int w, u16 s, int h, int v)
{
    LOCK(this);

	presetSize = sz;
	presetColor = c;
	presetStyle = s;
	presetMaxWidth = w;
	presetAlignmentHor = h;
	presetAlignmentVert = v;
}

void GuiText::SetFontSize(int s)
{
    LOCK(this);

	size = s;
}

void GuiText::SetSpace(bool space)
{
    LOCK(this);
    usespace=space;
}

void GuiText::SetMaxWidth(int width, int w)
{
    LOCK(this);

	maxWidth = width;
	wrapMode = w;

	if(w == SCROLL_HORIZONTAL)
	{
        textScrollPos = 0;
        textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;
        textScrollDelay = TEXT_SCROLL_DELAY;
	}

    ClearDynamicText();
}

void GuiText::SetColor(GXColor c)
{
    LOCK(this);
	color = c;
	alpha = c.a;
}

void GuiText::SetStyle(u16 s)
{
    LOCK(this);
	style |= s;
}

void GuiText::SetAlignment(int hor, int vert)
{
    LOCK(this);
	style = 0;

	switch(hor)
	{
		case ALIGN_LEFT:
			style |= FTGX_JUSTIFY_LEFT;
			break;
		case ALIGN_RIGHT:
			style |= FTGX_JUSTIFY_RIGHT;
			break;
		default:
			style |= FTGX_JUSTIFY_CENTER;
			break;
	}
	switch(vert)
	{
		case ALIGN_TOP:
			style |= FTGX_ALIGN_TOP;
			break;
		case ALIGN_BOTTOM:
			style |= FTGX_ALIGN_BOTTOM;
			break;
		default:
			style |= FTGX_ALIGN_MIDDLE;
			break;
	}

	alignmentHor = hor;
	alignmentVert = vert;
}

void GuiText::SetLinesToDraw(int l)
{
    linestodraw = l;
}

int GuiText::GetLinesCount()
{
	if(!text)
		return 0;

	return textDyn.size()>0 ? textDyn.size() : 1;
}

int GuiText::GetTextWidth()
{
	if(!text)
		return 0;

	return fontSystem->getWidth(text, currentSize);
}

int GuiText::GetTextWidth(int ind)
{
	if(ind < 0 || ind >= (int) textDyn.size())
		return this->GetTextWidth();

	return fontSystem->getWidth(textDyn[ind], currentSize);
}

int GuiText::GetTextMaxWidth()
{
    return maxWidth;
}

const wchar_t * GuiText::GetDynText(int ind)
{
    if(ind < 0 || ind >= (int) textDyn.size())
        return text;

    return textDyn[ind];
}

const wchar_t * GuiText::GetText()
{
    return text;
}

/**
 * Change font
 */
bool GuiText::SetFont(const u8 *fontbuffer, const u32 filesize)
{
    if(!fontbuffer || !filesize)
        return false;

    LOCK(this);
    if(font)
    {
        delete font;
        font = NULL;
    }
	font = new FreeTypeGX(fontbuffer, filesize);
	textWidth = font->getWidth(text, currentSize);

	return true;
}

void GuiText::MakeDottedText()
{
    int pos = textDyn.size();
    textDyn.resize(pos+1);

    int i = 0, currentWidth = 0;
    textDyn[pos] = new wchar_t[maxWidth];

    while(text[i])
    {
        currentWidth += (font ? font : fontSystem)->getCharWidth(text[i], currentSize, i > 0 ? text[i-1] : 0x0000);
        if(currentWidth >= maxWidth)
        {
            if(i > 3)
            {
                textDyn[pos][i-3] = '.';
                textDyn[pos][i-2] = '.';
                textDyn[pos][i-1] = '.';
            }
            break;
        }

        textDyn[pos][i] = text[i];

        i++;
    }
    textDyn[pos][i] = 0;
}

void GuiText::ScrollText()
{
    if(textDyn.size() == 0)
    {
        int pos = textDyn.size();
        int i = 0, currentWidth = 0;
        textDyn.resize(pos+1);

        textDyn[pos] = new wchar_t[maxWidth];

        while(text[i] && currentWidth < maxWidth)
        {
            textDyn[pos][i] = text[i];

            currentWidth += (font ? font : fontSystem)->getCharWidth(text[i], currentSize, i > 0 ? text[i-1] : 0x0000);

            ++i;
        }
        textDyn[pos][i] = 0;

        return;
    }

    if(FrameTimer % textScrollDelay != 0)
    {
        return;
    }

    if(textScrollInitialDelay)
    {
        --textScrollInitialDelay;
        return;
    }

    int strlen = wcslen(text);

    ++textScrollPos;
    if(textScrollPos > strlen)
    {
        textScrollPos = 0;
        textScrollInitialDelay = TEXT_SCROLL_INITIAL_DELAY;
    }

    int ch = textScrollPos;
    int pos = textDyn.size()-1;

    if(textDyn[pos])
        delete [] textDyn[pos];

    textDyn[pos] = new wchar_t[maxWidth];

    int i = 0, currentWidth = 0;

    while(currentWidth < maxWidth)
    {
        if(ch > strlen-1)
        {
            textDyn[pos][i++] = ' ';
            textDyn[pos][i++] = ' ';
            textDyn[pos][i++] = ' ';
            ch = 0;
        }

        textDyn[pos][i] = text[ch];
        ++ch;
        ++i;

        currentWidth += (font ? font : fontSystem)->getCharWidth(text[ch], currentSize, ch > 0 ? text[ch-1] : 0x0000);
    }
    textDyn[pos][i] = 0;
}

void GuiText::WrapText()
{

    if(textDyn.size() > 0)
        return;

    int i = 0;
    int ch = 0;
    int linenum = 0;
    int lastSpace = -1;
    int lastSpaceIndex = -1;
    int currentWidth = 0;

    while(text[ch] && linenum < linestodraw)
    {
        if(linenum >= (int) textDyn.size())
        {
            textDyn.resize(linenum+1);
            textDyn[linenum] = new wchar_t[maxWidth];
        }

        textDyn[linenum][i] = text[ch];
        textDyn[linenum][i+1] = 0;

        currentWidth += (font ? font : fontSystem)->getCharWidth(text[ch], currentSize, ch > 0 ? text[ch-1] : 0x0000);

        if(currentWidth >= ((linenum == 0 || (style & FTGX_JUSTIFY_MASK) != FTGX_JUSTIFY_LEFT) ? maxWidth : screenwidth-90))
        {
            if(lastSpace >= 0)
            {
                textDyn[linenum][lastSpaceIndex] = 0; // discard space, and everything after
                ch = lastSpace; // go backwards to the last space
                lastSpace = -1; // we have used this space
                lastSpaceIndex = -1;
            }

            if(linenum+1 == linestodraw && text[ch+1] != 0x0000)
            {
                textDyn[linenum][i-2] = '.';
                textDyn[linenum][i-1] = '.';
                textDyn[linenum][i] = '.';
                textDyn[linenum][i+1] = 0;
            }

            currentWidth = 0;
            ++linenum;
            i = -1;
        }
        if(usespace && text[ch] == ' ' && i >= 0)
        {
            lastSpace = ch;
            lastSpaceIndex = i;
        }
        ++ch;
        ++i;
    }
}

/**
 * Update the text on screen
 */
bool GuiText::IsOver(int x, int y)
{
	if(unsigned(x - this->GetLeft())  < unsigned(textWidth)
	&& unsigned(y - this->GetTop())  < unsigned(size/**GetLinesCount()*/))
		return true;
	return false;
}

void GuiText::SetModel(int model)
{
    textModel = model;
}

void GuiText::Select(GuiTrigger * t)
{
    if (textModel != ANCHOR)
        return;

    if(t->wpad->ir.valid && t->chan >= 0)
	{
        if(this->IsOver(t->wpad->ir.x, t->wpad->ir.y))
        {
            this->SetStyle(FTGX_STYLE_UNDERLINE);
        }
        else
        {
            style &= (~FTGX_STYLE_MASK);
        }
	}
}

/**
 * Draw the text on screen
 */
void GuiText::Draw()
{
	if(!text)
		return;

	if(!IsVisible())
		return;

	GXColor c = color;
	c.a = GetAlpha();

	int newSize = size*GetScale();

	if(newSize != currentSize)
	{
		currentSize = newSize;

        if(text)
            textWidth = (font ? font : fontSystem)->getWidth(text, currentSize);
	}

	if(maxWidth > 0 && maxWidth <= textWidth)
	{
		if(wrapMode == DOTTED) // text dotted
		{
		    if(textDyn.size() == 0)
		        MakeDottedText();

		    if(textDyn.size() > 0)
                (font ? font : fontSystem)->drawText(this->GetLeft(), this->GetTop(), GetZPosition(), textDyn[textDyn.size()-1], currentSize, c, style);
		}

		else if(wrapMode == SCROLL_HORIZONTAL)
		{
		    ScrollText();

			if(textDyn.size() > 0)
				(font ? font : fontSystem)->drawText(this->GetLeft(), this->GetTop(), GetZPosition(), textDyn[textDyn.size()-1], currentSize, c, style);
        }
		else if(wrapMode == WRAP)
		{
            int lineheight = currentSize + 5;
            int voffset = 0, leftPoint;
            if(alignmentVert == ALIGN_MIDDLE)
                voffset = -(lineheight*textDyn.size())/2 + lineheight/2;

		    if(textDyn.size() == 0)
                WrapText();
            leftPoint = ((style & FTGX_JUSTIFY_MASK) != FTGX_JUSTIFY_LEFT) ? this->GetLeft() : this->GetOffset();

            for(u32 i = 0; i < textDyn.size(); i++)
            {
                if (i == 0)
                    (font ? font : fontSystem)->drawText(this->GetLeft(), this->GetTop()+voffset, GetZPosition(), textDyn[i], currentSize, c, style);
                else (font ? font : fontSystem)->drawText(leftPoint, this->GetTop()+voffset+i*lineheight, GetZPosition(), textDyn[i], currentSize, c, style);
            }
		}
	}
	else
	{
		(font ? font : fontSystem)->drawText(this->GetLeft(), this->GetTop(), GetZPosition(), text, currentSize, c, style, textWidth);
	}
	this->UpdateEffects();
}
