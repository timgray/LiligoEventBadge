#include "EventBadge.h"
#include "AppManager.h"
#include "Badge.h"
#include "Schedule.h"
#include "ClockApp.h"
#include "SystemInfo.h"
#include "BluetoothScanner.h"

static Schedule scheduleApp;
static ClockApp clockApp;
static SystemInfo systemInfoApp;
static BluetoothScanner bluetoothScannerApp;

void event_badge_init()
{
    gAppManager.Register(&gBadge);
    gAppManager.Register(&scheduleApp);
    gAppManager.Register(&clockApp);
    gAppManager.Register(&systemInfoApp);
    gAppManager.Register(&bluetoothScannerApp);

    gAppManager.Start();
}

void event_badge_update()
{
    gAppManager.Update();
}

void event_badge_home()
{
    gAppManager.ShowMenu();
}
