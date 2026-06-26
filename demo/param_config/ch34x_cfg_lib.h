#ifndef CH34X_CFG_LIB_H
#define CH34X_CFG_LIB_H

#include <stdint.h>
#include <stdbool.h>

/*----------------------------------------------------------------------
 * USB Configuration Structure
 *----------------------------------------------------------------------*/
typedef struct ch34x_usbConfig {
    char     vendor_id[8];                       /* Vendor ID, 4-hex-char string (e.g. "1A86") */
    char     product_id[8];                      /* Product ID, 4-hex-char string (e.g. "55D2") */
    int      max_power;                          /* Max power consumption in mA (0–500) */
    char     power_mode[32];                     /* Power mode: "Bus-Powered" or "Self-Powered" */
    bool     remote_wakeup;                      /* Remote wakeup enabled */
    bool     serial_string_enable;               /* Serial number string descriptor enabled (bCfg bit7) */
    bool     product_string_enable;              /* Product string descriptor enabled (bCfg bit6) */
    bool     manufacturer_string_enable;         /* Manufacturer string descriptor enabled (bCfg bit5) */
    char     serial_string[64];                  /* USB serial number string (≤10 characters) */
    char     product_string[64];                 /* USB product string (≤18 characters) */
    char     manufacturer_string[64];            /* USB manufacturer string (≤18 characters) */
    bool     cdc_flow_control;                   /* CDC CTS/RTS flow control enabled (bCfg bit4) */
    bool     suspend_disable;                    /* Force-disable USB suspend (0xFA in s_dis) */
    bool     pin_eeprom_def_enable;              /* Pin defaults loaded from EEPROM (bCfg bit3) */
    bool     dtr_mux_tnow_softset_enable;        /* DTR/TNOW soft-set enabled (bCfg bit2) */
    uint8_t  dtr_mux_tnow_setbits;               /* DTR/TNOW mux configuration bits (bDTR, 0x00–0xFF) */

    /* ---- CH9101R/Y MODEM pin-mux (bM_S bits 3:0) ---- */
    bool     CH9101RY_MODEM_Enable;              /* MODEM mode enabled (bCfg bit1) */
    bool     CH9101RY_TXS_MUX_DSR_Enable;        /* TXS mux to DSR (bM_S bit0) */
    bool     CH9101RY_RXS_MUX_RI_Enable;         /* RXS mux to RI  (bM_S bit1) */
    bool     CH9101RY_TNOW_MUX_DCD_Enable;       /* TNOW mux to DCD (bM_S bit2) */
    bool     CH9101RY_SLEEP_MUX_DTR_Enable;      /* SLEEP mux to DTR (bM_S bit3) */

    /* ---- CH346C extended configuration ---- */
    bool     ch346_config_enable;                /* CH346 extension config enabled (bEx_f == 0x8A) */
    uint32_t ch346_config_freq;                  /* Main frequency (Hz, stored in bFrq[4]) */
    int      ch346_config_mode;                  /* Operating mode (bWm) */
    int      ch346_config_plen;                  /* Communication packet length (bP_l[2]) */
    int      ch346_config_m0_io0;                /* Mode-0 IO0 (PIN12) function */
    int      ch346_config_m0_io1;                /* Mode-0 IO1 (PIN13) function */
    int      ch346_config_m1_io0;                /* Mode-1 IO0 (PIN15) function */
    int      ch346_config_m1_io1;                /* Mode-1 IO1 (PIN16) function */
    int      ch346_config_m2_io0;                /* Mode-2 IO0 (PIN13) function */
    int      ch346_config_m2_io1;                /* Mode-2 IO1 (PIN15) function */
    int      ch346_config_m2_io2;                /* Mode-2 IO2 (PIN16) function */
    int      ch346_config_m2_io3;                /* Mode-2 IO3 (PIN17) function */
} usbConfig_t;


