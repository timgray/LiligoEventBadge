#include "Schedule.h"
#include "Menu.h"
#include "Display.h"
#include "RTC.h"
#include "Storage.h"
#include "UIHelpers.h"
lv_obj_t*Schedule::timeLabel=NULL;lv_obj_t*Schedule::dateLabel=NULL;uint32_t Schedule::lastUpdate=0;
static void Back(lv_event_t*){Menu::Show();}
void Schedule::Show(){lv_obj_t*s=lv_obj_create(NULL);UIHelpers::Prepare(s);lv_scr_load(s);UIHelpers::Back(s,Back);
timeLabel=lv_label_create(s);lv_obj_set_width(timeLabel,250);lv_obj_set_style_text_align(timeLabel,LV_TEXT_ALIGN_RIGHT,0);lv_obj_align(timeLabel,LV_ALIGN_TOP_RIGHT,-20,20);
dateLabel=lv_label_create(s);lv_obj_set_width(dateLabel,440);lv_obj_set_style_text_align(dateLabel,LV_TEXT_ALIGN_CENTER,0);lv_obj_align(dateLabel,LV_ALIGN_TOP_MID,0,85);
lv_obj_t*h=lv_label_create(s);lv_label_set_text(h,"SCHEDULE");lv_obj_align(h,LV_ALIGN_TOP_LEFT,30,155);
lv_obj_t*m=lv_label_create(s);lv_obj_set_width(m,440);lv_label_set_text(m,Storage::Ready()?"SD ready. Schedule parsing is the next step.":"SD card unavailable.");lv_obj_align(m,LV_ALIGN_TOP_LEFT,30,210);
RefreshClock();lastUpdate=millis();Display::Refresh();}
void Schedule::RefreshClock(){if(!timeLabel||!dateLabel)return;BadgeDateTime n;if(!BadgeRTC::Read(n)){lv_label_set_text(timeLabel,"--:--");lv_label_set_text(dateLabel,"RTC unavailable");return;}uint8_t h=n.hour;const char*s=h>=12?"PM":"AM";if(h==0)h=12;else if(h>12)h-=12;lv_label_set_text_fmt(timeLabel,"%u:%02u %s",h,n.minute,s);lv_label_set_text_fmt(dateLabel,"%02u/%02u/%04u",n.month,n.day,n.year);}
void Schedule::Update(){if(!timeLabel||millis()-lastUpdate<60000)return;RefreshClock();lastUpdate=millis();Display::Refresh();}
