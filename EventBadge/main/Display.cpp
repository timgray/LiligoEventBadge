#include "Display.h"
#include <epdiy.h>
#include <esp_heap_caps.h>
#define WAVEFORM EPD_BUILTIN_WAVEFORM
#define DISPLAY_BOARD epd_board_v7
static EpdiyHighlevelState hl;
static uint8_t *epdBuffer=NULL;
static void Check(enum EpdDrawError e){if(e!=EPD_DRAW_SUCCESS) Serial.printf("EPD error: %X\n",e);}
bool Display::Begin(){
 epd_init(&DISPLAY_BOARD,&ED047TC1,EPD_LUT_64K);
 epd_set_rotation(EPD_ROT_INVERTED_PORTRAIT);
 epd_set_lcd_pixel_clock_MHz(17);
 hl=epd_hl_init(WAVEFORM);
 epd_poweron(); epd_clear(); epd_poweroff();
 lv_init();
 int32_t pixels=epd_rotated_display_width()*epd_rotated_display_height();
 int32_t bytes=((epd_rotated_display_width()+1)/2)*epd_rotated_display_height();
 lv_color_t *a=(lv_color_t*)heap_caps_calloc(pixels,sizeof(lv_color_t),MALLOC_CAP_SPIRAM);
 lv_color_t *b=(lv_color_t*)heap_caps_calloc(pixels,sizeof(lv_color_t),MALLOC_CAP_SPIRAM);
 epdBuffer=(uint8_t*)heap_caps_calloc(bytes,1,MALLOC_CAP_SPIRAM);
 if(!a||!b||!epdBuffer) return false;
 memset(epdBuffer,0xFF,bytes);
 static lv_disp_draw_buf_t db; lv_disp_draw_buf_init(&db,a,b,pixels);
 static lv_disp_drv_t d; lv_disp_drv_init(&d);
 d.hor_res=epd_rotated_display_width(); d.ver_res=epd_rotated_display_height();
 d.flush_cb=Flush; d.draw_buf=&db; d.full_refresh=1; lv_disp_drv_register(&d);
 return true;
}
void Display::Loop(){lv_timer_handler();}
uint8_t Display::Gray(lv_color_t c){lv_color32_t x; x.full=lv_color_to32(c); uint16_t g=x.ch.red*76U+x.ch.green*150U+x.ch.blue*30U; uint8_t v=((g>>8)+8U)>>4; return v>15?15:v;}
void Display::Pixel(uint8_t*b,int32_t w,int32_t x,int32_t y,uint8_t g){int32_t p=(w+1)/2; uint8_t*d=&b[y*p+(x>>1)]; if(x&1)*d=(*d&0x0F)|(g<<4); else *d=(*d&0xF0)|g;}
void Display::Flush(lv_disp_drv_t*d,const lv_area_t*a,lv_color_t*c){
 if(!epdBuffer){lv_disp_flush_ready(d);return;}
 int32_t w=lv_area_get_width(a),h=lv_area_get_height(a),sw=epd_rotated_display_width(),sh=epd_rotated_display_height();
 for(int32_t y=0;y<h;y++){int32_t dy=a->y1+y;if(dy<0||dy>=sh)continue;for(int32_t x=0;x<w;x++){int32_t dx=a->x1+x;if(dx<0||dx>=sw)continue;Pixel(epdBuffer,sw,dx,dy,Gray(c[y*w+x]));}}
 lv_disp_flush_ready(d);
}
void Display::Refresh(){
 lv_refr_now(NULL);
 EpdRect r={.x=0,.y=0,.width=epd_rotated_display_width(),.height=epd_rotated_display_height()};
 epd_hl_set_all_white(&hl); epd_poweron(); Check(epd_hl_update_screen(&hl,MODE_DU,epd_ambient_temperature())); epd_poweroff();
 epd_draw_rotated_image(r,epdBuffer,epd_hl_get_framebuffer(&hl));
 epd_poweron(); Check(epd_hl_update_screen(&hl,MODE_GL16,epd_ambient_temperature())); epd_poweroff();
}
void Display::PowerOff(){epd_poweroff();}
