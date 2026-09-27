#include "Badge.h"
#include "Menu.h"
#include "Display.h"
#include "UIHelpers.h"
static void Back(lv_event_t*){Menu::Show();}
void Badge::Show(){Build(false);} void Badge::ShowOffMode(){Build(true);}
void Badge::Build(bool off){lv_obj_t*s=lv_obj_create(NULL);UIHelpers::Prepare(s);lv_scr_load(s);if(!off)UIHelpers::Back(s,Back);
const char*txt[]={"CRESTRON MASTERS 2026","TIMOTHY GRAY","CTI Senior Technical Trainer","Platinum Certified"};int y[]={140,290,345,380};
for(int i=0;i<4;i++){lv_obj_t*l=lv_label_create(s);lv_label_set_text(l,txt[i]);lv_obj_set_width(l,440);lv_obj_set_style_text_align(l,LV_TEXT_ALIGN_CENTER,0);lv_obj_align(l,LV_ALIGN_TOP_MID,0,y[i]);}
if(off){lv_obj_t*l=lv_label_create(s);lv_label_set_text(l,"OFF MODE");lv_obj_set_width(l,440);lv_obj_set_style_text_align(l,LV_TEXT_ALIGN_CENTER,0);lv_obj_align(l,LV_ALIGN_BOTTOM_MID,0,-35);}Display::Refresh();}
