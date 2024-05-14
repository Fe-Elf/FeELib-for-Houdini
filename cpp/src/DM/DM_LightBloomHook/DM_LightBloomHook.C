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

#include <RE/RE_Geometry.h>
#include <RE/RE_OGLFramebuffer.h>
#include <RE/RE_Render.h>
#include <RE/RE_Shader.h>
#include <RE/RE_Texture.h>

#include <GUI/GUI_DisplayOption.h>
#include <DM/DM_RenderTable.h>
#include <DM/DM_VPortAgent.h>

namespace HDK_Sample {

const char *copy_frag =
    "#version 150\n"
    "uniform sampler2D beauty;\n"
    "in vec2 texcoord0; \n"
    "out vec4 color; \n"
    "void main()\n"
    "{ color = texture(beauty, texcoord0); }\n";

const char *bloom_vert =
    "#version 150\n"
    "in vec3 P;\n"
    "in vec2 uv;\n"
    "out vec2 texcoord0;\n"
    "void main()\n"
    "{\n"
    "  texcoord0 = uv;\n"
    "  gl_Position = vec4(P, 1.0);\n"
    "}\n";
 
const char *bloom_frag =
    "#version 150\n"
    "uniform sampler2D beauty;\n"
    "uniform sampler2D bloom;\n"
    "uniform vec2 off;\n"
    "in vec2 texcoord0; \n"
    "out vec4 color; \n"
    "void main()\n"
    "{\n"
    "  float scale = 2.5 - texture(beauty, texcoord0).a * 2.0;\n"
    "  color = scale * (texture( bloom, texcoord0) * 0.24 +"
    "      (texture2D(bloom, texcoord0 + vec2(off.x, 0.0)) + \n"
    "       texture2D(bloom, texcoord0 + vec2(-off.x, 0.0)) + \n"
    "       texture2D(bloom, texcoord0 + vec2(0.0, -off.y)) + \n"
    "       texture2D(bloom, texcoord0 + vec2(0.0,  off.y))) * 0.12 +\n"
    "      (texture2D(bloom, texcoord0 + off) + \n"
    "       texture2D(bloom, texcoord0 - off) + \n"
    "       texture2D(bloom, texcoord0 + vec2(off.x,-off.y)) + \n"
    "       texture2D(bloom, texcoord0 - vec2(off.x,-off.y)))*0.07);\n"
    "}\n";


// This hook is the actual render hook, responsible for doing the GL drawing.
class DM_LightBloomRenderHook : public DM_SceneRenderHook
{
public:
    DM_LightBloomRenderHook(DM_VPortAgent &vport)
	: DM_SceneRenderHook(vport, DM_VIEWPORT_ALL_3D),
	  myBloomBuffer(NULL),
	  myBloomTexture(NULL)
	{}
     
    ~DM_LightBloomRenderHook() override
	{
	    delete myBloomTexture;
	    delete myBloomBuffer;
	}

    bool                 supportsRenderer(GR_RenderVersion version) override
				{ return true; }
   
    bool                render(RE_Render *r,
			       const DM_SceneHookData &hook_data) override;
    void                viewportClosed() override;

    void		reset()
	{
	    delete myBloomTexture;
	    delete myBloomBuffer;
	    myBloomTexture = NULL;
	    myBloomBuffer = NULL;
	}
    
private:
    RE_OGLFramebuffer *myBloomBuffer;
    RE_Texture	      *myBloomTexture;
    
    static RE_Shader  *theCopyShader;
    static RE_Shader  *theBloomShader;
};

class DM_LightBloomHook : public DM_SceneHook
{
public:
	     DM_LightBloomHook() : DM_SceneHook("LightBloom", 0,
                                                DM_HOOK_ALL_VIEWS) {}
            ~DM_LightBloomHook() override {}
    
    DM_SceneRenderHook *newSceneRender(DM_VPortAgent &vport,
				       DM_SceneHookType type,
				       DM_SceneHookPolicy policy) override
	{ return new DM_LightBloomRenderHook(vport); }
    
    void retireSceneRender(DM_VPortAgent &vport,
			   DM_SceneRenderHook *hook) override
	{ delete hook; }
};

} // end HDK_Sample namespace

using namespace HDK_Sample;

RE_Shader *DM_LightBloomRenderHook::theCopyShader = NULL;
RE_Shader *DM_LightBloomRenderHook::theBloomShader = NULL;


