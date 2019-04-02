#include <string.h>
#include <sys/socket.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "nvs_flash.h"
#include "coap.h"

#include "coapapi.pb-c.h"
#include "coapserver.h"
#include "wifi.h"
#include "controller.h"

#define COAP_DEFAULT_TIME_SEC 30
#define COAP_DEFAULT_TIME_USEC 0

const static char *TAG = "CoAP_server";

static coap_async_state_t *async = NULL;

static void res_config_put(coap_context_t *ctx, const coap_endpoint_t *local_if)
{
    coap_pdu_t *response;

    response = coap_pdu_init(async->flags & COAP_MESSAGE_CON, COAP_RESPONSE_CODE(200), 0, COAP_MAX_PDU_SIZE);
    response->hdr->id = coap_new_message_id(ctx);
    if (async->tokenlen)
        coap_add_token(response, async->tokenlen, async->token);

    coap_send(ctx, local_if, &async->peer, response);
    coap_delete_pdu(response);
    coap_async_state_t *tmp;
    coap_remove_async(ctx, async->id, &tmp);
    coap_free_async(async);
    async = NULL;
}

/*
 * The resource handler
 */
static void hnd_config_put(coap_context_t *ctx, struct coap_resource_t *resource,
    const coap_endpoint_t *local_interface, coap_address_t *peer,
    coap_pdu_t *request, str *token, coap_pdu_t *response)
{
    size_t size;
    uint8_t *data;
    Ledapi__Config *config;

    coap_get_data(request, &size, &data);
    ESP_LOGI(TAG, "******PAYLOAD SIZE: %d", size);
    config = ledapi__config__unpack(NULL, size, data);
    controller_configure(config);
    ledapi__config__free_unpacked(config, NULL);

    async = coap_register_async(ctx, peer, request, COAP_ASYNC_SEPARATE | COAP_ASYNC_CONFIRM, (void*)"no data");
}

void coap_task(void *pParam)
{
    coap_context_t*  ctx = NULL;
    coap_address_t   serv_addr;
    coap_resource_t* resource = NULL;
    fd_set           readfds;
    struct timeval tv;
    int flags = 0;

    while (1) {
        /* Wait for the callback to set the CONNECTED_BIT in the
           event group.
        */
        wifi_wait(portMAX_DELAY);
        ESP_LOGI(TAG, "Connected to AP");

        /* Prepare the CoAP server socket */
        coap_address_init(&serv_addr);
        serv_addr.addr.sin.sin_family      = AF_INET;
        serv_addr.addr.sin.sin_addr.s_addr = INADDR_ANY;
        serv_addr.addr.sin.sin_port        = htons(COAP_DEFAULT_PORT);
        ctx                                = coap_new_context(&serv_addr);
        if (ctx) {
            flags = fcntl(ctx->sockfd, F_GETFL, 0);
            fcntl(ctx->sockfd, F_SETFL, flags|O_NONBLOCK);

            tv.tv_usec = COAP_DEFAULT_TIME_USEC;
            tv.tv_sec = COAP_DEFAULT_TIME_SEC;
            /* Initialize the resource */
            resource = coap_resource_init((unsigned char *)"config", 6, 0);
            if (resource){
                coap_register_handler(resource, COAP_REQUEST_PUT, hnd_config_put);
                coap_add_resource(ctx, resource);
                /*For incoming connections*/
                for (;;) {
                    FD_ZERO(&readfds);
                    FD_CLR( ctx->sockfd, &readfds);
                    FD_SET( ctx->sockfd, &readfds);

                    int result = select( ctx->sockfd+1, &readfds, 0, 0, &tv );
                    if (result > 0){
                        if (FD_ISSET( ctx->sockfd, &readfds ))
                            coap_read(ctx);
                    } else if (result < 0){
                        break;
                    }

                    if (async) {
                        res_config_put(ctx, ctx->endpoint);
                    }
                }
            }

            coap_free_context(ctx);
        }
    }

    vTaskDelete(NULL);
}
