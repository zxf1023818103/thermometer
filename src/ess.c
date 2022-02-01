#include <errno.h>
#include <init.h>
#include <sys/__assert.h>
#include <stdbool.h>
#include <zephyr/types.h>
#include <device.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>
#include <bluetooth/uuid.h>

#define BT_UUID_ESS_TEMPERATURE_VAL 0x2A6E
#define BT_UUID_ESS_TEMPERATURE BT_UUID_DECLARE_16(BT_UUID_ESS_TEMPERATURE_VAL)

#define BT_UUID_ESS_HUMIDITY_VAL 0x2A6F
#define BT_UUID_ESS_HUMIDITY BT_UUID_DECLARE_16(BT_UUID_ESS_HUMIDITY_VAL)

static uint8_t humidity[2];
static uint8_t temperature[2];
static bool temperature_notif_enabled, humidity_notif_enabled;

static void ess_temperature_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	ARG_UNUSED(attr);

	temperature_notif_enabled = (value == BT_GATT_CCC_NOTIFY);

	printk("Temperature Notifications %s\n", temperature_notif_enabled ? "enabled" : "disabled");
}

static void ess_humidity_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	ARG_UNUSED(attr);

	humidity_notif_enabled = (value == BT_GATT_CCC_NOTIFY);

	printk("Humidity Notifications %s\n", humidity_notif_enabled ? "enabled" : "disabled");
}

static ssize_t read_temperature(struct bt_conn *conn,
			       const struct bt_gatt_attr *attr, void *buf,
			       uint16_t len, uint16_t offset)
{
	return bt_gatt_attr_read(conn, attr, buf, len, offset, &temperature, sizeof(temperature));
}

static ssize_t read_humidity(struct bt_conn *conn,
			       const struct bt_gatt_attr *attr, void *buf,
			       uint16_t len, uint16_t offset)
{
	return bt_gatt_attr_read(conn, attr, buf, len, offset, &humidity, sizeof(humidity));
}

BT_GATT_SERVICE_DEFINE(ess,
	BT_GATT_PRIMARY_SERVICE(BT_UUID_ESS),
	BT_GATT_CHARACTERISTIC(BT_UUID_ESS_TEMPERATURE,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_READ, read_temperature, NULL,
			       temperature),
	BT_GATT_CCC(ess_temperature_ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	BT_GATT_CUD("Temperature", BT_GATT_PERM_READ),
	BT_GATT_CHARACTERISTIC(BT_UUID_ESS_HUMIDITY,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_READ, read_humidity, NULL,
			       humidity),
	BT_GATT_CCC(ess_humidity_ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	BT_GATT_CUD("Humidity", BT_GATT_PERM_READ)
);

static int ess_init(void)
{
	return 0;
}

int bt_ess_set_temperature_and_humidity(uint16_t temperature_value, int16_t humidity_value)
{
	int rc = 0;

	static uint16_t old_humidity_value;
	static int16_t old_temperature_value;

	struct bt_gatt_notify_params params[] = {
		{
			.uuid = BT_UUID_ESS_HUMIDITY,
			.attr = &ess.attrs[5],
			.data = humidity,
			.len = sizeof humidity,
		},
		{
			.uuid = BT_UUID_ESS_TEMPERATURE,
			.attr = &ess.attrs[1],
			.data = temperature,
			.len = sizeof temperature,
		},
	};

	if (humidity_notif_enabled && old_humidity_value != humidity_value) {
		old_humidity_value = humidity_value;
		humidity[0] = humidity_value;
		humidity[1] = humidity_value >> 8;

		rc = bt_gatt_notify_cb(NULL, &params[0]);
		if (rc != -ENOTCONN && rc != 0) {
			printk("bt_gatt_notify_cb: %d\n", rc);
			return rc;
		}
	}

	if (temperature_notif_enabled && old_temperature_value != temperature_value) {
		old_temperature_value = temperature_value;
		temperature[0] = temperature_value;
		temperature[1] = temperature_value >> 8;

		rc = bt_gatt_notify_cb(NULL, &params[1]);
		if (rc != -ENOTCONN && rc != 0) {
			printk("bt_gatt_notify_cb: %d\n", rc);
			return rc;
		}
	}

	return 0;
}

SYS_INIT(ess_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