/*----------------------------------------------------------------------
 * Error Codes
 *
 * Library APIs return 0 on success or a NEGATIVE error code on failure.
 * Compare the return value against -(error_code), for example:
 *     ret = ch34x_cfg_get(pcfg);
 *     if (ret == -CH34X_ERR_EEPROM_INIT) { ... run setdef first ... }
 *
 * Functions may also return:
 *   -1                    – generic error (invalid argument, etc.)
 *   -errno                – system error (check errno for details)
 *   -CH34X_ERR_PARA_ERROR – invalid parameter passed to the API
 *----------------------------------------------------------------------*/
#define CH34X_ERR_USB_TRANS   134  /* USB control transfer failed */
#define CH34X_ERR_RAW_EEPROM  135  /* EEPROM raw read-back mismatch (write vs. read) */
#define CH34X_ERR_EEPROM_INIT 136  /* EEPROM is uninitialized; run setdef to write defaults first */
#define CH34X_ERR_PARA_ERROR  137  /* Invalid argument (NULL pointer, empty string, etc.) */


/*----------------------------------------------------------------------
 * Opaque library context (defined in the implementation – do not access
 * its members directly).
 *----------------------------------------------------------------------*/
struct ch34xcfg;

/*----------------------------------------------------------------------
 * Lifecycle API
 *----------------------------------------------------------------------*/

/**
 * @brief Allocate and initialise a chip configuration context.
 *
 * @param devname  Device node path, e.g. "/dev/ttyCH343USB0".
 *
 * @return  Opaque context pointer on success, NULL on failure
 *          (out of memory, device not found, unsupported chip).
 */
extern struct ch34xcfg *ch34x_cfg_alloc(char *devname);

/**
 * @brief Release all resources associated with the configuration context.
 *
 * After this call the pointer must never be used again.
 *
 * @param pcfg  Configuration context (must be from ch34x_cfg_alloc).
 *
 * @return  0 on success, negative error code on failure.
 */
extern int ch34x_cfg_free(struct ch34xcfg *pcfg);


/*----------------------------------------------------------------------
 * Configuration Read / Write API
 *----------------------------------------------------------------------*/

/**
 * @brief Read the current chip configuration from EEPROM.
 *
 * Populates the internal usbConfig_t which can be retrieved with
 * ch34x_get_usbCfg().
 *
 * @param pcfg  Configuration context.
 *
 * @return  0 on success,
 *         -CH34X_ERR_USB_TRANS   – USB transfer failed,
 *         -CH34X_ERR_RAW_EEPROM  – EEPROM read/write mismatch,
 *         -CH34X_ERR_EEPROM_INIT – EEPROM uninitialised (run setdef first).
 */
extern int ch34x_cfg_get(struct ch34xcfg *pcfg);

/**
 * @brief Copy the current USB configuration out of the library context.
 *
 * Call this after a successful ch34x_cfg_get() or ch34x_cfg_set() to
 * obtain a snapshot of the current descriptor settings.
 *
 * @param pcfg       Configuration context.
 * @param usbConfig  Output buffer (caller-allocated).
 *
 * @return  0 on success, -CH34X_ERR_PARA_ERROR if either pointer is NULL.
 */
extern int ch34x_get_usbCfg(struct ch34xcfg *pcfg, usbConfig_t *usbConfig);

/**
 * @brief Write configuration from an INI-style file to the chip EEPROM.
 *
 * The file is parsed into key-value pairs and only the present keys are
 * applied; missing keys keep their current values.
 *
 * @param pcfg       Configuration context.
 * @param file_path  Path to the .ini configuration file.
 *
 * @return  0 on success, negative error code on failure
 *          (see Error Codes section).
 */
extern int ch34x_cfg_set(struct ch34xcfg *pcfg, char *file_path);

/**
 * @brief Restore the chip to its factory-default configuration.
 *
 * Overwrites EEPROM with the vendor-default VID/PID, power, and string
 * descriptor values appropriate for the detected chip type.
 *
 * @param pcfg  Configuration context.
 *
 * @return  0 on success, negative error code on failure
 *          (see Error Codes section).
 */
extern int ch34x_cfg_setdef(struct ch34xcfg *pcfg);


#endif /* CH34X_CFG_LIB_H */
