#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "door_control_task.h"
#include "servo.h"
#include "esp_log.h"
#include "utils.h"
#include "cJSON.h"
#include "esp_timer.h"

static const char* TAG = "task";
static const char* OPEN_CMD = "open";

QueueHandle_t      door_queue;
esp_timer_handle_t timer_handle;

static void DoorOpen()
{
    printf("DoorOpen\r\n");
    for (int i = -180; i < 180; i += 20)
    {
        ChangeAngle(i);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

static void DoorClose()
{
    printf("DoorClose\r\n");
    esp_timer_stop(timer_handle);
    for (int i = 180; i >= 0; i -= 20)
    {
        ChangeAngle(i);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

static void InitDoorCloseTimer()
{
    const esp_timer_create_args_t timer_args =
    {
        .callback = &DoorClose,
        .arg = NULL,
        .name = "timer"
    };
    esp_timer_create(&timer_args, &timer_handle);
}

static void CreateDoorQueue()
{   
    door_queue = xQueueCreate(5, sizeof(door_msg_t));
    if (door_queue == NULL)
    {
        /* Todo */
    }
}

static void DataProcessing(door_msg_t* msg)
{
    unsigned char* data = msg->data;
    cJSON* root = cJSON_Parse((char* )data);

    if ((msg->data != NULL) && (msg->len >= 1) && root)
    {
        cJSON* order_item     = cJSON_GetObjectItem(root, "order");
        cJSON* open_time_item = cJSON_GetObjectItem(root, "max_open_time");
        if (!order_item || !open_time_item)
        {
            goto free;
        }

        char* order = cJSON_GetStringValue(order_item);
        if (order && (!strncmp(order, "open", CONST_SIZEOF("open"))))
        {
            int open_time = open_time_item->valueint;
            if (open_time > 0 && open_time < 60)
            {
                printf("open_time : [%d]\r\n", open_time);
                esp_timer_start_periodic(timer_handle, open_time * 1000000);
            }
            DoorOpen();
        }

        free:
        if (root)
        {
            cJSON_Delete(root);
        }       
    }
}

static void DoorTask(void* pv_parameters)
{
    CreateDoorQueue();
    InitServo();
    InitDoorCloseTimer();

    for (;;)
    {
        door_msg_t msg = {0,};
        xQueueReceive(door_queue, &msg, portMAX_DELAY);

        DataProcessing(&msg);

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void InitDoorTask()
{
    printf("init door task \r\n");
    xTaskCreate(DoorTask, "Door_task", 4096, NULL, 1, NULL);   
}