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

static uint16_t humidity = 0xFFFF;
static int16_t temperature = 0x8000;

static void ess_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	ARG_UNUSED(attr);

	bool notif_enabled = (value == BT_GATT_CCC_NOTIFY);

	printk("ESS Notifications %s\n", notif_enabled ? "enabled" : "disabled");
}

static ssize_t read_temperature(struct bt_conn *conn,
			       const struct bt_gatt_attr *attr, void *buf,
			       uint16_t len, uint16_t offset)
{
	int16_t value = temperature;

	return bt_gatt_attr_read(conn, attr, buf, len, offset, &value, sizeof(value));
}

static ssize_t read_humidity(struct bt_conn *conn,
			       const struct bt_gatt_attr *attr, void *buf,
			       uint16_t len, uint16_t offset)
{
	uint16_t value = humidity;

	return bt_gatt_attr_read(conn, attr, buf, len, offset, &value, sizeof(value));
}

BT_GATT_SERVICE_DEFINE(ess,
	BT_GATT_PRIMARY_SERVICE(BT_UUID_ESS),
	BT_GATT_CHARACTERISTIC(BT_UUID_ESS_TEMPERATURE,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_READ, read_temperature, NULL,
			       &temperature),
	BT_GATT_CHARACTERISTIC(BT_UUID_ESS_HUMIDITY,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_READ, read_humidity, NULL,
			       &humidity),
	BT_GATT_CCC(ess_ccc_cfg_changed,
		    BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
);

static int ess_init(void)
{
	return 0;
}

int16_t bt_ess_get_temperature(void)
{
	return temperature;
}

uint16_t bt_ess_get_humidity(void)
{
	return humidity;
}

int bt_ess_set_humidity(uint16_t value)
{
	int rc;

	if (value > 10000U) {
		return -EINVAL;
	}

	humidity = value;

	rc = bt_gatt_notify(NULL, &ess.attrs[1], &value, sizeof(value));

	return rc == -ENOTCONN ? 0 : rc;
}

int bt_ess_set_temperature(int16_t value)
{
	int rc;

	humidity = value;

	rc = bt_gatt_notify(NULL, &ess.attrs[2], &value, sizeof(value));

	return rc == -ENOTCONN ? 0 : rc;
}

SYS_INIT(ess_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
