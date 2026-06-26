/*
 * CH342/CH343/CH344/CH346/CH347/CH348/CH9344/CH9101/CH9102/CH9103/CH9104/CH9105/CH9111/CH9114
 * parameter configuration application
 * Copyright (C) 2026 Nanjing Qinheng Microelectronics Co., Ltd.
 * Web: http://wch.cn
 * Author: WCH <tech@wch.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * Cross-compile with cross-gcc -I /path/to/cross-kernel/include
 *
 * V1.0 - initial version
 * v1.1 - add support of CH9105
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>

#include "ch343_lib.h"
#include "ch9344_lib.h"
#include "ch34x_cfg_lib.h"

static const char *ch34x_strerr(int ret)
{
    switch (-ret) {
    case CH34X_ERR_USB_TRANS:
        return "USB transfer failed";
    case CH34X_ERR_RAW_EEPROM:
        return "EEPROM read/write value mismatch";
    case CH34X_ERR_EEPROM_INIT:
        return "EEPROM is uninitialized, please run 'd' to write default config first";
    default:
        return "unknown or system error";
    }
}

static void print_usbCfg(usbConfig_t *usbCfg)
{
    /* ============ Print chip configuration ============ */
    printf("\n========================= USB Configuration =========================\n");
    printf("VendorID                       : %s\n", usbCfg->vendor_id);
    printf("ProductID                      : %s\n", usbCfg->product_id);
    printf("MaxPower(mA)                   : %d\n", usbCfg->max_power);
    printf("PowerMode                      : %s\n", usbCfg->power_mode);

    printf("Remote_Wakeup_Enable           : %s\n", usbCfg->remote_wakeup ? "yes" : "no");

    printf("Serial_String_Enable           : %s\n", usbCfg->serial_string_enable ? "yes" : "no");
    printf("Product_String_Enable          : %s\n", usbCfg->product_string_enable ? "yes" : "no");
    printf("Manufacturer_String_Enable     : %s\n", usbCfg->manufacturer_string_enable ? "yes" : "no");
    if (usbCfg->serial_string_enable)
        printf("Serial_String                  : %s\n", usbCfg->serial_string);
    if (usbCfg->product_string_enable)
        printf("Product_String                 : %s\n", usbCfg->product_string);
    if (usbCfg->manufacturer_string_enable)
        printf("Manufacturer_String            : %s\n", usbCfg->manufacturer_string);

    printf("CDC_CTSRTS_FlowControl_Enable  : %s\n", usbCfg->cdc_flow_control ? "yes" : "no");
    printf("PIN_USE_EEPROM_Enable          : %s\n", usbCfg->pin_eeprom_def_enable ? "yes" : "no");
    printf("Suspend_Disable                : %s\n", usbCfg->suspend_disable ? "yes" : "no");
    printf("DTR_MUX_TNOW_SoftSet_Enable    : %s\n", usbCfg->dtr_mux_tnow_softset_enable ? "yes" : "no");
    printf("UARTx_TNOW_DTR_SETBITS         : 0x%02X\n", usbCfg->dtr_mux_tnow_setbits);
    printf("=====================================================================\n");
}

int main(int argc, char *argv[])
{
    int ret;
    char c = 0;
    char device_path[64] = {0};
    char cfgfile_path[64] = {0};
    struct ch34xcfg *ch34xcfg;
    usbConfig_t usbCfg;

    if (argc == 3) {
        memset(device_path, 0x00, sizeof(device_path));
        memcpy(device_path, argv[1], strlen(argv[1]));
        memset(cfgfile_path, 0x00, sizeof(cfgfile_path));
        memcpy(cfgfile_path, argv[2], strlen(argv[2]));
    } else {
        printf("Usage: ./sercfg [device_path] [config file path]\n");
        printf("Exp2: ./sercfg /dev/ttyCH343USB0 CONFIG.INI\n");
        printf("Exp3: ./sercfg /dev/ttyCH9344USB0 CONFIG.INI\n");
        exit(0);
    }

    ch34xcfg = ch34x_cfg_alloc(device_path);
    if (!ch34xcfg) {
        printf("ch34x_cfg_alloc error\n");
        exit(1);
    }

    while (1) {
        if (c != '\n')
            printf("\npress g to get usb config, s to set usb config, d to set default config, q to quit this app.\n");

        if (scanf("%c", &c) != 1)
            break;
        if (c == 'q')
            break;

        switch (c) {
        case 'g':
            ret = ch34x_cfg_get(ch34xcfg);
            if (ret < 0) {
                printf("ch34x_cfg_get error, ret:%d (%s)\n", ret, ch34x_strerr(ret));
                goto exit;
            }

            ret = ch34x_get_usbCfg(ch34xcfg, &usbCfg);
            if (ret < 0) {
                printf("ch34x_get_usbCfg error, ret:%d (%s)\n", ret, ch34x_strerr(ret));
                goto exit;
            }

            print_usbCfg(&usbCfg);
            break;
        case 's':
            ret = ch34x_cfg_set(ch34xcfg, cfgfile_path);
            if (ret < 0) {
                printf("ch34x_cfg_set error, ret:%d (%s)\n", ret, ch34x_strerr(ret));
                goto exit;
            }
            break;
        case 'd':
            ret = ch34x_cfg_setdef(ch34xcfg);
            if (ret < 0) {
                printf("ch34x_cfg_setdef error, ret:%d (%s)\n", ret, ch34x_strerr(ret));
                goto exit;
            }
            break;
        default:
            break;
        }
    }

exit:
    ret = ch34x_cfg_free(ch34xcfg);
    if (ret < 0)
        printf("ch34x_cfg_free error, ret:%d\n", ret);

    exit(0);
}
