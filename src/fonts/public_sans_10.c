/*******************************************************************************
 * Size: 10 px
 * Bpp: 4
 * Opts: --bpp 4 --size 10 --no-compress --stride 1 --align 1 --font PublicSans-Regular.ttf --symbols (sched.) --format lvgl -o public_sans_10.c
 ******************************************************************************/

#ifdef __has_include
    #if __has_include("lvgl.h")
        #ifndef LV_LVGL_H_INCLUDE_SIMPLE
            #define LV_LVGL_H_INCLUDE_SIMPLE
        #endif
    #endif
#endif

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif



#ifndef PUBLIC_SANS_10
#define PUBLIC_SANS_10 1
#endif

#if PUBLIC_SANS_10

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0028 "(" */
    0x3, 0x80, 0xb1, 0x1c, 0x4, 0xa0, 0x49, 0x3,
    0xb0, 0xd, 0x0, 0x74, 0x0, 0x40,

    /* U+0029 ")" */
    0x82, 0x2, 0xa0, 0xd, 0x0, 0xb3, 0xa, 0x30,
    0xb2, 0xc, 0x4, 0x70, 0x40, 0x0,

    /* U+002E "." */
    0x1, 0x7a,

    /* U+0063 "c" */
    0x9, 0xbc, 0x45, 0x90, 0x8, 0x85, 0x0, 0x5,
    0x90, 0x7, 0x9, 0xbb, 0x40,

    /* U+0064 "d" */
    0x0, 0x0, 0xb3, 0x0, 0x0, 0xb3, 0xb, 0xb9,
    0xd3, 0x69, 0x0, 0xe3, 0x86, 0x0, 0xb3, 0x69,
    0x0, 0xe3, 0xa, 0xba, 0xa3,

    /* U+0065 "e" */
    0x9, 0xbb, 0x50, 0x4a, 0x0, 0xd0, 0x7c, 0x99,
    0xb1, 0x59, 0x0, 0x60, 0x9, 0xbb, 0x70,

    /* U+0068 "h" */
    0x3a, 0x0, 0x0, 0x3a, 0x0, 0x0, 0x3d, 0xac,
    0x90, 0x3c, 0x0, 0xe1, 0x3b, 0x0, 0xd1, 0x3b,
    0x0, 0xd1, 0x3b, 0x0, 0xd1,

    /* U+0073 "s" */
    0x1b, 0xab, 0x24, 0xa0, 0x23, 0x7, 0xba, 0x23,
    0x30, 0x59, 0x1b, 0xab, 0x30
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 48, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 14, .adv_w = 49, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 28, .adv_w = 36, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 30, .adv_w = 87, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 43, .adv_w = 95, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 64, .adv_w = 91, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 79, .adv_w = 92, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 100, .adv_w = 81, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_0[] = {
    0x0, 0x1, 0x6, 0x3b, 0x3c, 0x3d, 0x40, 0x4b
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 40, .range_length = 76, .glyph_id_start = 1,
        .unicode_list = unicode_list_0, .glyph_id_ofs_list = NULL, .list_length = 8, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
};

/*-----------------
 *    KERNING
 *----------------*/


/*Pair left and right glyphs for kerning*/
static const uint8_t kern_pair_glyph_ids[] =
{
    3, 3,
    4, 2,
    4, 4,
    4, 5,
    4, 6,
    4, 7,
    4, 8,
    5, 7,
    6, 2,
    6, 8,
    7, 8,
    8, 2,
    8, 8
};

/* Kerning between the respective left and right glyphs
 * 4.4 format which needs to scaled with `kern_scale`*/
static const int8_t kern_pair_values[] =
{
    8, -2, -1, -1, -1, -1, -1, 1,
    -2, -1, -1, -1, 0
};

/*Collect the kern pair's data in one place*/
static const lv_font_fmt_txt_kern_pair_t kern_pairs =
{
    .glyph_ids = kern_pair_glyph_ids,
    .values = kern_pair_values,
    .pair_cnt = 13,
    .glyph_ids_size = 0
};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = &kern_pairs,
    .kern_scale = 16,
    .cmap_num = 1,
    .bpp = 4,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif

};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t public_sans_10 = {
#else
lv_font_t public_sans_10 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 9,          /*The maximum line height required by the font*/
    .base_line = 2,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -2,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if PUBLIC_SANS_10*/
