#include <stdlib.h>
#include <stdio.h>

#include <3ds.h>
#include <citro3d.h>

#include "textures.h"
#include "structs.h"
#include "json_wrapper.h"

#ifdef CITRA
#define STBI_NO_STDIO
#endif
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "utils.h"

#ifdef CITRA
    #include "rtype_png.h"
    #include "background_jpg.h"
#endif

// Used to convert textures to 3DS tiled format
// Note: vertical flip flag set so 0,0 is top left of texture
#define TEXTURE_TRANSFER_FLAGS(format_in, format_out) \
    (GX_TRANSFER_FLIP_VERT(1) | GX_TRANSFER_OUT_TILED(1) | GX_TRANSFER_RAW_COPY(0) | \
    GX_TRANSFER_IN_FORMAT(format_in) | GX_TRANSFER_OUT_FORMAT(format_out) | \
    GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

static Texture * textures;
static int nb_textures;

#ifdef CITRA
static int init_texture(Texture * texture, uint8_t const * data, uint32_t size)
#else
static int init_texture(Texture * texture, const char * filepath)
#endif
{
    // Configure buffers
    // C3D_BufInfo * bufInfo = C3D_GetBufInfo();
    // BufInfo_Init(bufInfo);

    int width, height;
    int bpp;

#ifdef CITRA
    unsigned char * image = stbi_load_from_memory(data, size, &width, &height, &bpp, 0);
#else
    unsigned char * image = stbi_load(filepath, &width, &height, &bpp, 0);
#endif

    u32 pow2Width = !is_pow_2(width) ? next_pow_2(width) : width;
    u32 pow2Height = !is_pow_2(height) ? next_pow_2(height) : height;

    u32 texture_size = pow2Width * pow2Height * bpp;
    u8 * gpusrc = linearAlloc(texture_size);
    if (!gpusrc)
    {
        printf("Failed to alloc %lu", texture_size);
        return 1;
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
    texture->frames = NULL;
    return 0;
}

int load_textures()
{
    nb_textures = 2;
    textures = malloc(sizeof(Texture) * nb_textures);

#ifdef CITRA
    if (0 != init_texture(&textures[0], rtype_png, rtype_png_size))
#else
    if (0 != init_texture(&textures[0], "data/rtype.png"))
#endif
    {
        printf("Failed to load texture 'rtype'");
        return 1;
    }

    int abort = 0;
    JsonWrapper * json = json_create("rtype");
    if (NULL == json || 0 != load_frames(json, &textures[0]))
    {
        abort = 1;
    }
    json_delete(json);

    if (abort)
    {
        return 1;
    }

#ifdef CITRA
    if (0 != init_texture(&textures[1], background_jpg, background_jpg_size))
#else
    if (0 != init_texture(&textures[1], "data/background.jpg"))
#endif
    {
        printf("Failed to load texture 'background'");
        return 1;
    }
    return 0;
}

void unload_textures()
{
    for (int i = 0; i < nb_textures; ++i)
    {
        C3D_TexDelete(&textures[i].ptr);

        free((Frame *)textures[i].frames);
    }
    free(textures);
    textures = NULL;
}

Texture const * get_texture(const char * name)
{
    if (0 == strncmp(name, "rtype", 11))
    {
        return &textures[0];
    }
    else if (0 == strncmp(name, "background", 10))
    {
        return &textures[1];
    }
    return NULL;
}

Frame const * get_frame(Texture const * texture, int index)
{
    if (NULL != texture)
    {
        return &(texture->frames[index]);
    }
    return NULL;
}