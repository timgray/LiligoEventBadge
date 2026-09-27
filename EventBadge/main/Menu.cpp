#include "Menu.h"
#include "Badge.h"
#include "Schedule.h"
#include "Power.h"
#include "Display.h"
#include "UIHelpers.h"
static void B(lv_event_t*){Badge::Show();} static void S(lv_event_t*){Schedule::Show();} static void P(lv_event_t*){Power::Shutdown();}
void Menu::Show(){lv_obj_t*s=lv_obj_create(NULL);UIHelpers::Prepare(s);lv_scr_load(s);lv_obj_t*t=lv_label_create(s);lv_label_set_text(t,"EVENT BADGE");lv_obj_align(t,LV_ALIGN_TOP_MID,0,45);UIHelpers::Button(s,"BADGE",145,B);UIHelpers::Button(s,"SCHEDULE",270,S);UIHelpers::Button(s,"POWER OFF",395,P);Display::Refresh();}
