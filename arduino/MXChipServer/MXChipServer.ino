#include "AZ3166WiFi.h"
#include "IoT_DevKit_HW.h"
#include "SystemTickCounter.h"

#ifndef PORT

/**
 * @brief The TCP port to listen for incoming connections on.
 */
#define PORT 3141

#endif

#ifndef READ_INTERVAL

/**
 * @brief The number of milliseconds between read operations.
 */
#define READ_INTERVAL 10

#endif

/**
 * @brief Whether or not the device was properly initialized.
 */
static bool g_is_initialized = false;

/**
 * @brief The number of milliseconds since the last read operation.
 */
static volatile uint64_t g_last_read_time = 0;

/**
 * @brief The buffer used to send sensor data to connected clients.
 *
 * @details
 *  It consists of:
 *  - accelerometer  (3xi16) = 6
 *  - gyroscope      (3xi16) = 12
 *  - magnetometer   (3xi16) = 18
 *  - pressure       (1xf32) = 22
 *  - temperature    (1xf32) = 26
 *  - humidity       (1xf32) = 30
 *  - time           (1xu64) = 38
 *  - magic "MX"     (2xi8)  = 40
 */

static uint8_t g_send_buffer[40];

/**
 * @brief The TCP server that listens for incoming telemetry clients.
 */
WiFiServer g_server(PORT);

void show_ipv4() {
  IPAddress ip = WiFi.localIP();

  char buf[64];
  snprintf(buf, sizeof(buf), "%s\r\n%d\r\n", ip.get_address(), PORT);

  textOutDevKitScreen(0, buf, 1);
}

void showInitError(int err) {
  cleanDevKitScreen();
  // Something wrong
  switch (err) {
    case -100:
      textOutDevKitScreen(0, "No WiFi\r\nEnter AP Mode\r\nto config", 1);
      break;
    case -101:
      textOutDevKitScreen(0, "Init failed\r\n I2C", 1);
      break;
    case -102:
      textOutDevKitScreen(0, "Init failed\r\n LSM6DSL\r\n   sensor", 1);
      break;
    case -103:
      textOutDevKitScreen(0, "Init failed\r\n HTS221\r\n   sensor", 1);
      break;
    case -104:
      textOutDevKitScreen(0, "Init failed\r\n LIS2MDL\r\n   sensor", 1);
      break;
    case -105:
      textOutDevKitScreen(0, "Init failed\r\n IRDA", 1);
      break;
    case -106:
      textOutDevKitScreen(0, "Init failed\r\n LPS22HB\r\n   sensor", 1);
      break;
  }
}

void setup() {
  int ret = initIoTDevKit(1);
  if (ret != 0) {
    showInitError(ret);
    return;
  }

  g_server.begin();

  g_server.setTimeout(0);

  g_is_initialized = true;

  cleanDevKitScreen();

  show_ipv4();

  turnOffUserLED();
}

void loop() {
  if (!g_is_initialized) return;

  WiFiClient client = g_server.available();

  if (client) {

    turnOnUserLED();

    bool should_close = false;

    while (client.connected() && !should_close) {

      if (client.available())
        should_close = client.read() == 'q';

      uint64_t ms = SystemTickCounterRead() - g_last_read_time;

      if (ms < READ_INTERVAL) {
        delay(READ_INTERVAL - ms);
        continue;
      }

      int xyz[3];

      getDevKitAcceleratorValue(&xyz[0], &xyz[1], &xyz[2]);
      memcpy(&g_send_buffer[0], xyz, sizeof(xyz));

      getDevKitGyroscopeValue(&xyz[0], &xyz[1], &xyz[2]);
      memcpy(&g_send_buffer[6], xyz, sizeof(xyz));

      getDevKitMagnetometerValue(&xyz[0], &xyz[1], &xyz[2]);
      memcpy(&g_send_buffer[12], xyz, sizeof(xyz));

      float f32 = getDevKitPressureValue();
      memcpy(&g_send_buffer[18], &f32, sizeof(f32));

      f32 = getDevKitTemperatureValue(0 /* 0 for Celcius */);
      memcpy(&g_send_buffer[22], &f32, sizeof(f32));

      f32 = getDevKitHumidityValue();
      memcpy(&g_send_buffer[26], &f32, sizeof(f32));

      uint64_t time = SystemTickCounterRead();
      memcpy(&g_send_buffer[30], &time, sizeof(time));

      g_send_buffer[38] = static_cast<uint8_t>('M');
      g_send_buffer[39] = static_cast<uint8_t>('X');

      client.write(g_send_buffer, sizeof(g_send_buffer));
    }

    turnOffUserLED();
  }
}

#if 0
void showMotionGyroSensor() {
  int x, y, z;
  snprintf(buffInfo, sizeof(buffInfo),
           "Gyroscope \r\n    x:%d   \r\n    y:%d   \r\n    z:%d  ", x, y, z);
  textOutDevKitScreen(0, buffInfo, 1);
}

void showMotionAccelSensor() {
  int x, y, z;
  snprintf(buffInfo, sizeof(buffInfo),
           "Accelerometer \r\n    x:%d   \r\n    y:%d   \r\n    z:%d  ", x, y,
           z);
  textOutDevKitScreen(0, buffInfo, 1);
}

void showPressureSensor() {
  uint64_t ms = SystemTickCounterRead() - g_last_read_time;
  if (ms < READ_ENV_INTERVAL) {
    return;
  }

  snprintf(buffInfo, sizeof(buffInfo),
           "Environment\r\nPressure: \r\n   %0.2f hPa\r\n  ", pressure);
  textOutDevKitScreen(0, buffInfo, 1);
  g_last_read_time = SystemTickCounterRead();
}

void showHumidTempSensor() {
  if (ms < READ_ENV_INTERVAL) {
    return;
  }
  float tempF = tempC * 1.8 + 32;

  snprintf(
      buffInfo, sizeof(buffInfo),
      "Environment \r\n Temp:%0.2f F \r\n      %0.2f C \r\n Humidity:%0.2f%%",
      tempF, tempC, humidity);
  textOutDevKitScreen(0, buffInfo, 1);

  g_last_read_time = SystemTickCounterRead();
}

void showMagneticSensor() {
  int x, y, z;
  snprintf(buffInfo, sizeof(buffInfo),
           "Magnetometer \r\n    x:%d   \r\n    y:%d   \r\n    z:%d  ", x, y,
           z);
  textOutDevKitScreen(0, buffInfo, 1);
}
#endif

