/**
 * Compiz Anaglyph plugin
 *
 * anaglyph.cpp
 * Copyright (c) 2007	Patryk Kowalczyk <wodor@wodor.org>
 * Ported to Compiz 0.9.x
 * Copyright (c) 2010 Sam Spilsbury <smspillaz@gmail.com>
 * Copyright (c) 2010 Scott Moreau
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Author:	Patryk Kowalczyk <wodor@wodor.org>
 **/

#include "anaglyph.h"

COMPIZ_PLUGIN_20090315 (anaglyph, AnaglyphPluginVTable);

void
AnaglyphWindow::toggle ()
{
    ANAGLYPH_SCREEN (screen);
    mIsAnaglyph = !mIsAnaglyph;

    if (as->optionGetExcludeMatch ().evaluate (window) ||
			    window->overrideRedirect ())
    {
	mIsAnaglyph = false;
    }
}

void
AnaglyphScreen::toggle ()
{
    mIsAnaglyph = !mIsAnaglyph;

    foreach (CompWindow *w, screen->windows ())
    {
	AnaglyphWindow::get (w)->toggle ();
    }
}

bool
AnaglyphScreen::anaglyphWindow (CompOption::Vector options)
{
    CompWindow *w = screen->findWindow (CompOption::getIntOptionNamed (options,
								       "window",
								       0));
    if (w)
	AnaglyphWindow::get (w)->toggle ();

    return true;
}

bool
AnaglyphScreen::anaglyphScreen ()
{
    toggle ();

    return true;
}

//------------------------------------------ MASTER FUNCTION

bool
AnaglyphWindow::glPaint (const GLWindowPaintAttrib &attrib,
			const GLMatrix		  &transform,
			const CompRegion	  &region,
			unsigned int		  mask)
{
    bool status;
    ANAGLYPH_SCREEN(screen);

    if (mIsAnaglyph && gWindow->textures ().size ())
    {
	//if (as->isAnaglyph)
	//	aw->isAnaglyph = FALSE;

	GLuint oldFilter = as->gScreen->textureFilter ();

	if (as->optionGetMipmaps ())
	    as->gScreen->setTextureFilter (GL_LINEAR_MIPMAP_LINEAR);

	mask |= PAINT_WINDOW_TRANSFORMED_MASK;

	GLMatrix sTransform (transform);
	GLWindowPaintAttrib wAttrib (attrib);

	float offset = as->optionGetOffset ();
	float desktopOffset = as->optionGetDesktopOffset ();

	if (as->optionGetDesaturate ())
	    wAttrib.saturation = 0.0f;

	gWindow->glPaintSetEnabled (this, false);

	//BLUE and ...
	glColorMask(GL_FALSE,GL_TRUE,GL_TRUE,GL_FALSE);
	if (window->type () & CompWindowTypeDesktopMask) //desktop
	    sTransform.translate (offset * desktopOffset, 0.0f, 0.0f);
	else if (window->state () & CompWindowStateShadedMask)
	    sTransform.translate (0.0f, 0.0f, 0.0f);
	else if ((window->state () & CompWindowStateMaximizedHorzMask) ||
		 (window->state () & CompWindowStateMaximizedVertMask ))
	    sTransform.translate (-offset * 3.5, 0.0f, 0.0f);
	else if (window->type () & CompWindowTypeDockMask) // dock
	    sTransform.translate (0.0f, 0.0f, 0.0f);
	else if (window->state () & CompWindowStateStickyMask) // sticky
	    sTransform.translate (offset * desktopOffset, 0.0f, 0.0f);
	else if (window->state () & CompWindowStateBelowMask) //below
	    sTransform.translate (offset, 0.0f, 0.0f);
	else if (window->state () & CompWindowStateAboveMask) // above
	    sTransform.translate (-offset * 4.0, 0.0f, 0.0f);
	else if (window->id () == screen->activeWindow ()) // active window
	    sTransform.translate (-offset * 3.0, 0.0f, 0.0f);
	else //other windows
	    sTransform.translate (-offset, 0.0f, 0.0f);

	status = gWindow->glPaint (wAttrib, sTransform, region, mask);

	//RED
	glColorMask(GL_TRUE,GL_FALSE,GL_FALSE,GL_FALSE);
	if (window->type () & CompWindowTypeDesktopMask) //desktop
	    sTransform.translate (-offset * 2.0 * desktopOffset, 0.0f, 0.0f);
	else if (window->state () & CompWindowStateShadedMask)
	    sTransform.translate (0.0f, 0.0f, 0.0f);
	else if ((window->state () & CompWindowStateMaximizedHorzMask) ||
		 (window->state () & CompWindowStateMaximizedVertMask ))
	    sTransform.translate (offset *3.5, 0.0f, 0.0f);
	else if (window->type () & CompWindowTypeDockMask)// dock
	    sTransform.translate (0.0f, 0.0f, 0.0f);
	else if (window->state () & CompWindowStateStickyMask) // sticky
	    sTransform.translate (-offset * 2.0 * desktopOffset, 0.0f, 0.0f);
	else if (window->state () & CompWindowStateBelowMask) //below
	    sTransform.translate (-offset * 2.0, 0.0f, 0.0f);
	else if (window->state () & CompWindowStateAboveMask) //above
	    sTransform.translate (offset * 4.0, 0.0f, 0.0f);
	else if (window->id () == screen->activeWindow ()) // active window
	    sTransform.translate (offset * 3.0, 0.0f, 0.0f);
	else //other windows
	    sTransform.translate (offset * 2.0, 0.0f, 0.0f);

	status = gWindow->glPaint (wAttrib, sTransform, region, mask);

	gWindow->glPaintSetEnabled (this, true);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	as->gScreen->setTextureFilter (oldFilter);
    }
    else
    {
	status = gWindow->glPaint (attrib, transform, region, mask);
    }
    return status;
}

