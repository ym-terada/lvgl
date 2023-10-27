/**
 * @file lv_draw_sdl.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "../lv_draw.h"
#if LV_USE_DRAW_SDL
#include LV_SDL_INCLUDE_PATH
#include "lv_draw_sdl.h"
#include "../../core/lv_refr.h"
#include "../../display/lv_display_private.h"
#include "../../stdlib/lv_string.h"
#include "../../dev/sdl/lv_sdl_window.h"

/*********************
 *      DEFINES
 *********************/
#define DRAW_UNIT_ID_SDL     100

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void execute_drawing(lv_draw_sdl_unit_t * u);

static int32_t dispatch(lv_draw_unit_t * draw_unit, lv_layer_t * layer);

static void evaluate(lv_draw_unit_t * draw_unit, lv_draw_task_t * task);

/**********************
 *  GLOBAL PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_draw_sdl_init(void)
{

    lv_draw_sdl_unit_t * draw_sw_unit = lv_draw_create_unit(sizeof(lv_draw_sdl_unit_t));
    draw_sw_unit->base_unit.dispatch_cb = dispatch;
    draw_sw_unit->base_unit.evaluate_cb = evaluate;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static int32_t dispatch(lv_draw_unit_t * draw_unit, lv_layer_t * layer)
{
    lv_draw_sdl_unit_t * draw_sdl_unit = (lv_draw_sdl_unit_t *) draw_unit;

    /*Return immediately if it's busy with a draw task*/
    if(draw_sdl_unit->task_act) return 0;

    lv_draw_task_t * t = NULL;
    t = lv_draw_get_next_available_task(layer, NULL, DRAW_UNIT_ID_SDL);
    if(t == NULL) return -1;

    void * buf = lv_draw_layer_alloc_buf(layer);
    if(buf == NULL) return -1;


    t->state = LV_DRAW_TASK_STATE_IN_PROGRESS;
    draw_sdl_unit->base_unit.target_layer = layer;
    draw_sdl_unit->base_unit.clip_area = &t->clip_area;
    draw_sdl_unit->task_act = t;

    execute_drawing(draw_sdl_unit);

    draw_sdl_unit->task_act->state = LV_DRAW_TASK_STATE_READY;
    draw_sdl_unit->task_act = NULL;

    /*The draw unit is free now. Request a new dispatching as it can get a new task*/
    lv_draw_dispatch_request();
    return 1;
}
int32_t (*evaluate_cb)(struct _lv_draw_unit_t * draw_unit, lv_draw_task_t * task);

static void evaluate(lv_draw_unit_t * draw_unit, lv_draw_task_t * task)
{
    if(((lv_draw_dsc_base_t *)task->draw_dsc)->user_data == NULL) {
        task->preference_score = 0;
        task->preferred_draw_unit_id = DRAW_UNIT_ID_SDL;
    }
}


uint8_t sdl_render_buf[1024 * 1024 * 4];


