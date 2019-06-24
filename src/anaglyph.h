 
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
 
#include <core/core.h>
#include <composite/composite.h>
#include <opengl/opengl.h>

#include "anaglyph_options.h"

class AnaglyphScreen :
    public PluginClassHandler <AnaglyphScreen, CompScreen>,
    public CompositeScreenInterface,
    public GLScreenInterface,
    public AnaglyphOptions
{
    public:
	
	AnaglyphScreen (CompScreen *);
	~AnaglyphScreen ();
    
    public:
    
	CompositeScreen *cScreen;
	GLScreen	*gScreen;
	
	bool		mIsAnaglyph;
	bool		mIsDamage;

	void toggle ();

	bool anaglyphWindow (CompOption::Vector options);
	bool anaglyphScreen ();

	void
	optionChanged (CompOption *, Options);
	
	bool
	glPaintOutput (const GLScreenPaintAttrib &,
		       const GLMatrix		 &,
		       const CompRegion		 &,
		       CompOutput		 *,
		       unsigned int);
};

#define ANAGLYPH_SCREEN(s) \
    AnaglyphScreen *as = AnaglyphScreen::get (s);


class AnaglyphWindow :
    public PluginClassHandler <AnaglyphWindow, CompWindow>,
    public CompositeWindowInterface,
    public GLWindowInterface
{
    public:
    
	AnaglyphWindow (CompWindow *w);
	~AnaglyphWindow ();
	
    public:

	CompWindow *window;
	GLWindow   *gWindow;
	CompositeWindow *cWindow;
	
	bool mIsAnaglyph;

	void toggle ();
	
	bool
	glPaint (const GLWindowPaintAttrib &,
		 const GLMatrix		   &,
		 const CompRegion	   &,
		 unsigned int		    );
	
	bool
	damageRect (bool, const CompRect &);
	
};

#define ANAGLYPH_WINDOW(w) \
    AnaglyphWindow *aw = AnaglyphWindow::get (w);

class AnaglyphPluginVTable :
    public CompPlugin::VTableForScreenAndWindow <AnaglyphScreen, AnaglyphWindow>
{
    public:

	bool init ();
};