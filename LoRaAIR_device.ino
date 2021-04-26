#include <ArduinoJson.h>
#include <sps30.h>

#define RXD2 16
#define TXD2 17



int16_t ret;
uint8_t auto_clean_days = 4;
uint32_t auto_clean;
struct sps30_measurement m;
float pm10, pm25;

String output;

void sps30_start(){
  sensirion_i2c_init();

  while (sps30_probe() != 0) {
    Serial.print("SPS sensor probing failed\n");
    delay(500);
  }

  ret = sps30_set_fan_auto_cleaning_interval_days(auto_clean_days);
  if (ret) {
    Serial.print("error setting the auto-clean interval: ");
    Serial.println(ret);
  }

  ret = sps30_start_measurement();
  if (ret < 0) {
    Serial.print("error starting measurement\n");
  }

}

void sps30_getdata(){
  uint16_t data_ready;
  int16_t ret;

  do {
    ret = sps30_read_data_ready(&data_ready);
    if (ret < 0) {
      Serial.print("error reading data-ready flag: ");
      Serial.println(ret);
    } else if (!data_ready)
      Serial.print("data not ready, no new measurement available\n");
    else
      break;
    delay(100); /* retry in 100ms */
  } while (1);

  ret = sps30_read_measurement(&m);
  if (ret < 0) {
    Serial.print("error reading measurement\n");
  } else {
    pm10=m.mc_10p0;
    pm25=m.mc_2p5;
  }

}


void setup() {
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  delay(2000);
  sps30_start();
  
}

void loop() {
  sps30_getdata();
  StaticJsonDocument<32> doc;

  doc["pm25"] = pm25;
  doc["pm10"] = pm10;

  serializeJson(doc, output);
  Serial.println(output);
  Serial2.print(output);
  output="";
  delay(3000);
}
