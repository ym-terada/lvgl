/**
 * @file lv_style.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_obj.h"
#include "../lv_misc/lv_mem.h"
#include "../lv_misc/lv_anim.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void set_prop(lv_style_t * style, lv_style_prop_t prop, lv_style_value_t value);
static bool get_prop(const lv_style_t * style, lv_style_prop_t prop, lv_style_value_t * value);
static bool remove_prop(lv_style_t * style, lv_style_prop_t prop);

/**********************
 *  GLOBAL VARIABLES
 **********************/
lv_style_class_t lv_style;

/**********************
 *  STATIC VARIABLES
 **********************/
static int16_t buf_num[32];
static const void * buf_ptr[16];
static lv_color_t buf_color[16];
static uint32_t buf_num_p = 1;
static uint32_t buf_color_p = 1;
static uint32_t buf_ptr_p = 1;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void _lv_style_system_init(void)
{
    lv_style.remove_prop = remove_prop;
    lv_style.set_prop = set_prop;
    lv_style.get_prop = get_prop;
}

/**
 * Initialize a style
 * @param style pointer to a style to initialize
 */
void lv_style_init(lv_style_t * style)
{
    _lv_memset_00(style, sizeof(lv_style_t));
    style->class_p = &lv_style;
#if LV_USE_ASSERT_STYLE
    style->sentinel = LV_DEBUG_STYLE_SENTINEL_VALUE;
#endif

}

uint16_t lv_style_register_prop(bool inherit)
{
    static uint16_t act_id = (uint16_t)_LV_STYLE_LAST_BUILT_IN_PROP;
    act_id++;
    if(inherit) return act_id | LV_STYLE_PROP_INHERIT;
    else return act_id;
}

/**
 * Remove a property from a style
 * @param style pointer to a style
 * @param prop  a style property ORed with a state.
 * E.g. `LV_STYLE_BORDER_WIDTH | (LV_STATE_PRESSED << LV_STYLE_STATE_POS)`
 * @return true: the property was found and removed; false: the property wasn't found
 */
bool lv_style_remove_prop(lv_style_t * style, lv_style_prop_t prop)
{
    return style->class_p->remove_prop(style, prop);
}

/**
 * Clear all properties from a style and all allocated memories.
 * @param style pointer to a style
 */
void lv_style_reset(lv_style_t * style)
{
    LV_ASSERT_STYLE(style);
    lv_mem_free(style->ext);
    lv_style_init(style);
}

void _alloc_ext(lv_style_t * style)
{
    if(style->ext) return;
    style->ext = lv_mem_alloc(sizeof(lv_style_ext_t));
    LV_ASSERT_MEM(style->ext);
    _lv_memset_00(style->ext, sizeof(lv_style_ext_t));
}

void lv_style_set_prop(lv_style_t * style, lv_style_prop_t prop, lv_style_value_t value)
{
    style->class_p->set_prop(style, prop, value);
}

bool lv_style_get_prop(lv_style_t * style, lv_style_prop_t prop, lv_style_value_t * value)
{
    return style->class_p->get_prop(style, prop, value);
}

void lv_style_transition_init(lv_style_transiton_t * tr, const lv_style_prop_t * props, const lv_anim_path_t * path, uint32_t time, uint32_t delay)
{
    _lv_memset_00(tr, sizeof(lv_style_transiton_t));
    tr->props = props;
    tr->path = path;
    tr->time = time;
    tr->delay = delay;
}

lv_style_value_t lv_style_prop_get_default(lv_style_prop_t prop)
{
    lv_style_value_t value;
    switch(prop) {
            break;
        case LV_STYLE_TRANSFORM_ZOOM:
            value.num = LV_IMG_ZOOM_NONE;
            break;
        case LV_STYLE_BG_COLOR:
            value.color = LV_COLOR_WHITE;
            break;
        case LV_STYLE_OPA:
        case LV_STYLE_BORDER_OPA:
        case LV_STYLE_TEXT_OPA:
        case LV_STYLE_IMG_OPA:
        case LV_STYLE_LINE_OPA:
        case LV_STYLE_OUTLINE_OPA:
        case LV_STYLE_SHADOW_OPA:
            value.num = LV_OPA_COVER;
            break;
        case LV_STYLE_BG_GRAD_STOP:
            value.num = 255;
            break;
        case LV_STYLE_BORDER_SIDE:
            value.num = LV_BORDER_SIDE_FULL;
            break;
        case LV_STYLE_TEXT_FONT:
            value.ptr = LV_THEME_DEFAULT_FONT_NORMAL;
            break;
        default:
            value.ptr = NULL;
            value.num = 0;
            break;
    }

    return value;
}

uint32_t lv_style_find_index_num(lv_style_value_t v)
{
    uint32_t i;
    for(i = 1; i < buf_num_p; i++) {
        if(v.num == buf_num[i])  return i;
    }
    return 0;
}

uint32_t lv_style_find_index_color(lv_style_value_t v)
{
    uint32_t i;
    for(i = 1; i < buf_color_p; i++) {
        if(v.color.full == buf_color[i].full)  return i;
    }
    return 0;
}

uint32_t lv_style_find_index_ptr(lv_style_value_t v)
{
    uint32_t i;
    for(i = 1; i < buf_ptr_p; i++) {
        if(v.ptr == buf_ptr[i])  return i;
    }
    return 0;
}

int32_t lv_style_get_indexed_num(uint32_t id)
{
    return buf_num[id];
}


lv_color_t lv_style_get_indexed_color(uint32_t id)
{
    return buf_color[id];
}

const void * lv_style_get_indexed_ptr(uint32_t id)
{
    return buf_ptr[id];
}



/**
 * Check whether a style is valid (initialized correctly)
 * @param style pointer to a style
 * @return true: valid
 */
bool lv_debug_check_style(const lv_style_t * style)
{
    if(style == NULL) return true;  /*NULL style is still valid*/

#if LV_USE_ASSERT_STYLE
    if(style->sentinel != LV_DEBUG_STYLE_SENTINEL_VALUE) {
        LV_LOG_WARN("Invalid style (local variable or not initialized?)");
        return false;
    }
#endif

    return true;
}

