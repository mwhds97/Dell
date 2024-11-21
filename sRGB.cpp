#include <print>
#include <ranges>
#include <string>
#include <vector>

#include <Windows.h>
#include <Icm.h>
#include "UP_API.h"
#include "UP_DATA.h"

#pragma comment(lib, "DellMonitorSdkDll.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "mscms.lib")

int main(int argc, char* argv[]) {
    int res = Initialize();
    if(res != MONITOR_SUCCESS) {
        std::println("Initializing SDK failed!");
        return -1;
    }
    byte bval;
    MonitorDetailStructType* monitor_detail = nullptr;
    res = GetAvailableMonitorsDetail(&bval, &monitor_detail);
    if(res != MONITOR_SUCCESS) {
        std::println("Detecting monitors failed!");
        Shutdown();
        return -1;
    }
    if(bval == 0) {
        std::println("No supported monitors!");
        free(monitor_detail);
        Shutdown();
        return -1;
    }
    for(byte i = 0; i < bval; i++) {
        std::print("Monitor {}: Dell ", i + 1);
        for(char c : monitor_detail[i].MonitorName | std::views::transform(
            [](byte b) {
                return static_cast<char>(b);
            })) {
            std::print("{}", c);
        }
        std::println();
    }
    free(monitor_detail);
    for(byte i = 0; i < bval; i++) {
        res = ConnectMonitor(i);
        if(res == MONITOR_SUCCESS) {
            res = SetColorSpaceState(COLOR_SPACE2_CAL_2);
            if(res == MONITOR_SUCCESS) {
                std::println("Color space has been set to sRGB!");
            }
            else {
                std::println("Setting color space failed!");
            }
            DisconnectMonitor();
        }
        else {
            std::println("Connecting monitor failed!");
        }
    }
    Shutdown();

    std::vector<std::wstring> device_keys;
    DISPLAY_DEVICEW graphic_card;
    graphic_card.cb = sizeof(DISPLAY_DEVICEW);
    if(!EnumDisplayDevicesW(0, 0, &graphic_card, 0)) {
        std::println("Enumerating graphic card failed!");
        return -1;
    };
    DISPLAY_DEVICEW monitors;
    monitors.cb = sizeof(DISPLAY_DEVICEW);
    DWORD m = 0;
    while(EnumDisplayDevicesW(graphic_card.DeviceName, m, &monitors, 0)) {
        std::wstring dn = monitors.DeviceString;
        if(dn.contains(L"Dell")) {
            std::wstring dk = monitors.DeviceKey;
            device_keys.push_back(dk);
        }
        m++;
    }
    for(std::wstring dk : device_keys) {
        BOOL flag = WcsSetDefaultColorProfile(
            WCS_PROFILE_MANAGEMENT_SCOPE_CURRENT_USER,
            dk.c_str(),
            CPT_ICC,
            CPST_PERCEPTUAL,
            0,
            L"DELL UP3221Q_JZQDR43_CAL_2_V2.icm"
        );
        if(flag) {
            std::println("The color profile has been switched to sRGB!");
        }
        else {
            std::println("Switching color profiles failed!");
        }
    }

    return 0;
}
