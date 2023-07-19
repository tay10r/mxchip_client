#include <mxchip_client/client.h>

#include <stdio.h>
#include <stdlib.h>

static void
on_mxchip_close(void* unused)
{
  printf("socket closed\n");
}

static int counter = 0;

static void
on_mxchip_read(void* unused, struct mxchip_client* client, const struct mxchip_data* data)
{
  if (!data) {
    printf("error occurred while reading\n");
    mxchip_client_stop_read(client);
    return;
  }

  printf("data:\n");
  printf("  accelerometer:\n");
  printf("    x: %f\n", data->accelerometer[0]);
  printf("    y: %f\n", data->accelerometer[1]);
  printf("    z: %f\n", data->accelerometer[2]);

  counter++;

  if (counter == 100)
    mxchip_client_stop_read(client);
}

static void
on_mxchip_connect(void* unused, struct mxchip_client* client, int status)
{
  if (status != 0) {
    printf("unable to connect (%s)\n", uv_strerror(status));
    return;
  }

  printf("connected\n");

  int result = mxchip_client_start_read(client, on_mxchip_read);

  if (result != 0)
    fprintf(stderr, "failed to start reading (%s)\n", uv_strerror(result));
}

static void
on_mxchip_disconnect(void* unused, struct mxchip_client* client, int status)
{
  if (status != 0) {
    printf("unable to disconnect (%s)\n", uv_strerror(status));
    return;
  }

  printf("disconnected\n");
}

int
main(int argc, char** argv)
{
  if (argc != 2) {
    printf("usage: %s <ip>\n", argv[0]);
    return EXIT_FAILURE;
  }

  const char* ip = argv[1];

  uv_loop_t loop;

  uv_loop_init(&loop);

  struct mxchip_client* client = mxchip_client_new(&loop);

  mxchip_client_connect(client, ip, on_mxchip_connect);

  uv_run(&loop, UV_RUN_DEFAULT);

  /* cleanup */

  mxchip_client_disconnect(client, on_mxchip_disconnect);

  mxchip_client_close(client, on_mxchip_close);

  uv_run(&loop, UV_RUN_DEFAULT);

  uv_loop_close(&loop);

  return 0;
}
