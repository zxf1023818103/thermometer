#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/printk.h>
#include <zephyr.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/services/bas.h>

#include <drivers/sensor.h>
#include <drivers/gpio.h>
#include <drivers/pwm.h>

#include "ess.h"

const struct device *hdc1080 = DEVICE_DT_GET(DT_INST(0, ti_hdc));

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(BT_UUID_ESS_VAL)),
};

static void connected(struct bt_conn *conn, uint8_t conn_err)
{
	int err;
	struct bt_conn_info info;
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (conn_err) {
		printk("Connection failed (err %d)\n", conn_err);
		return;
	}

	err = bt_conn_get_info(conn, &info);

	if (err) {
		printk("Failed to get connection info\n");
	} else {
		const struct bt_conn_le_phy_info *phy_info;
		phy_info = info.le.phy;

		printk("Connected: %s, tx_phy %u, rx_phy %u\n",
		       addr, phy_info->tx_phy, phy_info->rx_phy);
	}
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected (reason 0x%02x)\n", reason);
}

static struct bt_conn_cb conn_callbacks = {
	.connected = connected,
	.disconnected = disconnected,
};

static void bt_ready(int err)
{
	printk("Bluetooth initialized\n");

	err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		printk("Advertising failed to create (err %d)\n", err);
		return;
	}
}

static void bas_notify(void)
{
	uint8_t battery_level = 0;

	bt_bas_set_battery_level(battery_level);
}

static void ess_notify(const struct device *dev)
{
	struct sensor_value temperature, humidity;
	sensor_sample_fetch(dev);
	sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temperature);
	sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &humidity);
	
	uint16_t humidity_value = humidity.val1 * 100;
	humidity_value += humidity.val2 / 10000;
	
	int16_t temperature_value = temperature.val1 * 100;
	temperature_value += temperature.val2 / 10000;
	
	printk("Temperature: %04"PRId16" Humidity: %04"PRIu16"\n", temperature_value, humidity_value);

	bt_ess_set_temperature(temperature_value);
	bt_ess_set_humidity(humidity_value);
}

void main(void)
{
	int err;

	if (!device_is_ready(hdc1080)) {
		printk("HDC1080 not ready\n");
	}
	
	bt_conn_cb_register(&conn_callbacks);
	
	err = bt_enable(bt_ready);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	while (1) {
		k_sleep(K_SECONDS(1));

		bas_notify();

		ess_notify(hdc1080);
	}
}