/**
 * Check whether a style list is valid (initialized correctly)
 * @param style pointer to a style
 * @return true: valid
 */
bool lv_debug_check_style_list(const void * list)
{
    return true;
}

bool lv_style_is_empty(const lv_style_t * style)
{

    if(style->has_bg_grad_dir) return false;
    if(style->has_border_post) return false;
    if(style->has_clip_corner) return false;
    if(style->has_line_rounded) return false;
    if(!style->ext) return false;

    size_t s = sizeof(style->ext->has);
    const uint8_t * has =  (const uint8_t *)&style->ext->has;
    uint32_t i;
    for(i = 0; i < s; i++) {
        if(has[i]) return false;
    }

    return true;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static int32_t alloc_index_num(lv_style_value_t v)
{
    uint32_t i;
    for(i = 1; i < buf_num_p; i++) {
        if(v.num == buf_num[i])  return i;
    }
    if(buf_num_p < 32) {
        buf_num[buf_num_p] = v.num;
        buf_num_p++;
        return buf_num_p - 1;
    }
    return 0;
}

static int32_t alloc_index_ptr(lv_style_value_t v)
{
    uint32_t i;
    for(i = 1; i < buf_ptr_p; i++) {
        if(v.ptr == buf_ptr[i])  return i;
    }
    if(buf_ptr_p < 16) {
        buf_ptr[buf_ptr_p] = v.ptr;
        buf_ptr_p++;
        return buf_ptr_p - 1;
    }
    return 0;
}

static int32_t alloc_index_color(lv_style_value_t v)
{
    uint32_t i;
    for(i = 1; i < buf_color_p; i++) {
        if(v.color.full == buf_color[i].full)  return i;
    }
    if(buf_color_p) {
        buf_color[buf_color_p].full = v.color.full;
        buf_color_p++;
        return buf_color_p - 1;
    }
    return 0;
}


static void set_prop(lv_style_t * style, lv_style_prop_t prop, lv_style_value_t value)
{
    if(style == NULL) return;
     LV_ASSERT_STYLE(style);
     int32_t id;
     switch(prop) {

     case LV_STYLE_RADIUS:
         id = style->dont_index ? 0 : alloc_index_num(value);
         if(id > 0) {
             style->radius = id;
         } else {
             _alloc_ext(style);
             style->ext->radius = value.num;
             style->ext->has.radius = 1;
             style->radius = 0;
         }
         break;
     case LV_STYLE_CLIP_CORNER:
         style->clip_corner = value.num;
         style->has_clip_corner = 1;
         break;
     case LV_STYLE_TRANSFORM_WIDTH:
         id= style->dont_index ? 0 : alloc_index_num(value);
         if(id > 0) {
             style->transform_width = id;
         } else {
             _alloc_ext(style);
             style->ext->transform_width = value.num;
             style->ext->has.transform_width = 1;
             style->transform_width = 0;
         }
         break;
     case LV_STYLE_TRANSFORM_HEIGHT:
         id= style->dont_index ? 0 : alloc_index_num(value);
         if(id > 0) {
             style->transform_height = id;
         } else {
             _alloc_ext(style);
             style->ext->transform_height = value.num;
             style->ext->has.transform_height = 1;
             style->transform_height = 0;
         }
         break;
     case LV_STYLE_TRANSFORM_ANGLE:
         _alloc_ext(style);
         style->ext->transform_angle = value.num;
         style->ext->has.transform_angle = 1;
         break;
     case LV_STYLE_TRANSFORM_ZOOM:
         _alloc_ext(style);
         style->ext->transform_zoom = value.num;
         style->ext->has.transform_zoom = 1;
         break;
     case LV_STYLE_TRANSITION:
         id= style->dont_index ? 0 : alloc_index_ptr(value);
         if(id > 0) {
             style->transition = id;
         } else {
             _alloc_ext(style);
             style->ext->transition = value.ptr;
             style->ext->has.transition = 1;
             style->transition = 0;
         }
         break;
     case LV_STYLE_OPA:
         id= style->dont_index ? 0 : alloc_index_num(value);
         if(id > 0) {
             style->opa = id;
         } else {
             _alloc_ext(style);
             style->ext->opa = value.num;
             style->ext->has.opa = 1;
             style->opa = 0;
         }
         break;
     case LV_STYLE_COLOR_FILTER_CB:
         id= style->dont_index ? 0 : alloc_index_ptr(value);
         if(id > 0) {
             style->color_filter_cb = id;
         } else {
             _alloc_ext(style);
             style->ext->color_filter_cb = (lv_color_filter_cb_t)value.func;
             style->ext->has.color_filter_cb = 1;
             style->color_filter_cb = 0;
         }
         break;
     case LV_STYLE_COLOR_FILTER_OPA:
         id= style->dont_index ? 0 : alloc_index_num(value);
         if(id > 0) {
             style->color_filter_opa = id;
         } else {
             _alloc_ext(style);
             style->ext->color_filter_opa = value.num;
             style->ext->has.color_filter_opa = 1;
             style->color_filter_opa = 0;
         }
         break;

     case LV_STYLE_PAD_TOP:
         id= style->dont_index ? 0 : alloc_index_num(value);
         if(id > 0) {
             style->pad_top = id;
         } else {
             _alloc_ext(style);
             style->ext->pad_top = value.num;
             style->ext->has.pad_top = 1;
             style->pad_top = 0;
         }
         break;
     case LV_STYLE_PAD_BOTTOM:
         id= style->dont_index ? 0 : alloc_index_num(value);
         if(id > 0) {
             style->pad_bottom = id;
         } else {
             _alloc_ext(style);
             style->ext->pad_bottom = value.num;
             style->ext->has.pad_bottom = 1;
             style->pad_bottom = 0;
         }
         break;
     case LV_STYLE_PAD_LEFT:
         id= style->dont_index ? 0 : alloc_index_num(value);
         if(id > 0) {
             style->pad_left = id;
         } else {
             _alloc_ext(style);
             style->ext->pad_left = value.num;
             style->ext->has.pad_left = 1;
             style->pad_left = 0;
         }
         break;
     case LV_STYLE_PAD_RIGHT:
         id= style->dont_index ? 0 : alloc_index_num(value);
         if(id > 0) {
             style->pad_right = id;
         } else {
             _alloc_ext(style);
             style->ext->pad_right = value.num;
             style->ext->has.pad_right = 1;
             style->pad_right = 0;
         }
         break;
     case LV_STYLE_PAD_ROW:
         _alloc_ext(style);
         style->ext->pad_row = value.num;
         style->ext->has.pad_row = 1;
         break;
     case LV_STYLE_PAD_COLUMN:
         _alloc_ext(style);
         style->ext->pad_column = value.num;
         style->ext->has.pad_column = 1;
         break;

     case LV_STYLE_BG_COLOR:
         id= style->dont_index ? 0 : alloc_index_color(value);
         if(id > 0) {
             style->bg_color = id;
         } else {
             _alloc_ext(style);
             style->ext->bg_color = value.color;
             style->ext->has.bg_color = 1;
             style->bg_color = 0;
         }
         break;
     case LV_STYLE_BG_OPA:
         id= style->dont_index ? 0 : alloc_index_num(value);
         if(id > 0) {
             style->bg_opa = id;
         } else {
             _alloc_ext(style);
             style->ext->bg_opa = value.num;
             style->ext->has.bg_opa = 1;
             style->bg_opa = 0;
         }
         break;
     case LV_STYLE_BG_GRAD_COLOR:
         id= style->dont_index ? 0 : alloc_index_color(value);
         if(id > 0) {
             style->bg_grad_color = id;
         } else {
             _alloc_ext(style);
             style->ext->bg_grad_color = value.color;
             style->ext->has.bg_grad_color = 1;
             style->bg_grad_color = 0;
         }
         break;
     case LV_STYLE_BG_GRAD_DIR:
         style->bg_grad_dir = value.num;
         style->has_bg_grad_dir = 1;
         break;
     case LV_STYLE_BG_BLEND_MODE:
         _alloc_ext(style);
         style->ext->bg_blend_mode = value.num;
         style->ext->has.bg_blend_mode = 1;
         break;
     case LV_STYLE_BG_MAIN_STOP:
         _alloc_ext(style);
         style->ext->bg_main_stop = value.num;
         style->ext->has.bg_main_stop = 1;
         break;
     case LV_STYLE_BG_GRAD_STOP:
         _alloc_ext(style);
         style->ext->bg_grad_stop = value.num;
         style->ext->has.bg_grad_stop = 1;
         break;

     case LV_STYLE_BORDER_COLOR:
         id= style->dont_index ? 0 : alloc_index_color(value);
         if(id > 0) {
             style->border_color = id;
         } else {
             _alloc_ext(style);
             style->ext->border_color = value.color;
             style->ext->has.border_color = 1;
             style->border_color = 0;
         }
         break;
     case LV_STYLE_BORDER_OPA:
         id= style->dont_index ? 0 : alloc_index_num(value);
         if(id > 0) {
             style->border_opa = id;
         } else {
             _alloc_ext(style);
             style->ext->border_opa = value.num;
             style->ext->has.border_opa = 1;
             style->border_opa = 0;
         }
         break;
     case LV_STYLE_BORDER_WIDTH:
         id= style->dont_index ? 0 : alloc_index_num(value);
         if(id > 0) {
             style->border_width = id;
         } else {
             _alloc_ext(style);
             style->ext->border_width = value.num;
             style->ext->has.border_width = 1;
             style->border_width = 0;
         }
         break;
     case LV_STYLE_BORDER_SIDE:
         _alloc_ext(style);
         style->ext->border_side = value.num;
         style->ext->has.border_side = 1;
         break;
     case LV_STYLE_BORDER_POST:
         style->border_post = value.num;
         style->has_border_post = 1;
         break;
     case LV_STYLE_BORDER_BLEND_MODE:
         _alloc_ext(style);
         style->ext->border_blend_mode = value.num;
         style->ext->has.border_blend_mode = 1;
         break;

     case LV_STYLE_TEXT_COLOR:
         id= style->dont_index ? 0 : alloc_index_color(value);
         if(id > 0) {
             style->text_color = id;
         } else {
             _alloc_ext(style);
             style->ext->text_color = value.color;
             style->ext->has.text_color = 1;
             style->text_color = 0;
         }
         break;
     case LV_STYLE_TEXT_OPA:
         id= style->dont_index ? 0 : alloc_index_num(value);
         if(id > 0) {
             style->text_opa = id;
         } else {
             _alloc_ext(style);
             style->ext->text_opa = value.num;
             style->ext->has.text_opa = 1;
             style->text_opa = 0;
         }
         break;
     case LV_STYLE_TEXT_LETTER_SPACE:
         _alloc_ext(style);
         style->ext->text_letter_space = value.num;
         style->ext->has.text_letter_space = 1;
         break;
     case LV_STYLE_TEXT_LINE_SPACE:
         _alloc_ext(style);
         style->ext->text_line_space = value.num;
         style->ext->has.text_line_space = 1;
         break;
         break;
     case LV_STYLE_TEXT_DECOR:
         _alloc_ext(style);
         style->ext->text_decor = value.num;
         style->ext->has.text_decor = 1;
         break;
     case LV_STYLE_TEXT_BLEND_MODE:
         _alloc_ext(style);
         style->ext->text_blend_mode = value.num;
         style->ext->has.text_blend_mode = 1;
         break;

     case LV_STYLE_IMG_OPA:
         id= style->dont_index ? 0 : alloc_index_num(value);
         if(id > 0) {
             style->img_opa = id;
         } else {
             _alloc_ext(style);
             style->ext->img_opa = value.num;
             style->ext->has.img_opa = 1;
             style->img_opa = 0;
         }
         break;
     case LV_STYLE_IMG_BLEND_MODE:
         _alloc_ext(style);
         style->ext->img_blend_mode = value.num;
         style->ext->has.img_blend_mode = 1;
         break;
     case LV_STYLE_IMG_RECOLOR:
         _alloc_ext(style);
         style->ext->img_recolor = value.color;
         style->ext->has.img_recolor = 1;
         break;
     case LV_STYLE_IMG_RECOLOR_OPA:
         _alloc_ext(style);
         style->ext->img_recolor_opa = value.num;
         style->ext->has.img_recolor_opa= 1;
         break;

     case LV_STYLE_OUTLINE_WIDTH:
         id= style->dont_index ? 0 : alloc_index_num(value);
         if(id > 0) {
             style->outline_width = id;
         } else {
             _alloc_ext(style);
             style->ext->outline_width = value.num;
             style->ext->has.outline_width = 1;
             style->outline_width = 0;
         }
         break;
     case LV_STYLE_OUTLINE_COLOR:
         id= style->dont_index ? 0 : alloc_index_color(value);
         if(id > 0) {
             style->outline_color = id;
         } else {
             _alloc_ext(style);
             style->ext->outline_color = value.color;
             style->ext->has.outline_color = 1;
             style->outline_color = 0;
         }
         break;
     case LV_STYLE_OUTLINE_OPA:
         id= style->dont_index ? 0 : alloc_index_num(value);
         if(id > 0) {
             style->outline_opa = id;
         } else {
             _alloc_ext(style);
             style->ext->outline_opa = value.num;
             style->ext->has.outline_opa = 1;
             style->outline_opa = 0;
         }
         break;
     case LV_STYLE_OUTLINE_PAD:
         id= style->dont_index ? 0 : alloc_index_num(value);
         if(id > 0) {
             style->outline_pad = id;
         } else {
             _alloc_ext(style);
             style->ext->outline_pad = value.num;
             style->ext->has.outline_pad = 1;
             style->outline_pad = 0;
         }
         break;
     case LV_STYLE_OUTLINE_BLEND_MODE:
         _alloc_ext(style);
         style->ext->outline_blend_mode = value.num;
         style->ext->has.outline_blend_mode = 1;
         break;

     case LV_STYLE_SHADOW_WIDTH:
         id= style->dont_index ? 0 : alloc_index_num(value);
         if(id > 0) {
             style->shadow_width = id;
         } else {
             _alloc_ext(style);
             style->ext->shadow_width = value.num;
             style->ext->has.shadow_width = 1;
             style->shadow_width = 0;
         }
         break;
     case LV_STYLE_SHADOW_OFS_X:
         _alloc_ext(style);
         style->ext->shadow_ofs_x = value.num;
         style->ext->has.shadow_ofs_x = 1;
         break;
     case LV_STYLE_SHADOW_OFS_Y:
         _alloc_ext(style);
         style->ext->shadow_ofs_y = value.num;
         style->ext->has.shadow_ofs_y = 1;
         break;
     case LV_STYLE_SHADOW_SPREAD:
         _alloc_ext(style);
         style->ext->shadow_spread = value.num;
         style->ext->has.shadow_spread = 1;
         break;
     case LV_STYLE_SHADOW_BLEND_MODE:
         _alloc_ext(style);
         style->ext->shadow_blend_mode = value.num;
         style->ext->has.shadow_blend_mode = 1;
         break;
     case LV_STYLE_SHADOW_COLOR:
         id= style->dont_index ? 0 : alloc_index_color(value);
         if(id > 0) {
             style->shadow_color = id;
         } else {
             _alloc_ext(style);
             style->ext->shadow_color = value.color;
             style->ext->has.shadow_color = 1;
             style->shadow_color = 0;
         }
         break;
     case LV_STYLE_SHADOW_OPA:
         id= style->dont_index ? 0 : alloc_index_num(value);
         if(id > 0) {
             style->shadow_opa = id;
         } else {
             _alloc_ext(style);
             style->ext->shadow_opa = value.num;
             style->ext->has.shadow_opa = 1;
             style->shadow_opa = 0;
         }
         break;

     case LV_STYLE_LINE_WIDTH:
         id= style->dont_index ? 0 : alloc_index_num(value);
         if(id > 0) {
             style->line_width = id;
         } else {
             _alloc_ext(style);
             style->ext->line_width = value.num;
             style->ext->has.line_width = 1;
             style->line_width = 0;
         }
         break;
     case LV_STYLE_LINE_BLEND_MODE:
         _alloc_ext(style);
         style->ext->line_blend_mode = value.num;
         style->ext->has.line_blend_mode = 1;
         break;
     case LV_STYLE_LINE_DASH_WIDTH:
         _alloc_ext(style);
         style->ext->line_dash_width = value.num;
         style->ext->has.line_dash_width = 1;
         break;
     case LV_STYLE_LINE_DASH_GAP:
         _alloc_ext(style);
         style->ext->line_dash_gap = value.num;
         style->ext->has.line_dash_gap = 1;
         break;
     case LV_STYLE_LINE_ROUNDED:
         _alloc_ext(style);
         style->line_rounded = value.num;
         style->has_line_rounded = 1;
         break;
     case LV_STYLE_LINE_COLOR:
         id= style->dont_index ? 0 : alloc_index_color(value);
         if(id > 0) {
             style->line_color = id;
         } else {
             _alloc_ext(style);
             style->ext->line_color = value.color;
             style->ext->has.line_color = 1;
             style->line_color = 0;
         }
         break;
     case LV_STYLE_LINE_OPA:
         id= style->dont_index ? 0 : alloc_index_num(value);
         if(id > 0) {
             style->line_opa = id;
         } else {
             _alloc_ext(style);
             style->ext->line_opa = value.num;
             style->ext->has.line_opa = 1;
             style->line_opa = 0;
         }
         break;

     case LV_STYLE_CONTENT_SRC:
         _alloc_ext(style);
         style->ext->content_src = value.ptr;
         style->ext->has.content_src = 1;
         break;
     case LV_STYLE_CONTENT_ALIGN:
         _alloc_ext(style);
         style->ext->content_align = value.num;
         style->ext->has.content_align = 1;
         break;
     case LV_STYLE_CONTENT_OFS_X:
         _alloc_ext(style);
         style->ext->content_ofs_x = value.num;
         style->ext->has.content_ofs_x = 1;
         break;
     case LV_STYLE_CONTENT_OFS_Y:
         _alloc_ext(style);
         style->ext->content_ofs_y = value.num;
         style->ext->has.content_ofs_y = 1;
         break;
     default:
         break;
     }
}


static bool get_prop(const lv_style_t * style, lv_style_prop_t prop, lv_style_value_t * value)
{
    switch(prop) {
      case LV_STYLE_RADIUS:
          if(style->radius) { value->num = buf_num[style->radius]; return true; }
          if(style->ext && style->ext->has.radius) { value->num = style->ext->radius; return true; }
          break;
      case LV_STYLE_CLIP_CORNER:
          if(style->has_clip_corner) { value->num = style->clip_corner; return true; }
          break;
      case LV_STYLE_TRANSFORM_WIDTH:
          if(style->transform_width) { value->num = buf_num[style->transform_width]; return true; }
          if(style->ext && style->ext->has.transform_width) { value->num = style->ext->transform_width; return true; }
          break;
      case LV_STYLE_TRANSFORM_HEIGHT:
          if(style->transform_height) { value->num = buf_num[style->transform_height]; return true; }
          if(style->ext && style->ext->has.transform_height) { value->num = style->ext->transform_height; return true; }
          break;
      case LV_STYLE_TRANSFORM_ZOOM:
          if(style->ext && style->ext->has.transform_zoom) { value->num = style->ext->transform_zoom; return true; }
          break;
      case LV_STYLE_TRANSFORM_ANGLE:
          if(style->ext && style->ext->has.transform_angle) { value->num = style->ext->transform_angle; return true; }
          break;
      case LV_STYLE_OPA:
          if(style->opa) { value->num = buf_num[style->opa]; return true; }
          if(style->ext && style->ext->has.opa) { value->num = style->ext->opa; return true; }
          break;
      case LV_STYLE_COLOR_FILTER_CB:
          if(style->color_filter_cb) { value->func = buf_ptr[style->color_filter_cb]; return true; }
          if(style->ext && style->ext->has.color_filter_cb) { value->func = (void(*)(void)) style->ext->color_filter_cb; return true; }
          break;
      case LV_STYLE_COLOR_FILTER_OPA:
          if(style->color_filter_opa) { value->num = buf_num[style->color_filter_opa]; return true; }
          if(style->ext && style->ext->has.color_filter_opa) { value->num = style->ext->color_filter_opa; return true; }
          break;
      case LV_STYLE_TRANSITION:
          if(style->transition) { value->ptr = buf_ptr[style->transition]; return true; }
          if(style->ext && style->ext->has.transition) { value->ptr = style->ext->transition; return true; }
             break;

      case LV_STYLE_PAD_TOP:
          if(style->pad_top) { value->num = buf_num[style->pad_top]; return true; }
          if(style->ext && style->ext->has.pad_top) { value->num = style->ext->pad_top; return true; }
          break;
      case LV_STYLE_PAD_BOTTOM:
          if(style->pad_bottom) { value->num = buf_num[style->pad_bottom]; return true; }
          if(style->ext && style->ext->has.pad_bottom) { value->num = style->ext->pad_bottom; return true; }
          break;
      case LV_STYLE_PAD_LEFT:
          if(style->pad_left) { value->num = buf_num[style->pad_left]; return true; }
          if(style->ext && style->ext->has.pad_left) { value->num = style->ext->pad_left; return true; }
          break;
      case LV_STYLE_PAD_RIGHT:
          if(style->pad_right) { value->num = buf_num[style->pad_right]; return true; }
          if(style->ext && style->ext->has.pad_top) { value->num = style->ext->pad_right; return true; }
          break;
      case LV_STYLE_PAD_ROW:
          if(style->ext && style->ext->has.pad_row) { value->num = style->ext->pad_row; return true; }
          break;
      case LV_STYLE_PAD_COLUMN:
          if(style->ext && style->ext->has.pad_column) { value->num = style->ext->pad_column; return true; }
          break;

      case LV_STYLE_BG_COLOR:
      case LV_STYLE_BG_COLOR_FILTERED:
          if(style->bg_color) { value->color = buf_color[style->bg_color]; return true; }
          if(style->ext && style->ext->has.bg_color) { value->color = style->ext->bg_color; return true; }
          break;
      case LV_STYLE_BG_OPA:
          if(style->bg_opa) { value->num = buf_num[style->bg_opa]; return true; }
          if(style->ext && style->ext->has.bg_opa) { value->num = style->ext->bg_opa; return true; }
          break;
      case LV_STYLE_BG_GRAD_COLOR:
      case LV_STYLE_BG_GRAD_COLOR_FILTERED:
          if(style->bg_grad_color) { value->color = buf_color[style->bg_grad_color]; return true; }
          if(style->ext && style->ext->has.bg_grad_color) { value->color = style->ext->bg_grad_color; return true; }
          break;
      case LV_STYLE_BG_GRAD_DIR:
          if(style->has_bg_grad_dir) { value->num = style->bg_grad_dir; return true; }
          break;
      case LV_STYLE_BG_BLEND_MODE:
          if(style->ext && style->ext->has.bg_blend_mode) { value->num = style->ext->bg_blend_mode; return true; }
          break;
      case LV_STYLE_BG_MAIN_STOP:
          if(style->ext && style->ext->has.bg_main_stop) { value->num = style->ext->bg_main_stop; return true; }
          break;
      case LV_STYLE_BG_GRAD_STOP:
          if(style->ext && style->ext->has.bg_grad_stop) { value->num = style->ext->bg_grad_stop; return true; }
          break;

      case LV_STYLE_BORDER_COLOR:
      case LV_STYLE_BORDER_COLOR_FILTERED:
          if(style->border_color) { value->color = buf_color[style->border_color]; return true; }
          if(style->ext && style->ext->has.border_color) { value->color = style->ext->border_color; return true; }
          break;
      case LV_STYLE_BORDER_OPA:
          if(style->border_opa) { value->num = buf_num[style->border_opa]; return true; }
          if(style->ext && style->ext->has.border_opa) { value->num = style->ext->border_opa; return true; }
          break;
      case LV_STYLE_BORDER_WIDTH:
          if(style->border_width) { value->num = buf_num[style->border_width]; return true; }
          if(style->ext && style->ext->has.border_width) { value->num = style->ext->border_width; return true; }
          break;
      case LV_STYLE_BORDER_SIDE:
          if(style->ext && style->ext->has.border_side) { value->num = style->ext->border_side; return true; }
          break;
      case LV_STYLE_BORDER_POST:
          if(style->border_post) { value->num = style->border_post; return true; }
          break;
      case LV_STYLE_BORDER_BLEND_MODE:
          if(style->ext && style->ext->has.border_blend_mode) { value->num = style->ext->border_blend_mode; return true; }
          break;

      case LV_STYLE_TEXT_COLOR:
      case LV_STYLE_TEXT_COLOR_FILTERED:
          if(style->text_color) { value->color = buf_color[style->text_color]; return true; }
          if(style->ext && style->ext->has.text_color) { value->color = style->ext->text_color; return true; }
          break;
      case LV_STYLE_TEXT_OPA:
          if(style->text_opa) { value->num = buf_num[style->text_opa]; return true; }
          if(style->ext && style->ext->has.text_opa) { value->num = style->ext->text_opa; return true; }
          break;
      case LV_STYLE_TEXT_FONT:
          if(style->text_font) { value->ptr = buf_ptr[style->text_font]; return true; }
          if(style->ext && style->ext->has.text_font) { value->ptr = style->ext->text_font; return true; }
          break;
      case LV_STYLE_TEXT_LETTER_SPACE:
          if(style->ext && style->ext->has.text_letter_space) { value->num = style->ext->text_letter_space; return true; }
          break;
      case LV_STYLE_TEXT_LINE_SPACE:
          if(style->ext && style->ext->has.text_letter_space) { value->num = style->ext->text_line_space; return true; }
          break;
      case LV_STYLE_TEXT_DECOR:
          if(style->ext && style->ext->has.text_letter_space) { value->num = style->ext->text_decor; return true; }
          break;
      case LV_STYLE_TEXT_BLEND_MODE:
          if(style->ext && style->ext->has.text_blend_mode) { value->num = style->ext->text_blend_mode; return true; }
          break;

      case LV_STYLE_IMG_OPA:
          if(style->img_opa) { value->num = buf_num[style->img_opa]; return true; }
          if(style->ext && style->ext->has.img_opa) { value->num = style->ext->img_opa; return true; }
          break;
      case LV_STYLE_IMG_BLEND_MODE:
          if(style->ext && style->ext->has.img_blend_mode) { value->num = style->ext->img_blend_mode; return true; }
          break;
      case LV_STYLE_IMG_RECOLOR:
      case LV_STYLE_IMG_RECOLOR_FILTERED:
          if(style->ext && style->ext->has.img_recolor) { value->color = style->ext->img_recolor; return true; }
          break;
      case LV_STYLE_IMG_RECOLOR_OPA:
          if(style->ext && style->ext->has.img_recolor_opa) { value->num = style->ext->img_recolor_opa; return true; }
          break;


      case LV_STYLE_OUTLINE_WIDTH:
          if(style->outline_width) { value->num = buf_num[style->outline_width]; return true; }
          if(style->ext && style->ext->has.outline_width) { value->num = style->ext->outline_width; return true; }
          break;
      case LV_STYLE_OUTLINE_COLOR:
      case LV_STYLE_OUTLINE_COLOR_FILTERED:
          if(style->outline_color) { value->color = buf_color[style->outline_color]; return true; }
          if(style->ext && style->ext->has.outline_color) { value->color = style->ext->outline_color; return true; }
          break;
      case LV_STYLE_OUTLINE_OPA:
          if(style->outline_opa) { value->num = buf_num[style->outline_opa]; return true; }
          if(style->ext && style->ext->has.outline_opa) { value->num = style->ext->outline_opa; return true; }
          break;
      case LV_STYLE_OUTLINE_PAD:
          if(style->outline_pad) { value->num = buf_num[style->outline_pad]; return true; }
          if(style->ext && style->ext->has.outline_pad) { value->num = style->ext->outline_pad; return true; }
          break;
      case LV_STYLE_OUTLINE_BLEND_MODE:
          if(style->ext && style->ext->has.outline_blend_mode) { value->num = style->ext->outline_blend_mode; return true; }
          break;

      case LV_STYLE_SHADOW_WIDTH:
          if(style->shadow_width) { value->num = buf_num[style->shadow_width]; return true; }
          if(style->ext && style->ext->has.shadow_width) { value->num = style->ext->shadow_width; return true; }
          break;
      case LV_STYLE_SHADOW_OFS_X:
          if(style->ext && style->ext->has.shadow_ofs_x) { value->num = style->ext->shadow_ofs_x; return true; }
          break;
      case LV_STYLE_SHADOW_OFS_Y:
          if(style->ext && style->ext->has.shadow_ofs_y) { value->num = style->ext->shadow_ofs_y; return true; }
          break;
      case LV_STYLE_SHADOW_SPREAD:
          if(style->ext && style->ext->has.shadow_spread) { value->num = style->ext->shadow_spread; return true; }
          break;
      case LV_STYLE_SHADOW_BLEND_MODE:
          if(style->ext && style->ext->has.shadow_blend_mode) { value->num = style->ext->shadow_blend_mode; return true; }
          break;
      case LV_STYLE_SHADOW_COLOR:
      case LV_STYLE_SHADOW_COLOR_FILTERED:
          if(style->shadow_color) { value->color = buf_color[style->shadow_color]; return true; }
          if(style->ext && style->ext->has.shadow_color) { value->color = style->ext->shadow_color; return true; }
          break;
      case LV_STYLE_SHADOW_OPA:
          if(style->shadow_opa) { value->num = buf_num[style->shadow_opa]; return true; }
          if(style->ext && style->ext->has.shadow_opa) { value->num = style->ext->shadow_opa; return true; }
          break;

      case LV_STYLE_LINE_WIDTH:
          if(style->line_width) { value->num = buf_num[style->line_width]; return true; }
          if(style->ext && style->ext->has.line_width) { value->num = style->ext->has.line_width; return true; }
          break;
      case LV_STYLE_LINE_BLEND_MODE:
          if(style->ext && style->ext->has.line_blend_mode) { value->num = style->ext->line_blend_mode; return true; }
          break;
      case LV_STYLE_LINE_DASH_GAP:
          if(style->ext && style->ext->has.line_dash_gap) { value->num = style->ext->line_dash_gap; return true; }
          break;
      case LV_STYLE_LINE_DASH_WIDTH:
          if(style->ext && style->ext->has.line_dash_width) { value->num = style->ext->line_dash_width; return true; }
          break;
      case LV_STYLE_LINE_ROUNDED:
          if(style->has_line_rounded) { value->num = style->line_rounded; return true; }
          break;
      case LV_STYLE_LINE_COLOR:
      case LV_STYLE_LINE_COLOR_FILTERED:
          if(style->line_color) { value->color = buf_color[style->line_color]; return true; }
          if(style->ext && style->ext->has.line_color) { value->color = style->ext->line_color; return true; }
          break;
      case LV_STYLE_LINE_OPA:
          if(style->line_opa) { value->num = buf_num[style->line_opa]; return true; }
          if(style->ext && style->ext->has.line_opa) { value->num = style->ext->line_opa; return true; }
          break;

      case LV_STYLE_CONTENT_SRC:
             if(style->ext && style->ext->has.content_src) { value->ptr = style->ext->content_src; return true; }
             break;
      case LV_STYLE_CONTENT_ALIGN:
             if(style->ext && style->ext->has.content_align) { value->num = style->ext->content_align; return true; }
             break;
      case LV_STYLE_CONTENT_OFS_X:
             if(style->ext && style->ext->has.content_ofs_x) { value->num = style->ext->content_ofs_x; return true; }
             break;
      case LV_STYLE_CONTENT_OFS_Y:
             if(style->ext && style->ext->has.content_ofs_y) { value->num = style->ext->content_ofs_x; return true; }
             break;
      default:
          break;
      }

      return false;
}

static bool remove_prop(lv_style_t * style, lv_style_prop_t prop)
{
    if(style == NULL) return false;
    LV_ASSERT_STYLE(style);

    switch(prop) {

    case LV_STYLE_RADIUS:
        style->radius = 0;
        if(style->ext) style->ext->has.radius = 0;
        break;
    case LV_STYLE_CLIP_CORNER:
        style->has_clip_corner = 0;
        break;
    case LV_STYLE_TRANSFORM_WIDTH:
        style->transform_width = 0;
        if(style->ext) style->ext->has.transform_width = 0;
        break;
    case LV_STYLE_TRANSFORM_HEIGHT:
        style->transform_height = 0;
        if(style->ext) style->ext->has.transform_height = 0;
        break;
    case LV_STYLE_TRANSFORM_ZOOM:
        if(style->ext) style->ext->has.transform_zoom = 0;
        break;
    case LV_STYLE_TRANSFORM_ANGLE:
        if(style->ext) style->ext->has.transform_angle = 0;
        break;
    case LV_STYLE_OPA:
        style->opa = 0;
        if(style->ext) style->ext->has.opa = 0;
        break;
    case LV_STYLE_COLOR_FILTER_CB:
        style->color_filter_cb = 0;
        if(style->ext) style->ext->has.color_filter_cb = 0;
        break;
    case LV_STYLE_COLOR_FILTER_OPA:
        style->color_filter_opa = 0;
        if(style->ext) style->ext->has.color_filter_opa = 0;
        break;
    case LV_STYLE_TRANSITION:
        style->transition = 0;
        if(style->ext) style->ext->has.transition = 0;
        break;


    case LV_STYLE_PAD_TOP:
        style->pad_top = 0;
        if(style->ext) style->ext->has.pad_top = 0;
        break;
    case LV_STYLE_PAD_BOTTOM:
        style->pad_bottom = 0;
        if(style->ext) style->ext->has.pad_bottom = 0;
        break;
    case LV_STYLE_PAD_LEFT:
        style->pad_left = 0;
        if(style->ext) style->ext->has.pad_left = 0;
        break;
    case LV_STYLE_PAD_RIGHT:
        style->pad_right = 0;
        if(style->ext) style->ext->has.pad_right = 0;
        break;
    case LV_STYLE_PAD_ROW:
        if(style->ext) style->ext->has.pad_row = 0;
        break;
    case LV_STYLE_PAD_COLUMN:
        if(style->ext) style->ext->has.pad_column = 0;
        break;

    case LV_STYLE_BG_COLOR:
        style->bg_color = 0;
        if(style->ext) style->ext->has.bg_color = 0;
        break;
    case LV_STYLE_BG_OPA:
        style->bg_opa = 0;
        if(style->ext) style->ext->has.bg_opa = 0;
        break;
    case LV_STYLE_BG_GRAD_COLOR:
        style->bg_grad_color = 0;
        if(style->ext) style->ext->has.bg_grad_color = 0;
        break;
    case LV_STYLE_BG_GRAD_DIR:
        style->has_bg_grad_dir = 0;
        break;
    case LV_STYLE_BG_BLEND_MODE:
        if(style->ext) style->ext->has.bg_blend_mode = 0;
        break;
    case LV_STYLE_BG_MAIN_STOP:
        if(style->ext) style->ext->has.bg_main_stop = 0;
        break;
    case LV_STYLE_BG_GRAD_STOP:
        if(style->ext) style->ext->has.bg_grad_stop = 0;
        break;

    case LV_STYLE_BORDER_COLOR:
        style->border_color = 0;
        if(style->ext) style->ext->has.border_color = 0;
        break;
    case LV_STYLE_BORDER_OPA:
        style->border_opa = 0;
        if(style->ext) style->ext->has.border_opa = 0;
        break;
    case LV_STYLE_BORDER_WIDTH:
        style->border_width = 0;
        if(style->ext) style->ext->has.border_width = 0;
        break;
    case LV_STYLE_BORDER_SIDE:
        if(style->ext) style->ext->has.border_side = 0;
        break;
    case LV_STYLE_BORDER_POST:
        style->has_border_post = 0;
        break;
    case LV_STYLE_BORDER_BLEND_MODE:
        if(style->ext) style->ext->has.border_blend_mode = 0;
        break;

    case LV_STYLE_TEXT_COLOR:
        style->text_color = 0;
        if(style->ext) style->ext->has.text_color = 0;
        break;
    case LV_STYLE_TEXT_OPA:
        style->text_opa = 0;
        if(style->ext) style->ext->has.text_opa = 0;
        break;
    case LV_STYLE_TEXT_FONT:
        style->text_font = 0;
        if(style->ext) style->ext->has.text_font = 0;
        break;
    case LV_STYLE_TEXT_LETTER_SPACE:
        if(style->ext) style->ext->has.text_letter_space = 0;
        break;
    case LV_STYLE_TEXT_LINE_SPACE:
        if(style->ext) style->ext->has.text_line_space = 0;
        break;
    case LV_STYLE_TEXT_DECOR:
        if(style->ext) style->ext->has.text_decor = 0;
        break;
    case LV_STYLE_TEXT_BLEND_MODE:
        if(style->ext) style->ext->has.text_blend_mode = 0;
        break;

    case LV_STYLE_IMG_OPA:
        style->img_opa = 0;
        if(style->ext) style->ext->has.img_opa = 0;
        break;
    case LV_STYLE_IMG_BLEND_MODE:
        style->ext->has.img_blend_mode = 0;
        break;
    case LV_STYLE_IMG_RECOLOR:
        if(style->ext) style->ext->has.img_recolor = 0;
        break;
    case LV_STYLE_IMG_RECOLOR_OPA:
        if(style->ext) style->ext->has.img_recolor_opa = 0;
        break;

    case LV_STYLE_OUTLINE_OPA:
        style->outline_opa = 0;
        if(style->ext) style->ext->has.outline_opa = 0;
        break;
    case LV_STYLE_OUTLINE_COLOR:
        style->outline_color = 0;
        if(style->ext) style->ext->has.outline_color = 0;
        break;
    case LV_STYLE_OUTLINE_WIDTH:
        style->outline_width = 0;
        if(style->ext) style->ext->has.outline_width = 0;
        break;
    case LV_STYLE_OUTLINE_PAD:
        style->outline_pad = 0;
        if(style->ext) style->ext->has.outline_pad = 0;
        break;
    case LV_STYLE_OUTLINE_BLEND_MODE:
        if(style->ext) style->ext->has.outline_blend_mode = 0;
        break;

    case LV_STYLE_SHADOW_WIDTH:
        style->shadow_width = 0;
        if(style->ext) style->ext->has.shadow_width = 0;
        break;
    case LV_STYLE_SHADOW_OFS_X:
        if(style->ext) style->ext->has.shadow_ofs_x = 0;
        break;
    case LV_STYLE_SHADOW_OFS_Y:
        if(style->ext) style->ext->has.shadow_ofs_y = 0;
        break;
    case LV_STYLE_SHADOW_SPREAD:
        if(style->ext) style->ext->has.shadow_spread = 0;
        break;
    case LV_STYLE_SHADOW_BLEND_MODE:
        if(style->ext) style->ext->has.shadow_blend_mode = 0;
        break;
    case LV_STYLE_SHADOW_COLOR:
        style->shadow_color = 0;
        if(style->ext) style->ext->has.shadow_color = 0;
        break;
    case LV_STYLE_SHADOW_OPA:
        style->shadow_opa = 0;
        if(style->ext) style->ext->has.shadow_opa = 0;
        break;

    case LV_STYLE_LINE_WIDTH:
        style->line_width = 0;
        if(style->ext) style->ext->has.line_width = 0;
        break;
    case LV_STYLE_LINE_BLEND_MODE:
        if(style->ext) style->ext->has.line_blend_mode = 0;
        break;
    case LV_STYLE_LINE_DASH_GAP:
        if(style->ext) style->ext->has.line_dash_gap = 0;
        break;
    case LV_STYLE_LINE_DASH_WIDTH:
        if(style->ext) style->ext->has.line_dash_width = 0;
        break;
    case LV_STYLE_LINE_ROUNDED:
        style->has_line_rounded = 0;
        break;
    case LV_STYLE_LINE_COLOR:
        style->line_color = 0;
        if(style->ext) style->ext->has.line_color = 0;
        break;
    case LV_STYLE_LINE_OPA:
        style->line_opa = 0;
        if(style->ext) style->ext->has.line_opa = 0;
        break;

    case LV_STYLE_CONTENT_ALIGN:
        if(style->ext) style->ext->has.content_align = 0;
        break;
    case LV_STYLE_CONTENT_OFS_X:
        if(style->ext) style->ext->has.content_ofs_x = 0;
        break;
    case LV_STYLE_CONTENT_OFS_Y:
        if(style->ext) style->ext->has.content_ofs_y = 0;
        break;
    case LV_STYLE_CONTENT_SRC:
        if(style->ext) style->ext->has.content_src = 0;
        break;
    default:
        return false;
    }

    return true;
}
