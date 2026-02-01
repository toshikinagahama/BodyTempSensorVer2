
#include "my_ble.h"

#include "common.h"
#include "global.h"

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>

// アドバタイズデータの設定
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME,
            sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};
/* カスタムサービスのUUID定義 (128bit) */
static struct bt_uuid_128 temp_service_uuid = BT_UUID_INIT_128(
    BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef0));

static struct bt_uuid_128 temp_char_uuid = BT_UUID_INIT_128(
    BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef1));

/* 2. 読み取りコールバック */
static ssize_t read_temp(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                         void *buf, uint16_t len, uint16_t offset)
{
    uint8_t temp_value = 36; // 例として36度を返す
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &temp_value,
                             sizeof(temp_value));
}

static struct k_work_delayable le_param_work;

static bool is_advertising = false;

static struct bt_le_conn_param *connection_param = BT_LE_CONN_PARAM(
    400, // Min interval: 400 * 1.25ms = 500ms
    400, // Max interval: 400 * 1.25ms = 500ms
    4,   // Latency: スレーブ側が4回まで通信をスルーできる（さらに省エネ）
    600  // Timeout: 400 * 10ms = 4000ms
);

void start_advertising(void)
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
        is_advertising = true;
        DEBUG_PRINT("Advertising restarted\n");
    }
}

void stop_advertising(void)
{
    int err = bt_le_adv_stop();
    if (err)
    {
        DEBUG_PRINT("Failed to stop advertising (err %d)\n", err);
    }
    else
    {
        is_advertising = false;
        DEBUG_PRINT("Advertising stopped\n");
    }
}

bool get_is_advertising(void)
{
    return is_advertising;
}

static ssize_t write_cmd(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                         const void *buf, uint16_t len, uint16_t offset,
                         uint8_t flags);
/* カスタムサービスの定義 */
BT_GATT_SERVICE_DEFINE(
    my_data_svc, BT_GATT_PRIMARY_SERVICE(&temp_service_uuid),
    /* 特徴量（読み取り可能 | 通知可能 | ペアリング/暗号化が必要） */
    BT_GATT_CHARACTERISTIC(
        &temp_char_uuid.uuid,
        BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE | BT_GATT_CHRC_NOTIFY,
        BT_GATT_PERM_READ_ENCRYPT |
            BT_GATT_PERM_WRITE_ENCRYPT, // 読み取りにはペアリングが必要
        read_temp, write_cmd, NULL),
    BT_GATT_CCC(NULL, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE_ENCRYPT), );

/* 書き込み時のコールバック関数（スマホからデータが届いたら呼ばれる） */
static ssize_t write_cmd(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                         const void *buf, uint16_t len, uint16_t offset,
                         uint8_t flags)
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
    }
    return len;
}
static void update_le_params(struct k_work *work)
{
    // 接続中の conn オブジェクトを保持している必要がありますが、
    // 面倒な場合は NULL を渡すと全ての接続に対して更新を試みます。
    bt_conn_le_param_update(NULL, connection_param);
    DEBUG_PRINT("Sent late LE param update request\n");
}
static void le_param_updated(struct bt_conn *conn, uint16_t interval,
                             uint16_t latency, uint16_t timeout)
{
    // interval が 400 (500ms) になっていれば成功！
    // もし 24 (30ms) などのままであれば、スマホに拒否されています。
    DEBUG_PRINT("LE params updated: interval %d, latency %d\n", interval,
                latency);
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
        // k_work_init_delayable(&le_param_work, update_le_params);
        // k_work_schedule(&le_param_work, K_MSEC(2000));
        int update_err = bt_conn_le_param_update(conn, connection_param);
        if (update_err)
        {
            DEBUG_PRINT("Param update request failed (err %d)\n", update_err);
        }
        else
        {
            DEBUG_PRINT("Param update request sent!\n");
        }
        enqueue(EVT_BLE_CONNECTED, NULL, 0);
    }
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    DEBUG_PRINT("Disconnected, reason 0x%02x %s\n", reason,
                bt_hci_err_to_str(reason));
    enqueue(EVT_BLE_DISCONNECTED, NULL, 0);
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected        = connected,
    .disconnected     = disconnected,
    .le_param_updated = le_param_updated,
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

void my_ble_init(void)
{
    int err = bt_enable(NULL);
    if (err)
    {
        return;
    }
    bt_conn_auth_cb_register(&auth_cb_display);
    bt_passkey_set(123456);
    // 保存されているペアリング情報を読み込む（これを忘れると毎回ペアリングが必要になる）
    if (IS_ENABLED(CONFIG_BT_SETTINGS))
    {
        settings_load();
    }

    // start_advertising();
}
void my_ble_notify(void *data, size_t size)
{
    bt_gatt_notify(NULL, &my_data_svc.attrs[2], data, size);
}