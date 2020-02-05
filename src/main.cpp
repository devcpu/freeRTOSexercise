#include <Arduino.h>
#include <esp_freertos_hooks.h>
#include <freertos/FreeRTOS.h>
#include <freertos/FreeRTOSConfig.h>
#include <freertos/queue.h>
#include <freertos/task.h>

void tx(void *pParam);
void rx(void *pParam);
void rxput(void *pParam);
void txput(void *pParam);

struct message {
  String msg = "";
  uint64_t counter = 0;
};

uint64_t counter0 = 0UL;
uint64_t counter1 = 0UL;
uint64_t txputcounter = 0UL;
uint64_t rxputcounter = 0UL;

bool Core0IdleHook(void) {
  counter0++;
  //Serial.print("+");
  return true;
}

bool Core1IdleHook(void) {
  counter1++;
  // Serial.print("*");
  return true;
}

QueueHandle_t TXQueue, RXQueue;

void setup() {
  Serial.begin(500000);
  delay(2000);

  esp_register_freertos_idle_hook_for_cpu(&Core0IdleHook, 0);
  esp_register_freertos_idle_hook_for_cpu(&Core1IdleHook, 1);

  TXQueue = xQueueCreate(5, sizeof(char) * 256);
  RXQueue = xQueueCreate(5, sizeof(char) * 256);

  static const char *RXtxt = "Lala";
  if (xTaskCreatePinnedToCore(rx, "rx task", 15000, (void *)RXtxt, 2, NULL,                             1) == pdPASS) {
    Serial.printf("rx    task created, all fine\n");
  } else {
    Serial.printf("-ERR rx task creation fails\n");
  }


  if (xTaskCreatePinnedToCore(txput, "txput task", 15000, NULL, 1, NULL, 1) ==
      pdPASS) {
    Serial.printf("txput task created, all fine\n");
  } else {
    Serial.printf("-ERR txput task creation fails\n");
  }

  if (xTaskCreatePinnedToCore(rxput, "rxput task", 15000, NULL, 1, NULL, 0) ==
      pdPASS) {
    Serial.printf("rxput task created, all fine\n");
  } else {
    Serial.printf("-ERR rxput task creation fails\n");
  }

  if (xTaskCreatePinnedToCore(tx, "tx task", 15000, NULL, 3, NULL, 0) ==
      pdPASS) {
    Serial.printf("tx    task created, all fine\n");
  } else {
    Serial.printf("-ERR tx task creation fails\n");
  }


  delay(3000);
}

void loop() {
  // put your main code here, to run repeatedly:
}

void tx(void *pParam) {
  message msg;
  while (1) {
    Serial.printf(
        "\nfunction tx running on CPU:%d CPU0-Idle: %llu CPU1-Idle: %llu\n",
        xPortGetCoreID(), counter0, counter1);
    while (xQueueReceive(TXQueue, &msg, pdMS_TO_TICKS(100)) == pdPASS) {
      Serial.printf("TXMSG: number: %llu MSG: %s\n", msg.counter,
                    msg.msg.c_str());
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }
  vTaskDelete(NULL);
}

void rx(void *pParam) {
  char *parameter;
  parameter = (char *)pParam;
  message msg;
  while (1) {
    Serial.printf("\nfunction rx running on CPU:%d %s\n", xPortGetCoreID(),
                  parameter);
    while (xQueueReceive(RXQueue, &msg, pdMS_TO_TICKS(100)) != errQUEUE_EMPTY) {
      Serial.printf("RXMSG: number: %llu MSG: %s\n", msg.counter,
                    msg.msg.c_str());
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }
  vTaskDelete(NULL);
}

void rxput(void *pParam) {
  message rxmsg;
  rxmsg.msg = String("RXMSG");// und jetzt noch einen ganzen Haufen leeres Blabla Blah. Hallo Welt, wie sie mir gefällt und so weiter und sol fort!");
  while (1) {
    rxmsg.counter = ++rxputcounter;
    if (xQueueSendToBack(RXQueue, &rxmsg, pdMS_TO_TICKS(10)) == errQUEUE_FULL) {
      Serial.printf("**********ERR RXQueue full!********************************\n");
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
  vTaskDelete(NULL);
}

void txput(void *pParam) {
  message txmsg;
  
  txmsg.msg = String("TXMSG"); // und jetzt noch einen ganzen Haufen leeres Blabla Blah. Hallo Welt, wie sie mir gefällt und so weiter und sol fort!");
  while (1) {
    txmsg.counter = ++txputcounter;
    if (xQueueSendToBack(TXQueue, &txmsg, pdMS_TO_TICKS(10)) == errQUEUE_FULL) {
      Serial.printf("-----------ERR TXQueue full!--------------------------------\n");
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
  vTaskDelete(NULL);
}
