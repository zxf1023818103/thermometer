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
	BT_GATT_CHARACTERISTIC(BT_UUID_ESS_HUMIDITY,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_READ, read_humidity, NULL,
			       humidity),
	BT_GATT_CCC(ess_ccc_cfg_changed,
		    BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
);

static int ess_init(void)
{
	return 0;
}

int bt_ess_set_temperature_and_humidity(uint16_t temperature_value, int16_t humidity_value)
{
	int rc;

	humidity[0] = humidity_value >> 8;
	humidity[1] = humidity_value;
	temperature[0] = temperature_value >> 8;
	temperature[1] = temperature_value;

	struct bt_gatt_notify_params params[] = {
		{
			.uuid = BT_UUID_ESS_HUMIDITY,
			.attr = &ess.attrs[1],
			.data = humidity,
			.len = sizeof humidity,
		},
		{
			.uuid = BT_UUID_ESS_TEMPERATURE,
			.attr = &ess.attrs[2],
			.data = temperature,
			.len = sizeof temperature,
		},
	};

	rc = bt_gatt_notify_multiple(NULL, sizeof params / sizeof params[0], params);

	return rc == -ENOTCONN ? 0 : rc;
}

SYS_INIT(ess_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
