#include "common.h"
#include "mlx90614.h"

#include <hal/nrf_gpio.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/dt-bindings/pinctrl/nrf-pinctrl.h>
#include <zephyr/kernel.h>
#include <zephyr/pm/device.h>
#include <zephyr/settings/settings.h>

// 周辺機器定義
static const struct i2c_dt_spec  i2c_mlx90614 = I2C_DT_SPEC_GET(DT_NODELABEL(mlx_sensor));
static const struct gpio_dt_spec sda_gpio     = GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), sda_gpios);
static const struct gpio_dt_spec scl_gpio     = GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), scl_gpios);

// アドバタイズデータの設定
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};
/* カスタムサービスのUUID定義 (128bit) */
static struct bt_uuid_128 temp_service_uuid =
    BT_UUID_INIT_128(BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef0));

static struct bt_uuid_128 temp_char_uuid =
    BT_UUID_INIT_128(BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef1));

static uint8_t temp_value = 25; // 送信する温度データ（例）
/* 2. 読み取りコールバック */
static ssize_t read_temp(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
                         uint16_t len, uint16_t offset)
{
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &temp_value, sizeof(temp_value));
}

static void start_advertising(void)
{
    bt_le_adv_stop();

    struct bt_le_adv_param adv_param;
    adv_param = *BT_LE_ADV_CONN_FAST_2;
    int err   = bt_le_adv_start(&adv_param, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err)
    {
        DEBUG_PRINT("Advertising failed to restart (err %d)\n", err);
    }
    else
    {
        DEBUG_PRINT("Advertising restarted\n");
    }
}
static ssize_t write_cmd(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf,
                         uint16_t len, uint16_t offset, uint8_t flags);
/* カスタムサービスの定義 */
BT_GATT_SERVICE_DEFINE(
    my_temp_svc, BT_GATT_PRIMARY_SERVICE(&temp_service_uuid),
    /* 特徴量（読み取り可能 | 通知可能 | ペアリング/暗号化が必要） */
    BT_GATT_CHARACTERISTIC(&temp_char_uuid.uuid,
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE | BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_READ_ENCRYPT |
                               BT_GATT_PERM_WRITE_ENCRYPT, // 読み取りにはペアリングが必要
                           read_temp, write_cmd, &temp_value),
    BT_GATT_CCC(NULL, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE_ENCRYPT), );

/* 書き込み時のコールバック関数（スマホからデータが届いたら呼ばれる） */
static ssize_t write_cmd(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf,
                         uint16_t len, uint16_t offset, uint8_t flags)
{
    const uint8_t *data = buf;

    // 届いたデータの長さを表示
    DEBUG_PRINT("Received data length: %u\n", len);

    // データを16進数で1つずつ表示
    DEBUG_PRINT("Command data (Hex): ");
    for (uint16_t i = 0; i < len; i++)
    {
        DEBUG_PRINT("%02x ", data[i]);
    }
    DEBUG_PRINT("\n");
    if (len > 0 && data[0] == 0x01)
    { // 例えば「0x01」が届いたら温度測定
        bt_gatt_notify(NULL, &my_temp_svc.attrs[2], &temp_value, sizeof(temp_value));
    }
    return len;
}

/* ペアリング時のコールバック（必要に応じて） */
static void connected(struct bt_conn *conn, uint8_t err)
{
    if (err)
    {
        DEBUG_PRINT("Connection failed (err %u)\n", err);
    }
    else
    {
        DEBUG_PRINT("Connected\n");
    }
}

static volatile bool should_restart_adv = false;

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    DEBUG_PRINT("Disconnected, reason 0x%02x %s\n", reason, bt_hci_err_to_str(reason));
    should_restart_adv = true;
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected    = connected,
    .disconnected = disconnected,
};

/* ペアリング要求が来た時の応答設定 */
static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey)
{
    DEBUG_PRINT("Passkey: %06u\n", passkey); // 6桁の数字をシリアルに出す
}

static void auth_cancel(struct bt_conn *conn)
{
    DEBUG_PRINT("Pairing cancelled\n");
}

/* 認証関係のコールバック構造体 */
static struct bt_conn_auth_cb auth_cb_display = {
    .passkey_display = auth_passkey_display,
    .passkey_entry   = NULL,
    .cancel          = auth_cancel,
};

uint8_t is_ready_drivers(void)
{
    if (!device_is_ready(i2c_mlx90614.bus) || !device_is_ready(sda_gpio.port) ||
        !device_is_ready(scl_gpio.port))
    {
        return 1;
    }
    return 0;
}

int main(void)
{
    uint8_t ret = is_ready_drivers(); //
    if (ret != 0)
    {
        DEBUG_PRINT("Drivers not ready\n");
    }
    else
    {
        DEBUG_PRINT("Drivers ready\n");
    }

    // 2. Bluetoothの有効化
    int err = bt_enable(NULL);
    if (err)
    {
        printf("Bluetooth init failed (err %d)\n", err);
        return 0;
    }
    bt_conn_auth_cb_register(&auth_cb_display);
    bt_passkey_set(123456);
    // 保存されているペアリング情報を読み込む（これを忘れると毎回ペアリングが必要になる）
    if (IS_ENABLED(CONFIG_BT_SETTINGS))
    {
        settings_load();
    }

    printf("Bluetooth initialized\n");
    start_advertising();
    while (1)
    {
        k_msleep(1000);
        temp_value++; // 擬似的に値を更新
        if (temp_value > 50)
            temp_value = 20;
        if (should_restart_adv)
        {
            should_restart_adv = false;
            k_msleep(200);
            start_advertising();
        }

        // 値が変化したことをスマホに通知（Notify）
        bt_gatt_notify(NULL, &my_temp_svc.attrs[2], &temp_value, sizeof(temp_value));
    }

    // while (1)
    // {
    //     // 測定
    //     float env = mlx90614_read_env_temp(&i2c_mlx90614);
    //     float obj = mlx90614_read_obj_temp(&i2c_mlx90614);
    //     DEBUG_PRINT("Object Temp: %.2f C, Env Temp: %.2f C \n", obj, env);

    //     mlx90614_enter_sleep(&i2c_mlx90614, &scl_gpio, &sda_gpio); // センサーをスリープへ
    //     k_msleep(4000);                                            // 3秒待機
    //     mlx90614_exit_sleep(&i2c_mlx90614, &scl_gpio, &sda_gpio);  // センサーをウェイクアップ
    //     // NRF_POWER->SYSTEMOFF = 1;
    // }
    return 0;
}
