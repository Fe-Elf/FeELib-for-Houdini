/*
 * Copyright (c) 2022
 *	Side Effects Software Inc.  All rights reserved.
 *
 * Redistribution and use of Houdini Development Kit samples in source and
 * binary forms, with or without modification, are permitted provided that the
 * following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. The name of Side Effects Software may not be used to endorse or
 *    promote products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY SIDE EFFECTS SOFTWARE `AS IS' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 * NO EVENT SHALL SIDE EFFECTS SOFTWARE BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *----------------------------------------------------------------------------
 */
#include <UT/UT_DSOVersion.h>

#include <RE/RE_Font.h>
#include <RE/RE_Render.h>

#include <GUI/GUI_DisplayOption.h>
#include <DM/DM_GeoDetail.h>
#include <DM/DM_RenderTable.h>
#include <DM/DM_SceneHook.h>
#include <DM/DM_VPortAgent.h>

namespace HDK_Sample {

// This sample prints the name of the current SOP in the viewport when in an
// object.

class DM_InfoRenderHook : public DM_SceneRenderHook
{
public:
    DM_InfoRenderHook(DM_VPortAgent &vport)
	: DM_SceneRenderHook(vport, DM_VIEWPORT_ALL)
	{}
    
   bool render(RE_Render *r,
	       const DM_SceneHookData &hook_data) override
	{
	    // Check our display option, if not enabled, exit.
	    if(!hook_data.disp_options->isSceneOptionEnabled("current_info"))
		return false;
	    
	    DM_GeoDetail cur_detail = viewport().getCurrentDetail();
	    
	    if(cur_detail.isValid() && cur_detail.getNumDetails() > 0)
	    {
		UT_String name = cur_detail.getDescriptiveName(0);
		
		// Draw in the default viewport font with the wire color, so
		// that it is always visible regardless of color scheme.
		RE_Font &font = RE_Render::getViewportFont();
		UT_Color text_col = 
		    hook_data.disp_options->common().defaultWireColor();

		r->pushColor(text_col);
		r->setFont( font );

		int w = font.getStringWidth(name);
		int h = font.getHeight();

		// at the bottom and centered in the viewport
		r->textMoveW( (hook_data.view_width - w) * 0.5, h *0.5);
		r->putString( name );

		r->popColor();
	    }

	    return false; // allow other forground hooks to draw.
	}
};

class DM_InfoHook : public DM_SceneHook
{
public:
    DM_InfoHook() : DM_SceneHook("Simple Info Display", 0,
                                 DM_HOOK_OBJSOPDOP_VIEW)
	{}

    DM_SceneRenderHook *newSceneRender(DM_VPortAgent &vport,
				       DM_SceneHookType type,
				       DM_SceneHookPolicy policy) override
	{
	    return new DM_InfoRenderHook(vport);
	}

    void retireSceneRender(DM_VPortAgent &vport,
			   DM_SceneRenderHook *hook) override
	{
	    // Shared hooks might deference a ref count, but this example just
	    // creates one hook per viewport.
	    delete hook;
	}
};

} // END HDK_Sample namespace

using namespace HDK_Sample;


void
newRenderHook(DM_RenderTable *table)
{
    table->registerSceneHook(new DM_InfoHook,
			     DM_HOOK_FOREGROUND,
			     DM_HOOK_AFTER_NATIVE);
    table->installSceneOption("current_info", "Current SOP Info");
}