//------------------------------------------------------------------------------DAMAGE FUNCTION

bool
AnaglyphWindow::damageRect (bool initial,
			    const CompRect &rect)
{
    bool status = false;

    ANAGLYPH_SCREEN(screen);

    if (mIsAnaglyph || as->mIsAnaglyph || as->mIsDamage)
    {
	as->mIsDamage = TRUE;
	if (!mIsAnaglyph && !mIsAnaglyph)
	    as->mIsDamage = FALSE;

	as->cScreen->damageScreen ();
	status = TRUE;
    }

    status |= cWindow->damageRect (initial, rect);

    return status;
}

//----------------------------------------------------------------------------------- PAINT OUTPUT

bool
AnaglyphScreen::glPaintOutput (const GLScreenPaintAttrib &attrib,
			      const GLMatrix		&transform,
			      const CompRegion		&region,
			      CompOutput		*output,
			      unsigned int		mask)
{
    mask |= PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS_MASK;

    return gScreen->glPaintOutput (attrib, transform, region, output, mask);
}

//------------------------------------------------------------------------------

void
AnaglyphScreen::optionChanged (CompOption *option,
			       Options    num)
{
    switch (num)
    {
	case AnaglyphOptions::AnaglyphMatch:
	case AnaglyphOptions::ExcludeMatch:
	{
	    foreach (CompWindow *w, screen->windows ())
	    {
		bool isAnaglyph;

		ANAGLYPH_WINDOW (w);

		isAnaglyph = optionGetAnaglyphMatch ().evaluate (w);
		isAnaglyph = isAnaglyph &&
					!optionGetExcludeMatch ().evaluate (w);

		if (isAnaglyph && mIsAnaglyph && !aw->mIsAnaglyph)
		    aw->toggle ();
	    }
	}
	break;
	default:
	    break;
    }
}


//----------------------------------------------------------------------- SCREEN

AnaglyphScreen::AnaglyphScreen (CompScreen *s) :
    PluginClassHandler <AnaglyphScreen, CompScreen> (s),
    cScreen (CompositeScreen::get (screen)),
    gScreen (GLScreen::get (screen)),
    mIsAnaglyph (false),
    mIsDamage (false)
{
    CompositeScreenInterface::setHandler (cScreen);
    GLScreenInterface::setHandler (gScreen);

    optionSetWindowToggleKeyInitiate (boost::bind
				    (&AnaglyphScreen::anaglyphWindow, this,
				     _3));
    optionSetScreenToggleKeyInitiate (boost::bind
				    (&AnaglyphScreen::anaglyphScreen, this));
    optionSetWindowToggleKeyInitiate (boost::bind
				     (&AnaglyphScreen::anaglyphWindow, this,
				      _3));
    optionSetScreenToggleKeyInitiate (boost::bind
				      (&AnaglyphScreen::anaglyphScreen, this));

    optionSetAnaglyphMatchNotify (boost::bind (&AnaglyphScreen::optionChanged,
					       this, _1, _2));
    optionSetExcludeMatchNotify (boost::bind (&AnaglyphScreen::optionChanged,
					      this, _1, _2));
}

AnaglyphScreen::~AnaglyphScreen ()
{
}

//----------------------------------------------------------------------- WINDOW

AnaglyphWindow::AnaglyphWindow (CompWindow *w) :
    PluginClassHandler <AnaglyphWindow, CompWindow> (w),
    window (w),
    gWindow (GLWindow::get (w)),
    cWindow (CompositeWindow::get (w)),
    mIsAnaglyph (false)
{
    ANAGLYPH_SCREEN (screen);

    CompositeWindowInterface::setHandler (cWindow);
    GLWindowInterface::setHandler (gWindow);

    if (as->mIsAnaglyph && as->optionGetAnaglyphMatch ().evaluate (w))
	toggle ();
}

AnaglyphWindow::~AnaglyphWindow ()
{
}

bool
AnaglyphPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) ||
	!CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) ||
	!CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
	return false;
    
    return true;
}
