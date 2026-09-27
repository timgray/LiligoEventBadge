#include "Touch.h"
#include "utilities.h"
#include <Wire.h>
TouchDrvGT911 Touch::dev; void(*Touch::home)()=NULL;
bool Touch::Begin(){
 dev.setPins(BOARD_TOUCH_RST,BOARD_TOUCH_INT);
 if(!dev.begin(Wire,GT911_SLAVE_ADDRESS_L,BOARD_SDA,BOARD_SCL)){Serial.println("GT911 not found");return false;}
 dev.setInterruptMode(LOW_LEVEL_QUERY); dev.setHomeButtonCallback(Home,NULL);
 static lv_indev_drv_t d; lv_indev_drv_init(&d); d.type=LV_INDEV_TYPE_POINTER; d.read_cb=Read; lv_indev_drv_register(&d); return true;
}
void Touch::SetHomeCallback(void(*cb)()){home=cb;}
void Touch::Home(void*){if(home)home();}
void Touch::Read(lv_indev_drv_t*,lv_indev_data_t*d){static int16_t x=0,y=0;if(dev.isPressed()&&dev.getPoint(&x,&y,1))d->state=LV_INDEV_STATE_PRESSED;else d->state=LV_INDEV_STATE_RELEASED;d->point.x=x;d->point.y=y;}
