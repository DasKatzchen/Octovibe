#include <Arduino.h>

#include "config.h"
#include "src/tcode/tcode.h"
#include "src/communication/ICommunication.h"
#include "src/communication/BLECommunication.h"
//#include "src/esp32-softap-ota/softap_ota.h"

static ICommunication* comm;

struct
{
    TCodeAxis V0{"V0", 0, 0, 255};
    TCodeAxis V1{"V1", 0, 0, 255};
    TCodeAxis V2{"V2", 0, 0, 255};
    TCodeAxis V3{"V3", 0, 0, 255};
    TCodeAxis V4{"V4", 0, 0, 255};
    TCodeAxis V5{"V5", 0, 0, 255};
    TCodeAxis V6{"V6", 0, 0, 255};
    TCodeAxis V7{"V7", 0, 0, 255};
} axes;

// Immediately set every axis to 0, cancelling any ramp in progress.
void stop_all_axes() {
    uint32_t now = micros();
    axes.V0.set(now, 0, 0);
    axes.V1.set(now, 0, 0);
    axes.V2.set(now, 0, 0);
    axes.V3.set(now, 0, 0);
    axes.V4.set(now, 0, 0);
    axes.V5.set(now, 0, 0);
    axes.V6.set(now, 0, 0);
    axes.V7.set(now, 0, 0);
}

// Drive the motor outputs from the axes' (possibly ramping) values at time t.
void write_outputs(uint32_t t) {
    analogWrite(V0_PIN, axes.V0.get_remap(t));
    analogWrite(V1_PIN, axes.V1.get_remap(t));
    analogWrite(V2_PIN, axes.V2.get_remap(t));
    analogWrite(V3_PIN, axes.V3.get_remap(t));
    analogWrite(V4_PIN, axes.V4.get_remap(t));
    analogWrite(V5_PIN, axes.V5.get_remap(t));
    analogWrite(V6_PIN, axes.V6.get_remap(t));
    analogWrite(V7_PIN, axes.V7.get_remap(t));
}

void tcode_D0() {
    comm->output("Octovibe version 0.5");
}

void tcode_D1() {
    comm->output("TCode version 0.3");
}

void tcode_D2() {
    comm->output("8 vibration axes");
}

void tcode_DSTOP() {
    stop_all_axes();
}

//void tcode_DUPDATE() {
//    activate_ota();
//}

struct {
    TCodeDeviceCommand d0{"0", &tcode_D0};
    TCodeDeviceCommand d1{"1", &tcode_D1};
    TCodeDeviceCommand d2{"2", &tcode_D2};
    TCodeDeviceCommand d_stop{"STOP", &tcode_DSTOP};
    //TCodeDeviceCommand d_update{"UPDATE", &tcode_DUPDATE};
} tcode_device_commands;

TCode tcode(reinterpret_cast<TCodeAxis *>(&axes), sizeof(axes) / sizeof(TCodeAxis),
            reinterpret_cast<TCodeDeviceCommand *>(&tcode_device_commands), sizeof(tcode_device_commands) / sizeof(TCodeDeviceCommand));


void setup()
{
    //validate_ota();
    pinMode(DEBUG_LED, OUTPUT);
    pinMode(V0_PIN, OUTPUT);
    pinMode(V1_PIN, OUTPUT);
    pinMode(V2_PIN, OUTPUT);
    pinMode(V3_PIN, OUTPUT);
    pinMode(V4_PIN, OUTPUT);
    pinMode(V5_PIN, OUTPUT);
    pinMode(V6_PIN, OUTPUT);
    pinMode(V7_PIN, OUTPUT);
    write_outputs(micros());
    digitalWrite(DEBUG_LED, HIGH);
    comm = new BLECommunication();
    comm->start();
}

void loop()
{
    if (comm->isOpen()) {
        digitalWrite(DEBUG_LED, HIGH);
        char buffer[BLE_MAX_MESSAGE_LENGTH];
        while (comm->readData(buffer, sizeof(buffer))) {
            tcode.parse_single_line(buffer, strlen(buffer));
            #ifdef BT_ECHO
            uint32_t t0 = micros();
            Serial.println(axes.V0.get_remap(t0));
            Serial.println(axes.V1.get_remap(t0));
            Serial.println(axes.V2.get_remap(t0));
            Serial.println(axes.V3.get_remap(t0));
            Serial.println(axes.V4.get_remap(t0));
            Serial.println(axes.V5.get_remap(t0));
            Serial.println(axes.V6.get_remap(t0));
            Serial.println(axes.V7.get_remap(t0));
            #endif
        }
        // Update every pass, not just when a message arrives, so I-interval ramps progress.
        write_outputs(micros());
    }
    else {
        // Never leave the motors running without a connected controller.
        stop_all_axes();
        write_outputs(micros());

        digitalWrite(DEBUG_LED, HIGH);
        delay(1000);
        digitalWrite(DEBUG_LED, LOW);
        delay(1000);
    }
}
