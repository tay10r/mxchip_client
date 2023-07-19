#pragma once

#include <uv.h>

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct mxchip_data
{
  /**
   * @brief Accelerometer readings, in terms of mg.
i  */
  int16_t accelerometer[3];

  int16_t gyroscope[3];

  int16_t magnetometer[3];

  float pressure;

  float temperature;

  float humidity;

  uint64_t time;
};

struct mxchip_client;

/**
 * @brief The type of the function that is called when reading from the TCP stream.
 *
 * @param user_data Optional user-defined data.
 *
 * @param client A pointer to the MXChip client instance.
 *
 * @param data A pointer to the data that was read. If an error occurs while reading from the TCP stream, this pointer
 *             gets set to null.
 * */
typedef void
(*mxchip_client_read_cb)(void* user_data, struct mxchip_client* client, const struct mxchip_data* data);

typedef void
(*mxchip_client_connect_cb)(void* user_data, struct mxchip_client* client, int status);

typedef void
(*mxchip_client_disconnect_cb)(void* user_data, struct mxchip_client* client, int status);

typedef void
(*mxchip_client_close_cb)(void* user_data);

struct mxchip_client*
mxchip_client_new(uv_loop_t* loop);

void
mxchip_client_close(struct mxchip_client* client, mxchip_client_close_cb close_cb);

void
mxchip_client_set_user_data(struct mxchip_client* client, void* user_data);

int
mxchip_client_connect(struct mxchip_client* client, const char* ip, mxchip_client_connect_cb connect_cb);

int
mxchip_client_disconnect(struct mxchip_client* client, mxchip_client_disconnect_cb disconnect_cb);

int
mxchip_client_start_read(struct mxchip_client* client, mxchip_client_read_cb read_cb);

int
mxchip_client_stop_read(struct mxchip_client* client);

bool
mxchip_client_connected(const struct mxchip_client* client);

#ifdef __cplusplus
} /* extern "C" */
#endif
