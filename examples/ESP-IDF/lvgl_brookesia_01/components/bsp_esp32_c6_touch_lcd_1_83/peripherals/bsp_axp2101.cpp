#include "bsp_board.h"
#include "esp_check.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include <stdio.h>
#include <cstring>
#include "sdkconfig.h"
#include "esp_err.h"

#define XPOWERS_CHIP_AXP2101
#include "XPowersLib.h"

static const char *TAG = "AXP2101_driver";
XPowersPMU PMU;
static i2c_master_bus_handle_t i2c_bus_handle = NULL;
static i2c_master_dev_handle_t pmu_dev_handle = NULL;

// PMU read function using new API
int pmu_register_read(uint8_t devAddr, uint8_t regAddr, uint8_t *data, uint8_t len) {
    esp_err_t ret = i2c_master_transmit_receive(pmu_dev_handle, &regAddr, 1, data, len, 1000);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "PMU READ FAILED!");
        return -1;
    }
    return 0;
}

// PMU write function using new API
int pmu_register_write_byte(uint8_t devAddr, uint8_t regAddr, uint8_t *data, uint8_t len) {
    uint8_t *buffer = (uint8_t *)malloc(len + 1);
    if (!buffer) return -1;
    buffer[0] = regAddr;
    memcpy(&buffer[1], data, len);

    esp_err_t ret = i2c_master_transmit(pmu_dev_handle, buffer, len + 1, 1000);
    free(buffer);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "PMU WRITE FAILED!");
        return -1;
    }
    return 0;
}

esp_err_t pmu_init(void)
{
    if (PMU.begin(AXP2101_SLAVE_ADDRESS, pmu_register_read, pmu_register_write_byte))
    {
        ESP_LOGI(TAG, "Init PMU SUCCESS!");
    }
    else
    {
        ESP_LOGE(TAG, "Init PMU FAILED!");
        return ESP_FAIL;
    }

    // Turn off not use power channel
    PMU.disableDC2();
    PMU.disableDC3();
    PMU.disableDC4();
    PMU.disableDC5();

    PMU.disableALDO1();
    PMU.disableALDO2();
    PMU.disableALDO3();
    PMU.disableALDO4();
    PMU.disableBLDO1();
    PMU.disableBLDO2();

    PMU.disableCPUSLDO();
    PMU.disableDLDO1();
    PMU.disableDLDO2();





    PMU.setDC1Voltage(3300);
    PMU.enableDC1();

    PMU.setALDO1Voltage(3300);
    PMU.enableALDO1();

    // AMOLED VDD 3300
    PMU.setALDO2Voltage(3300);
    PMU.enableALDO2();

    ESP_LOGI(TAG, "DCDC=======================================================================\n");
    ESP_LOGI(TAG, "DC1  : %s   Voltage:%u mV \n", PMU.isEnableDC1() ? "+" : "-", PMU.getDC1Voltage());
    ESP_LOGI(TAG, "DC2  : %s   Voltage:%u mV \n", PMU.isEnableDC2() ? "+" : "-", PMU.getDC2Voltage());
    ESP_LOGI(TAG, "DC3  : %s   Voltage:%u mV \n", PMU.isEnableDC3() ? "+" : "-", PMU.getDC3Voltage());
    ESP_LOGI(TAG, "DC4  : %s   Voltage:%u mV \n", PMU.isEnableDC4() ? "+" : "-", PMU.getDC4Voltage());
    ESP_LOGI(TAG, "DC5  : %s   Voltage:%u mV \n", PMU.isEnableDC5() ? "+" : "-", PMU.getDC5Voltage());
    ESP_LOGI(TAG, "ALDO=======================================================================\n");
    ESP_LOGI(TAG, "ALDO1: %s   Voltage:%u mV\n", PMU.isEnableALDO1() ? "+" : "-", PMU.getALDO1Voltage());
    ESP_LOGI(TAG, "ALDO2: %s   Voltage:%u mV\n", PMU.isEnableALDO2() ? "+" : "-", PMU.getALDO2Voltage());
    ESP_LOGI(TAG, "ALDO3: %s   Voltage:%u mV\n", PMU.isEnableALDO3() ? "+" : "-", PMU.getALDO3Voltage());
    ESP_LOGI(TAG, "ALDO4: %s   Voltage:%u mV\n", PMU.isEnableALDO4() ? "+" : "-", PMU.getALDO4Voltage());
    ESP_LOGI(TAG, "BLDO=======================================================================\n");
    ESP_LOGI(TAG, "BLDO1: %s   Voltage:%u mV\n", PMU.isEnableBLDO1() ? "+" : "-", PMU.getBLDO1Voltage());
    ESP_LOGI(TAG, "BLDO2: %s   Voltage:%u mV\n", PMU.isEnableBLDO2() ? "+" : "-", PMU.getBLDO2Voltage());
    ESP_LOGI(TAG, "CPUSLDO====================================================================\n");
    ESP_LOGI(TAG, "CPUSLDO: %s Voltage:%u mV\n", PMU.isEnableCPUSLDO() ? "+" : "-", PMU.getCPUSLDOVoltage());
    ESP_LOGI(TAG, "DLDO=======================================================================\n");
    ESP_LOGI(TAG, "DLDO1: %s   Voltage:%u mV\n", PMU.isEnableDLDO1() ? "+" : "-", PMU.getDLDO1Voltage());
    ESP_LOGI(TAG, "DLDO2: %s   Voltage:%u mV\n", PMU.isEnableDLDO2() ? "+" : "-", PMU.getDLDO2Voltage());
    ESP_LOGI(TAG, "===========================================================================\n");

    PMU.clearIrqStatus();

    PMU.enableVbusVoltageMeasure();
    PMU.enableBattVoltageMeasure();
    PMU.enableSystemVoltageMeasure();
    PMU.enableTemperatureMeasure();

    // It is necessary to disable the detection function of the TS pin on the board
    // without the battery temperature detection function, otherwise it will cause abnormal charging
    PMU.disableTSPinMeasure();

    // Disable all interrupts
    PMU.disableIRQ(XPOWERS_AXP2101_ALL_IRQ);
    // Clear all interrupt flags
    PMU.clearIrqStatus();
    // Enable the required interrupt function
    PMU.enableIRQ(
        XPOWERS_AXP2101_BAT_INSERT_IRQ | XPOWERS_AXP2101_BAT_REMOVE_IRQ |    // BATTERY
        XPOWERS_AXP2101_VBUS_INSERT_IRQ | XPOWERS_AXP2101_VBUS_REMOVE_IRQ |  // VBUS
        XPOWERS_AXP2101_PKEY_SHORT_IRQ | XPOWERS_AXP2101_PKEY_LONG_IRQ |     // POWER KEY
        XPOWERS_AXP2101_BAT_CHG_DONE_IRQ | XPOWERS_AXP2101_BAT_CHG_START_IRQ // CHARGE
        // XPOWERS_AXP2101_PKEY_NEGATIVE_IRQ | XPOWERS_AXP2101_PKEY_POSITIVE_IRQ   |   //POWER KEY
    );

    // Set the precharge charging current
    PMU.setPrechargeCurr(XPOWERS_AXP2101_PRECHARGE_50MA);
    // Set constant current charge current limit
    PMU.setChargerConstantCurr(XPOWERS_AXP2101_CHG_CUR_400MA);
    // Set stop charging termination current
    PMU.setChargerTerminationCurr(XPOWERS_AXP2101_CHG_ITERM_25MA);

    // Set charge cut-off voltage
    PMU.setChargeTargetVoltage(XPOWERS_AXP2101_CHG_VOL_4V2);

    // Read battery percentage
    ESP_LOGI(TAG, "battery percentage:%d %%", PMU.getBatteryPercent());

    // Set the watchdog trigger event type
    // PMU.setWatchdogConfig(XPOWERS_AXP2101_WDT_IRQ_TO_PIN);
    // Set watchdog timeout
    // PMU.setWatchdogTimeout(XPOWERS_AXP2101_WDT_TIMEOUT_4S);
    // Enable watchdog to trigger interrupt event
    // PMU.enableWatchdog();
    return ESP_OK;
}

