#include <stdlib.h>
#include <stdio.h>

#include <3ds.h>
#include <citro3d.h>

#include "texture.h"
#include "structs.h"
#include "json_wrapper.h"
#include "utils.h"
#include "list.h"

#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Used to convert textures to 3DS tiled format
// Note: vertical flip flag set so 0,0 is top left of texture
#define TEXTURE_TRANSFER_FLAGS(format_in, format_out) \
    (GX_TRANSFER_FLIP_VERT(1) | GX_TRANSFER_OUT_TILED(1) | GX_TRANSFER_RAW_COPY(0) | \
    GX_TRANSFER_IN_FORMAT(format_in) | GX_TRANSFER_OUT_FORMAT(format_out) | \
    GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

static List * textures = NULL;

static uint8_t is_pow_2(uint32_t i)
{
    return (i && !(i & (i - 1)))
        ? 1 : 0;
}

static u32 next_pow_2(int i)
{
    i--;
    i |= i >> 1;
    i |= i >> 2;
    i |= i >> 4;
    i |= i >> 8;
    i |= i >> 16;
    i++;

    return (i < 64)
        ? 64 : i;
}

static int load_texture_frames(Texture * texture, const char * name)
{
    int result = 0;
    JsonWrapper * json = json_new(name);
    if (NULL == json || 0 != load_frames(json, texture))
    {
        result = 1;
    }
    json_delete(json);
    return result;
}

static int load_image(Texture * texture, const char * filepath)
{
    // Configure buffers
    // C3D_BufInfo * bufInfo = C3D_GetBufInfo();
    // BufInfo_Init(bufInfo);

    int width, height;
    int bpp;

    unsigned char * image = stbi_load(filepath, &width, &height, &bpp, 0);
    if (NULL == image)
    {
        return 1;
    }

    u32 pow2Width = !is_pow_2(width) ? next_pow_2(width) : width;
    u32 pow2Height = !is_pow_2(height) ? next_pow_2(height) : height;

    u32 texture_size = pow2Width * pow2Height * bpp;
    u8 * gpusrc = linearAlloc(texture_size);
    if (!gpusrc)
    {
        printf("Failed to alloc %lu", texture_size);
        return 2;
    }
    memset(gpusrc, 0, texture_size);

    // GX_DisplayTransfer needs input buffer in linear RAM
    // lodepng outputs big endian rgba so we need to convert
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            u32 imagePos = (y * width + x) * bpp;
            u32 gpuTexPos = (y * pow2Width + x) * bpp;

            for (int c = 0; c < bpp; ++c)
            {
                gpusrc[gpuTexPos + c] = image[imagePos + (bpp - c - 1)];
            }
        }
    }

    // ensure data is in physical ram
    GSPGPU_FlushDataCache(gpusrc, texture_size);

    GPU_TEXCOLOR tex_color = 3 == bpp
        ? GPU_RGB8
        : GPU_RGBA8;

    u32 flags = 3 == bpp
        ? TEXTURE_TRANSFER_FLAGS(GX_TRANSFER_FMT_RGB8, GX_TRANSFER_FMT_RGB8)
        : TEXTURE_TRANSFER_FLAGS(GX_TRANSFER_FMT_RGBA8, GX_TRANSFER_FMT_RGBA8);

    C3D_TexInit(&texture->ptr, pow2Width, pow2Height, tex_color);

    // Convert image to 3DS tiled texture format
    u32 dim = GX_BUFFER_DIM(pow2Width, pow2Height);
    GX_DisplayTransfer(
        (u32 *)gpusrc, dim,
        (u32 *)texture->ptr.data, dim,
        flags
    );
    gspWaitForPPF();

    C3D_TexSetFilter(&texture->ptr, GPU_LINEAR, GPU_NEAREST);

    stbi_image_free(image);
    linearFree(gpusrc);

    texture->width = width;
    texture->height = height;
    texture->real_width = pow2Width;
    texture->real_height = pow2Height;
    texture->bpp = bpp;
    texture->name = NULL;
    texture->frames = NULL;
    return 0;
}

int init_textures()
{
    textures = list_new(sizeof(Texture), 2);

    return NULL != textures
        ? 0 : 1;
}

void unload_textures()
{
    Texture * texture = NULL;
    while (list_next(textures, (void **)&texture))
    {
        C3D_TexDelete(&texture->ptr);

        free(texture->name);
        free((Frame *)texture->frames);
    }
    list_delete(textures);
    textures = NULL;
}

Texture const * load_texture(const char * name)
{
    Texture * addr = (Texture *)list_alloc(textures);

    int abort = 0;
    if (0 == strncmp(name, "rtype", 5))
    {
        if (0 != load_image(addr, "data/rtype.png"))
        {
            abort = 1;
        }
        else if (0 != load_texture_frames(addr, "rtype_frames"))
        {
            C3D_TexDelete(&addr->ptr);
            abort = 1;
        }
    }
    else if (0 == strncmp(name, "background", 10))
    {
        if (0 != load_image(addr, "data/background.jpg"))
        {
            abort = 1;
        }
    }

    if (abort)
    {
        list_dealloc(textures, addr);
        return NULL;
    }

    addr->name = strdup(name);
    return addr;
}

Texture const * get_texture(const char * name)
{
    Texture const * texture = NULL;
    while (list_next(textures, (void **)&texture))
    {
        if (NULL != name && NULL != texture->name && 0 == strcmp(texture->name, name))
        {
            return texture;
        }
    }
    return NULL;
}

Frame const * get_frame(Texture const * texture, int index)
{
    if (NULL != texture && NULL != texture->frames)
    {
        return &(texture->frames[index]);
    }
    return NULL;
}