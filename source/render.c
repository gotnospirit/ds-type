#include <stdlib.h>
#include <stdio.h>

#include <3ds.h>
#include <citro3d.h>

#include "render.h"
#include "structs.h"
#include "list.h"
#include "texture.h"

// Used to transfer the final rendered display to the framebuffer
#define DISPLAY_TRANSFER_FLAGS \
    (GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | \
    GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | \
    GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

#define CLEAR_COLOR 0x000000FF

#define SCREEN_WIDTH 400
#define SCREEN_HEIGHT 240

// These headers are generated by the build process
#include "vshader_shbin.h"

static DVLB_s * vshader_dvlb;
static shaderProgram_s program;
static int uLoc_projection;
static C3D_Mtx projection;

static C3D_RenderTarget * top_left;
static C3D_RenderTarget * top_right;

static list_t * render_pipe = NULL;
static uint8_t render_pipe_size = 0;

static void draw_quad(float x1, float y1, float x2, float y2, float tx1, float ty1, float tx2, float ty2, uint8_t flip_x, uint8_t flip_y)
{
    C3D_ImmDrawBegin(GPU_TRIANGLE_STRIP);

    if (1 == flip_x && 1 == flip_y)
    {
        C3D_ImmSendAttrib(x1, y1, 0.5f, 0.0f);
        C3D_ImmSendAttrib(tx2, ty2, 0.0f, 0.0f);

        C3D_ImmSendAttrib(x1, y2, 0.5f, 0.0f);
        C3D_ImmSendAttrib(tx2, ty1, 0.0f, 0.0f);

        C3D_ImmSendAttrib(x2, y1, 0.5f, 0.0f);
        C3D_ImmSendAttrib(tx1, ty2, 0.0f, 0.0f);

        C3D_ImmSendAttrib(x2, y2, 0.5f, 0.0f);
        C3D_ImmSendAttrib(tx1, ty1, 0.0f, 0.0f);
    }
    else if (1 == flip_x)
    {
        C3D_ImmSendAttrib(x1, y1, 0.5f, 0.0f);
        C3D_ImmSendAttrib(tx1, ty2, 0.0f, 0.0f);

        C3D_ImmSendAttrib(x1, y2, 0.5f, 0.0f);
        C3D_ImmSendAttrib(tx1, ty1, 0.0f, 0.0f);

        C3D_ImmSendAttrib(x2, y1, 0.5f, 0.0f);
        C3D_ImmSendAttrib(tx2, ty2, 0.0f, 0.0f);

        C3D_ImmSendAttrib(x2, y2, 0.5f, 0.0f);
        C3D_ImmSendAttrib(tx2, ty1, 0.0f, 0.0f);
    }
    else if (1 == flip_y)
    {
        C3D_ImmSendAttrib(x1, y1, 0.5f, 0.0f);
        C3D_ImmSendAttrib(tx2, ty1, 0.0f, 0.0f);

        C3D_ImmSendAttrib(x1, y2, 0.5f, 0.0f);
        C3D_ImmSendAttrib(tx2, ty2, 0.0f, 0.0f);

        C3D_ImmSendAttrib(x2, y1, 0.5f, 0.0f);
        C3D_ImmSendAttrib(tx1, ty1, 0.0f, 0.0f);

        C3D_ImmSendAttrib(x2, y2, 0.5f, 0.0f);
        C3D_ImmSendAttrib(tx1, ty2, 0.0f, 0.0f);
    }
    else
    {
        C3D_ImmSendAttrib(x1, y1, 0.5f, 0.0f);
        C3D_ImmSendAttrib(tx1, ty1, 0.0f, 0.0f);

        C3D_ImmSendAttrib(x1, y2, 0.5f, 0.0f);
        C3D_ImmSendAttrib(tx1, ty2, 0.0f, 0.0f);

        C3D_ImmSendAttrib(x2, y1, 0.5f, 0.0f);
        C3D_ImmSendAttrib(tx2, ty1, 0.0f, 0.0f);

        C3D_ImmSendAttrib(x2, y2, 0.5f, 0.0f);
        C3D_ImmSendAttrib(tx2, ty2, 0.0f, 0.0f);
    }

    C3D_ImmDrawEnd();
}

static void render_sprite(sprite_t const * sprite, texture_t const ** gpu_texture, float offset3d)
{
    texture_t const * texture = sprite->texture;
    if (NULL != texture)
    {
        if (texture != *gpu_texture)
        {
            *gpu_texture = texture;
            C3D_TexBind(0, &((texture_t *)texture)->ptr);
        }

        frame_t const * frame = sprite->frame;
        if (NULL != frame)
        {
            float left = sprite->x + offset3d;
            float top = sprite->y;
            float right = left + frame->width;
            float bottom = top + frame->height;

            draw_quad(
                left, top, right, bottom,
                frame->left, frame->top, frame->right, frame->bottom,
                sprite->flip_x, sprite->flip_y
            );
        }
    }
}

int init_rendering(surface_t * screen)
{
    // Initialize graphics
    gfxInitDefault();
    gfxSet3D(true);

    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);

    consoleInit(GFX_BOTTOM, NULL);

    render_pipe = list_new(sizeof(sprite_t *), 1);
    if (NULL == render_pipe)
    {
        return 1;
    }
    else if (!init_textures())
    {
        return 2;
    }

    // Initialize the render target
    top_left = C3D_RenderTargetCreate(SCREEN_HEIGHT, SCREEN_WIDTH, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    C3D_RenderTargetSetClear(top_left, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
    C3D_RenderTargetSetOutput(top_left, GFX_TOP, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);

    top_right = C3D_RenderTargetCreate(SCREEN_HEIGHT, SCREEN_WIDTH, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    C3D_RenderTargetSetClear(top_right, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
    C3D_RenderTargetSetOutput(top_right, GFX_TOP, GFX_RIGHT, DISPLAY_TRANSFER_FLAGS);

    // Initialize the scene
    // Load the vertex shader, create a shader program and bind it
    vshader_dvlb = DVLB_ParseFile((u32 *)vshader_shbin, vshader_shbin_size);
    shaderProgramInit(&program);
    shaderProgramSetVsh(&program, &vshader_dvlb->DVLE[0]);
    C3D_BindProgram(&program);

    // Get the location of the uniforms
    uLoc_projection = shaderInstanceGetUniformLocation(program.vertexShader, "projection");

    // Configure attributes for use with the vertex shader
    // Attribute format and element count are ignored in immediate mode
    C3D_AttrInfo * attrInfo = C3D_GetAttrInfo();
    AttrInfo_Init(attrInfo);
    AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // v0=position
    AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 2); // v2=texcoord

    // Compute the projection matrix
    // Note: we're setting top to 240 here so origin is at top left.
    Mtx_OrthoTilt(&projection, 0.0, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, 0.0, 0.0, 1.0);

    // Configure the first fragment shading substage to just pass through the texture color
    // See https://www.opengl.org/sdk/docs/man2/xhtml/glTexEnv.xml for more insight
    C3D_TexEnv * env = C3D_GetTexEnv(0);
    C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, 0, 0);
    C3D_TexEnvOp(env, C3D_Both, 0, 0, 0);
    C3D_TexEnvFunc(env, C3D_Both, GPU_REPLACE);

    // Configure depth test to overwrite pixels with the same depth (needed to draw overlapping sprites)
    C3D_DepthTest(true, GPU_GEQUAL, GPU_WRITE_ALL);

    // Update the uniforms
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, &projection);

    screen->width = SCREEN_WIDTH;
    screen->height = SCREEN_HEIGHT;

    return 0;
}

void shutdown_rendering()
{
    // Deinitialize the scene
    // Free the shader program
    shaderProgramFree(&program);
    DVLB_Free(vshader_dvlb);

    C3D_RenderTargetDelete(top_left);
    C3D_RenderTargetDelete(top_right);

    shutdown_textures();

    list_delete(&render_pipe);

    // Deinitialize graphics
    C3D_Fini();
    gfxExit();
}

void process_rendering()
{
    texture_t const * gpu_texture = NULL;
    sprite_t const ** sprite = NULL;
    float iod = osGet3DSliderState() * 15;

    // Render the scene
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

    if (iod <= 0.0f)
    {
        iod = 0;
    }

    printf("\x1b[4;0Hrender: %4d", render_pipe_size);

    int i = 6;
    C3D_FrameDrawOn(top_left);
    while (list_next(render_pipe, (void **)&sprite))
    {
        render_sprite(*sprite, &gpu_texture, -iod);
        printf("\x1b[%d;0H* %p at %4d %4d", i, *sprite, (*sprite)->x, (*sprite)->y);
        ++i;
    }

    for (; i < 28; ++i)
    {
        printf("\x1b[%d;0H%40s", i, " ");
    }

    if (iod > 0.0f)
    {
        sprite = NULL;
        C3D_FrameDrawOn(top_right);
        while (list_next(render_pipe, (void **)&sprite))
        {
            render_sprite(*sprite, &gpu_texture, iod);
        }
    }

    C3D_FrameEnd(0);

    gpu_texture = NULL;
}

int add_to_rendering(sprite_t * sprite)
{
    if (NULL == sprite)
    {
        return 0;
    }

    sprite_t ** ptr = NULL;
    while (list_next(render_pipe, (void **)&ptr))
    {
        if (*ptr == sprite)
        {
            return 0;
        }
    }

    ptr = (sprite_t **)list_alloc(render_pipe);
    if (NULL == ptr)
    {
        return 0;
    }
    *ptr = sprite;
    ++render_pipe_size;
    return 1;
}

int remove_from_rendering(sprite_t * sprite)
{
    if (NULL == sprite)
    {
        return 0;
    }

    sprite_t ** ptr = NULL;
    while (list_next(render_pipe, (void **)&ptr))
    {
        if (*ptr == sprite)
        {
            list_dealloc(render_pipe, ptr);
            --render_pipe_size;
            return 1;
        }
    }
    return 0;
}