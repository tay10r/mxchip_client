#include <mxchip_client/client.h>

#include <stdlib.h>
#include <string.h>

#ifndef MXCHIP_CLIENT_PORT
#define MXCHIP_CLIENT_PORT 3141
#endif

#define MXCHIP_MESSAGE_SIZE 40

struct mxchip_client
{
  uv_tcp_t socket;

  void* user_data;

  mxchip_client_connect_cb connect_cb;

  mxchip_client_disconnect_cb disconnect_cb;

  mxchip_client_read_cb read_cb;

  mxchip_client_close_cb close_cb;

  bool connected;

  uv_buf_t write_bufs[1];

  char close_message[1];

  char read_buffer[MXCHIP_MESSAGE_SIZE];

  size_t read_size;
};

static uv_handle_t*
as_handle(struct mxchip_client* client)
{
  return (uv_handle_t*)&client->socket;
}

static struct mxchip_client*
as_self(uv_handle_t* handle)
{
  return (struct mxchip_client*)uv_handle_get_data(handle);
}

static void
on_mxchip_client_close(uv_handle_t* handle)
{
  struct mxchip_client* client = as_self(handle);

  if (client->close_cb)
    client->close_cb(client->user_data);

  free(client);
}

static void
on_mxchip_client_connect(uv_connect_t* connect, int status)
{
  struct mxchip_client* client = uv_handle_get_data((uv_handle_t*)connect);

  free(connect);

  client->connected = status == 0;

  if (client->connect_cb)
    client->connect_cb(client->user_data, client, status);
}

static void
on_mxchip_client_disconnect_sent(uv_write_t* write_req, int status)
{
  struct mxchip_client* client = uv_handle_get_data((uv_handle_t*)write_req);

  client->connected = false;

  if (client->disconnect_cb)
    client->disconnect_cb(client->user_data, client, status);

  free(write_req);
}

static void
on_mxchip_alloc(uv_handle_t* handle, size_t size, uv_buf_t* buf)
{
  /* We know what size to expect in the message, so ignoring this. */
  (void)size;

  struct mxchip_client* client = (struct mxchip_client*)uv_handle_get_data(handle);

  const size_t remaining = (client->read_size > MXCHIP_MESSAGE_SIZE) ? 0 : (MXCHIP_MESSAGE_SIZE - client->read_size);

  buf->base = &client->read_buffer[client->read_size];

  buf->len = remaining;
}

static void
on_mxchip_read(uv_stream_t* stream, ssize_t size, const uv_buf_t* buf)
{
  /* We have a pointer to 'buf' already. */
  (void)buf;

  struct mxchip_client* client = (struct mxchip_client*)uv_handle_get_data((uv_handle_t*)stream);

  if (size < 0) {
    printf("%s\n", uv_strerror(size));
    if (client->read_cb)
      client->read_cb(stream, client, NULL);
    return;
  }

  client->read_size += size;

  if (client->read_size >= MXCHIP_MESSAGE_SIZE) {

    const struct mxchip_data* data = (const struct mxchip_data*)&client->read_buffer[0];

    if (client->read_cb)
      client->read_cb(client->user_data, client, data);

    const size_t clip_size = client->read_size - MXCHIP_MESSAGE_SIZE;

    memmove(&client->read_buffer[0], &client->read_buffer[MXCHIP_MESSAGE_SIZE], clip_size);

    client->read_size -= MXCHIP_MESSAGE_SIZE;
  }
}

struct mxchip_client*
mxchip_client_new(uv_loop_t* loop)
{
  struct mxchip_client* client = malloc(sizeof(struct mxchip_client));

  if (!client)
    return NULL;

  memset(client, 0, sizeof(struct mxchip_client));

  if (uv_tcp_init(loop, &client->socket) != 0) {
    free(client);
    return NULL;
  }

  uv_handle_set_data(as_handle(client), client);

  client->close_message[0] = 'q';

  client->write_bufs[0].base = client->close_message;

  client->write_bufs[0].len = 1;

  return client;
}

void
mxchip_client_close(struct mxchip_client* client, mxchip_client_close_cb close_cb)
{
  client->close_cb = close_cb;

  uv_close(as_handle(client), on_mxchip_client_close);
}

int
mxchip_client_connect(struct mxchip_client* client, const char* ip, mxchip_client_connect_cb connect_cb)
{
  client->connect_cb = connect_cb;

  struct sockaddr_in address;

  memset(&address, 0, sizeof(address));

  int result = uv_ip4_addr(ip, MXCHIP_CLIENT_PORT, &address);
  if (result != 0)
    return result;

  uv_connect_t* connect = malloc(sizeof(uv_connect_t));
  if (!connect)
    return UV_ENOMEM;

  uv_handle_set_data((uv_handle_t*)connect, client);

  result = uv_tcp_connect(connect, &client->socket, (const struct sockaddr*)&address, on_mxchip_client_connect);
  if (result != 0) {
    free(connect);
    return result;
  }

  return 0;
}

int
mxchip_client_disconnect(struct mxchip_client* client, mxchip_client_disconnect_cb disconnect_cb)
{
  client->disconnect_cb = disconnect_cb;

  uv_write_t* write_req = malloc(sizeof(uv_write_t));

  if (!write_req)
    return UV_ENOMEM;

  uv_handle_set_data((uv_handle_t*)write_req, client);

  int ret = uv_write(write_req, (uv_stream_t*)&client->socket, client->write_bufs, 1, on_mxchip_client_disconnect_sent);
  if (ret != 0) {
    free(write_req);
    return ret;
  }

  return 0;
}

bool
mxchip_client_connected(const struct mxchip_client* client)
{
  return client->connected;
}

int
mxchip_client_start_read(struct mxchip_client* client, mxchip_client_read_cb read_cb)
{
  client->read_cb = read_cb;

  int ret = uv_read_start((uv_stream_t*)&client->socket, on_mxchip_alloc, on_mxchip_read);
  if (ret != 0)
    return ret;

  return 0;
}

int
mxchip_client_stop_read(struct mxchip_client* client)
{
  return uv_read_stop((uv_stream_t*)&client->socket);
}

void
mxchip_client_set_user_data(struct mxchip_client* client, void* user_data)
{
  client->user_data = user_data;
}