bool
DM_LightBloomRenderHook::render(RE_Render *r,
				const DM_SceneHookData &hook_data)
{
    if(!hook_data.disp_options->isSceneOptionEnabled("light_bloom"))
	return false;

    RE_Texture *beauty = viewport().getBeautyPassTexture(r);
    if(!beauty)
	return false;

    if(!theCopyShader)
    {
	theCopyShader = RE_Shader::create("Copy Shader");
	theCopyShader->addShader(r, RE_SHADER_VERTEX, bloom_vert,"vert", 0);
	theCopyShader->addShader(r, RE_SHADER_FRAGMENT, copy_frag,"frag", 0);
	if(!theCopyShader->linkShaders(r))
	{
	    return false;
	}
    }
    if(!theBloomShader)
    {
	theBloomShader = RE_Shader::create("Bloom Shader");
	theBloomShader->addShader(r, RE_SHADER_VERTEX, bloom_vert,"vert", 0);
	theBloomShader->addShader(r, RE_SHADER_FRAGMENT, bloom_frag,"frag", 0);
	if(!theBloomShader->linkShaders(r))
	{
	    return false;
	}

	theBloomShader->bindInt(r, RE_UniformNames::beauty, 0); // texture unit 0
	theBloomShader->bindInt(r, RE_UniformNames::bloom,  1); // texture unit 1
    }

    // Initialize the small framebuffer
    int bw = hook_data.view_width/4;
    int bh = hook_data.view_width/4;

    if(myBloomBuffer &&
       (myBloomBuffer->getWidth() != bw || myBloomBuffer->getHeight() != bh))
    {
	reset();
    }

    if(!myBloomBuffer)
    {
	myBloomBuffer = new RE_OGLFramebuffer();
	myBloomBuffer->setResolution(bw, bh);
	myBloomTexture = myBloomBuffer->createTexture(r, RE_GPU_FLOAT16, 4);
	myBloomTexture->setTextureWrap(r, RE_CLAMP_EDGE, RE_CLAMP_EDGE);

	if(!myBloomTexture || !myBloomBuffer->isValid(r))
	{
	     reset();
	     return false;
	}
    }

    UT_DimRect saved_vp = r->getViewport2DI();
    UT_DimRect vp(0,0, bw,bh);
    RE_Geometry geo(4);
    fpreal32 view[12] = { -1.f, -1.f, -0.1f, -1.f, 1.f, -0.1f, 1.f, -1.f, -0.1f, 1.f, 1.f, -1.f };
    fpreal32 texc[8] = {  0,  0,  0, 1, 1,  0, 1, 1 };
    geo.createAttribute(r, "P", RE_GPU_FLOAT32, 3, view);
    geo.createAttribute(r, "uv", RE_GPU_FLOAT32, 2, texc);
    geo.connectAllPrims(r, 0, RE_PRIM_TRIANGLE_STRIP);
    
    // copy beauty texture to our texture
    r->pushDrawFramebuffer( myBloomBuffer );
    r->viewport2DI(vp);
    r->clearC();

    r->pushTextureState(0);
    r->bindTexture( beauty, 0);

    r->pushShader(theCopyShader);
    geo.draw(r, 0);

    r->popTextureState();

    r->popDrawFramebuffer();

    r->viewport2DI(saved_vp);

    // do a simple additive blur

    r->bindShader(theBloomShader);

    fpreal32 offset[2];
    offset[0] = 2.0 / bw;
    offset[1] = 2.0 / bh;
    theBloomShader->bindVariable2(r, RE_UniformNames::off, offset);

    r->pushBlendState();
    r->blend(1);
    r->setBlendFunction(RE_SBLEND_SRC_ALPHA, RE_DBLEND_ONE); // additive
    r->setAlphaBlendFunction(RE_SBLEND_ZERO, RE_DBLEND_ONE); // ignore

    r->pushTextureState(RE_ALL_UNITS);
    r->bindTexture(beauty,
		   theBloomShader->getUniformTextureUnit("beauty"));
    r->bindTexture(myBloomTexture,
		   theBloomShader->getUniformTextureUnit("bloom"));

    r->pushDepthState();
    r->disableDepthTest();

    
    // draw fullscreen quad
    geo.draw(r, 0);

    r->popDepthState();

    r->popTextureState();
    
    r->popBlendState();
    r->popShader();
    
    return false;
}

void
DM_LightBloomRenderHook::viewportClosed()
{
    reset();
}





// DSO hook registration function
void
newRenderHook(DM_RenderTable *table)
{
    table->registerSceneHook(new DM_LightBloomHook,
			     DM_HOOK_FOREGROUND,
			     DM_HOOK_BEFORE_NATIVE);

    // Note that the name or label doesn't have to match the hook's name.
    table->installSceneOption("light_bloom", "Light Bloom");
}
