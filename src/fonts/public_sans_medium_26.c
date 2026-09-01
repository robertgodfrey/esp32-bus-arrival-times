/*******************************************************************************
 * Size: 26 px
 * Bpp: 4
 * Opts: --bpp 4 --size 26 --no-compress --stride 1 --align 1 --font PublicSans-Medium.ttf --symbols min --format lvgl -o public_sans_medium_26.c
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



#ifndef PUBLIC_SANS_MEDIUM_26
#define PUBLIC_SANS_MEDIUM_26 1
#endif

#if PUBLIC_SANS_MEDIUM_26

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0069 "i" */
    0x1f, 0xfb, 0x1f, 0xfb, 0x1c, 0xc8, 0x0, 0x0,
    0x0, 0x0, 0xf, 0xfa, 0xf, 0xfa, 0xf, 0xfa,
    0xf, 0xfa, 0xf, 0xfa, 0xf, 0xfa, 0xf, 0xfa,
    0xf, 0xfa, 0xf, 0xfa, 0xf, 0xfa, 0xf, 0xfa,
    0xf, 0xfa, 0xf, 0xfa, 0xf, 0xfa,

    /* U+006D "m" */
    0x1f, 0xf9, 0x8, 0xdf, 0xe9, 0x10, 0x7, 0xdf,
    0xe9, 0x10, 0x1, 0xff, 0xac, 0xff, 0xff, 0xfe,
    0x3c, 0xff, 0xff, 0xfe, 0x10, 0x1f, 0xff, 0xf8,
    0x69, 0xff, 0xff, 0xf9, 0x68, 0xef, 0xf9, 0x1,
    0xff, 0xf2, 0x0, 0x5, 0xff, 0xf4, 0x0, 0x3,
    0xff, 0xe0, 0x1f, 0xfb, 0x0, 0x0, 0xf, 0xfd,
    0x0, 0x0, 0xd, 0xff, 0x1, 0xff, 0xa0, 0x0,
    0x0, 0xff, 0xd0, 0x0, 0x0, 0xcf, 0xf0, 0x1f,
    0xfa, 0x0, 0x0, 0xf, 0xfd, 0x0, 0x0, 0xc,
    0xff, 0x1, 0xff, 0xa0, 0x0, 0x0, 0xff, 0xd0,
    0x0, 0x0, 0xcf, 0xf0, 0x1f, 0xfa, 0x0, 0x0,
    0xf, 0xfd, 0x0, 0x0, 0xc, 0xff, 0x1, 0xff,
    0xa0, 0x0, 0x0, 0xff, 0xd0, 0x0, 0x0, 0xcf,
    0xf0, 0x1f, 0xfa, 0x0, 0x0, 0xf, 0xfd, 0x0,
    0x0, 0xc, 0xff, 0x1, 0xff, 0xa0, 0x0, 0x0,
    0xff, 0xd0, 0x0, 0x0, 0xcf, 0xf0, 0x1f, 0xfa,
    0x0, 0x0, 0xf, 0xfd, 0x0, 0x0, 0xc, 0xff,
    0x1, 0xff, 0xa0, 0x0, 0x0, 0xff, 0xd0, 0x0,
    0x0, 0xcf, 0xf0,

    /* U+006E "n" */
    0x1f, 0xf9, 0x19, 0xef, 0xeb, 0x30, 0x1, 0xff,
    0xbe, 0xff, 0xff, 0xff, 0x50, 0x1f, 0xff, 0xd5,
    0x23, 0x9f, 0xfe, 0x1, 0xff, 0xe1, 0x0, 0x0,
    0xbf, 0xf4, 0x1f, 0xfb, 0x0, 0x0, 0x6, 0xff,
    0x61, 0xff, 0xa0, 0x0, 0x0, 0x5f, 0xf6, 0x1f,
    0xfa, 0x0, 0x0, 0x5, 0xff, 0x61, 0xff, 0xa0,
    0x0, 0x0, 0x5f, 0xf6, 0x1f, 0xfa, 0x0, 0x0,
    0x5, 0xff, 0x61, 0xff, 0xa0, 0x0, 0x0, 0x5f,
    0xf6, 0x1f, 0xfa, 0x0, 0x0, 0x5, 0xff, 0x61,
    0xff, 0xa0, 0x0, 0x0, 0x5f, 0xf6, 0x1f, 0xfa,
    0x0, 0x0, 0x5, 0xff, 0x61, 0xff, 0xa0, 0x0,
    0x0, 0x5f, 0xf6
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 105, .box_w = 4, .box_h = 19, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 38, .adv_w = 363, .box_w = 21, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 185, .adv_w = 242, .box_w = 13, .box_h = 14, .ofs_x = 1, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint8_t glyph_id_ofs_list_0[] = {
    0, 0, 0, 0, 1, 2
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 105, .range_length = 6, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = glyph_id_ofs_list_0, .list_length = 6, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL
    }
};

/*-----------------
 *    KERNING
 *----------------*/


/*Map glyph_ids to kern left classes*/
static const uint8_t kern_left_class_mapping[] =
{
    0, 1, 0, 0
};

/*Map glyph_ids to kern right classes*/
static const uint8_t kern_right_class_mapping[] =
{
    0, 1, 1, 1
};

/*Kern values between classes*/
static const int8_t kern_class_values[] =
{
    2
};


/*Collect the kern class' data in one place*/
static const lv_font_fmt_txt_kern_classes_t kern_classes =
{
    .class_pair_values   = kern_class_values,
    .left_class_mapping  = kern_left_class_mapping,
    .right_class_mapping = kern_right_class_mapping,
    .left_class_cnt      = 1,
    .right_class_cnt     = 1,
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
    .kern_dsc = &kern_classes,
    .kern_scale = 16,
    .cmap_num = 1,
    .bpp = 4,
    .kern_classes = 1,
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
const lv_font_t public_sans_medium_26 = {
#else
lv_font_t public_sans_medium_26 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 19,          /*The maximum line height required by the font*/
    .base_line = 0,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -5,
    .underline_thickness = 2,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if PUBLIC_SANS_MEDIUM_26*/