static void execute_drawing(lv_draw_sdl_unit_t * u)
{
    /*Render the draw task*/
    lv_draw_task_t * t = u->task_act;

    lv_layer_t dest_layer;
    lv_memzero(&dest_layer, sizeof(dest_layer));
    //    uint8_t * sdl_render_buf = malloc(1024*1024*4);
    lv_memzero(sdl_render_buf, 1024 * 1024 * 4);
    dest_layer.buf = lv_draw_buf_align(sdl_render_buf, LV_COLOR_FORMAT_ARGB8888);
    dest_layer.color_format = LV_COLOR_FORMAT_ARGB8888;

    lv_area_t a;
    _lv_area_intersect(&a, u->base_unit.clip_area, &t->area);
    dest_layer.buf_area = *u->base_unit.clip_area;
    dest_layer._clip_area = *u->base_unit.clip_area;

    lv_display_t * disp = _lv_refr_get_disp_refreshing();

    switch(t->type) {
        case LV_DRAW_TASK_TYPE_FILL: {
                lv_draw_fill_dsc_t * fill_dsc = t->draw_dsc;;
                lv_draw_rect_dsc_t rect_dsc;
                lv_draw_rect_dsc_init(&rect_dsc);
                rect_dsc.base.user_data = lv_sdl_window_get_renderer(disp);
                rect_dsc.bg_color = fill_dsc->color;
                rect_dsc.bg_grad = fill_dsc->grad;
                rect_dsc.radius = fill_dsc->radius;
                rect_dsc.bg_opa = fill_dsc->opa;
                lv_draw_rect(&dest_layer, &rect_dsc, &t->area);
            }
            break;
        case LV_DRAW_TASK_TYPE_BORDER: {
                lv_draw_border_dsc_t * border_dsc = t->draw_dsc;;
                lv_draw_rect_dsc_t rect_dsc;
                lv_draw_rect_dsc_init(&rect_dsc);
                rect_dsc.base.user_data = lv_sdl_window_get_renderer(disp);
                rect_dsc.bg_opa = LV_OPA_TRANSP;
                rect_dsc.radius = border_dsc->radius;
                rect_dsc.border_color = border_dsc->color;
                rect_dsc.border_opa = border_dsc->opa;
                rect_dsc.border_side = border_dsc->side;
                rect_dsc.border_width = border_dsc->width;
                lv_draw_rect(&dest_layer, &rect_dsc, &t->area);
                break;
            }
        //        case LV_DRAW_TASK_TYPE_BOX_SHADOW:
        //            lv_draw_sw_box_shadow((lv_draw_unit_t *)u, t->draw_dsc, &t->area);
        //            break;
        //        case LV_DRAW_TASK_TYPE_BG_IMG:
        //            lv_draw_sw_bg_image((lv_draw_unit_t *)u, t->draw_dsc, &t->area);
        //            break;
        case LV_DRAW_TASK_TYPE_LABEL: {
                lv_draw_label_dsc_t label_dsc;
                lv_draw_label_dsc_init(&label_dsc);
                lv_memcpy(&label_dsc, t->draw_dsc, sizeof(label_dsc));
                label_dsc.base.user_data = lv_sdl_window_get_renderer(disp);
                lv_draw_label(&dest_layer, &label_dsc, &t->area);
            }
            break;
        case LV_DRAW_TASK_TYPE_IMAGE: {
                lv_draw_image_dsc_t image_dsc;
                lv_draw_image_dsc_init(&image_dsc);
                lv_memcpy(&image_dsc, t->draw_dsc, sizeof(image_dsc));
                image_dsc.base.user_data = lv_sdl_window_get_renderer(disp);
                lv_draw_image(&dest_layer, &image_dsc, &t->area);
                break;
            }
        //                case LV_DRAW_TASK_TYPE_ARC:
        //                    lv_draw_sw_arc((lv_draw_unit_t *)u, t->draw_dsc, &t->area);
        //                    break;
        //                case LV_DRAW_TASK_TYPE_LINE:
        //                    lv_draw_sw_line((lv_draw_unit_t *)u, t->draw_dsc);
        //                    break;
        //        case LV_DRAW_TASK_TYPE_TRIANGLE:
        //            lv_draw_sw_triangle((lv_draw_unit_t *)u, t->draw_dsc);
        //            break;
        //        case LV_DRAW_TASK_TYPE_LAYER:
        //            lv_draw_sw_layer((lv_draw_unit_t *)u, t->draw_dsc, &t->area);
        //            break;
        //        case LV_DRAW_TASK_TYPE_MASK_RECTANGLE:
        //            lv_draw_sw_mask_rect((lv_draw_unit_t *)u, t->draw_dsc, &t->area);
        //            break;
        default:
            return;
            break;
    }


    while(dest_layer.draw_task_head) {
        lv_draw_dispatch_wait_for_request();
        lv_draw_dispatch_layer(disp, &dest_layer);
    }

    SDL_Rect rect;
    rect.x = dest_layer.buf_area.x1;
    rect.y = dest_layer.buf_area.y1;
    rect.w = lv_area_get_width(&dest_layer.buf_area);
    rect.h = lv_area_get_height(&dest_layer.buf_area);

    SDL_Texture * texture = SDL_CreateTexture(lv_sdl_window_get_renderer(disp), SDL_PIXELFORMAT_ARGB8888,
                                              SDL_TEXTUREACCESS_STATIC, rect.w, rect.h);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    SDL_UpdateTexture(texture, NULL, sdl_render_buf, rect.w * 4);
    SDL_RenderCopy(lv_sdl_window_get_renderer(disp), texture, NULL, &rect);
    SDL_DestroyTexture(texture);
}

#endif /*LV_USE_DRAW_SDL*/