void pmu_isr_handler(void)
{
    // Get PMU Interrupt Status Register
    PMU.getIrqStatus();

    ESP_LOGI(TAG, "Power Temperature: %.2f°C", PMU.getTemperature());

    ESP_LOGI(TAG, "isCharging: %s", PMU.isCharging() ? "YES" : "NO");

    ESP_LOGI(TAG, "isDischarge: %s", PMU.isDischarge() ? "YES" : "NO");

    ESP_LOGI(TAG, "isStandby: %s", PMU.isStandby() ? "YES" : "NO");

    ESP_LOGI(TAG, "isVbusIn: %s", PMU.isVbusIn() ? "YES" : "NO");

    ESP_LOGI(TAG, "isVbusGood: %s", PMU.isVbusGood() ? "YES" : "NO");

    uint8_t charge_status = PMU.getChargerStatus();
    if (charge_status == XPOWERS_AXP2101_CHG_TRI_STATE)
    {
        ESP_LOGI(TAG, "Charger Status: tri_charge");
    }
    else if (charge_status == XPOWERS_AXP2101_CHG_PRE_STATE)
    {
        ESP_LOGI(TAG, "Charger Status: pre_charge");
    }
    else if (charge_status == XPOWERS_AXP2101_CHG_CC_STATE)
    {
        ESP_LOGI(TAG, "Charger Status: constant charge");
    }
    else if (charge_status == XPOWERS_AXP2101_CHG_CV_STATE)
    {
        ESP_LOGI(TAG, "Charger Status: constant voltage");
    }
    else if (charge_status == XPOWERS_AXP2101_CHG_DONE_STATE)
    {
        ESP_LOGI(TAG, "Charger Status: charge done");
    }
    else if (charge_status == XPOWERS_AXP2101_CHG_STOP_STATE)
    {
        ESP_LOGI(TAG, "Charger Status: not charge");
    }

    ESP_LOGI(TAG, "getBattVoltage: %d mV", PMU.getBattVoltage());

    ESP_LOGI(TAG, "getVbusVoltage: %d mV", PMU.getVbusVoltage());

    ESP_LOGI(TAG, "getSystemVoltage: %d mV", PMU.getSystemVoltage());

    if (PMU.isBatteryConnect())
    {
        ESP_LOGI(TAG, "getBatteryPercent: %d %%", PMU.getBatteryPercent());
    }
    // Clear PMU Interrupt Status Register
    PMU.clearIrqStatus();
}

esp_err_t AXP2101_driver_init(void)
{
    esp_err_t ret = ESP_OK;
    i2c_master_get_bus_handle(0,&i2c_bus_handle);

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x34,
        .scl_speed_hz = 10000,
        .scl_wait_us = 0,
        .flags = {
            .disable_ack_check = 0
        }
    };

    ret = i2c_master_bus_add_device(i2c_bus_handle, &dev_config, &pmu_dev_handle);
    ret = pmu_init();

    return ret;
}