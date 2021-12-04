/*
 * suzuki steering sw
 *   (c) 2021 Hayato Doi
 * スズキ車用ステアリングスイッチの読み込みスクリプト
 * スズキ ジムニーシエラ(3BA-JB74W)でのみ動作確認
 */

#include "HID-Project.h"

// ステアリングスイッチの抵抗値
enum STEERING_SW {
  UNKNOWN = -1,
  NONE = 0,
  VOL_UP,
  VOL_DOWN,
  MODE,
  LEFT,
  RIGHT,
  MUTE,
  TEST // for debug.
};
typedef struct {
  enum STEERING_SW id;
  char name[80];
  int code;
  int value;  // テスターで測定した値
} STEERING_SW_TABLE_TYPE;
const STEERING_SW_TABLE_TYPE STEERING_SW_TABLE[] {
  {NONE,      "NONE",     0,                  5172},  // 何も押されていない
  {VOL_UP,    "VOL_UP",   MEDIA_VOLUME_UP,    133},   // ボリュームアップ
  {VOL_DOWN,  "VOL_DOWN", MEDIA_VOLUME_DOWN,  243},   // ボリュームダウン
  {MODE,      "MODE",     MEDIA_PLAY_PAUSE,   424},   // Mode/Selectボタン
  {LEFT,      "LEFT",     MEDIA_PREV,         1572},  // 曲戻り
  {RIGHT,     "RIGHT",    MEDIA_NEXT,         752},   // 曲送り
  {MUTE,      "MUTE",     MEDIA_VOLUME_MUTE,  58},    // ミュート
  {TEST,      "TEST",     MEDIA_VOLUME_DOWN,   320},    // for debug.
};

const int ALLOWABLE = 20;    // 抵抗値の誤差範囲
const int INPUT_PIN = A0;
const int REFERENCE_5V_PIN = A3;  // 入力値確認用
const int REFERENCE_GND_PIN = A1; // 入力値確認用
const float STANDARD_RESISTANCE = 5000.;
const int READ_COUNT = 16; // この回数回データを読み込んで平均値を使用する

enum STEERING_SW old_sw_id = UNKNOWN;

enum STEERING_SW get_steering_sw (void) {
  int input   = analogRead(INPUT_PIN);
  int ref_5v  = analogRead(REFERENCE_5V_PIN);
  int ref_gnd = analogRead(REFERENCE_GND_PIN);
  for (int i = 0; i < READ_COUNT; i++) {
    input   += analogRead(INPUT_PIN);
    ref_5v  += analogRead(REFERENCE_5V_PIN);
    ref_gnd += analogRead(REFERENCE_GND_PIN);
  }
  float rx = STANDARD_RESISTANCE * (input - ref_gnd)/(ref_5v - input);
  int loop_end = sizeof(STEERING_SW_TABLE)/sizeof(STEERING_SW_TABLE_TYPE);
  for (int i = 0; i < loop_end; i++) {
    if (rx > STEERING_SW_TABLE[i].value - ALLOWABLE &&
        rx < STEERING_SW_TABLE[i].value + ALLOWABLE) {
      return STEERING_SW_TABLE[i].id;
    }
  }
  return UNKNOWN;
}

void setup (void) {
  Serial.begin (9600);
  Consumer.begin();
}

void loop (void) {
  enum STEERING_SW sw_id = get_steering_sw();
  bool key_changed = sw_id != old_sw_id;
  int loop_end = sizeof(STEERING_SW_TABLE)/sizeof(STEERING_SW_TABLE_TYPE);

  old_sw_id = sw_id;
  Serial.print("old_sw_id = ");
  Serial.println(old_sw_id);
  for (int i = 0; i <= loop_end; i++) {
    if (i == loop_end) {
      Serial.println("sw = UNKOWN");
      break;
    }
    if (key_changed && sw_id == STEERING_SW_TABLE[i].id) {
      Serial.print("sw = ");
      Serial.println(STEERING_SW_TABLE[i].name);
      Consumer.write(STEERING_SW_TABLE[i].code);
      break;
    }
  }
  // delay
  delay (10);
}
