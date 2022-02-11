#include <Ps3Controller.h>
#include <ESP32Servo.h>

// define servo objects to send torque commands to VESCs using PPM
Servo fl_vesc;
Servo fr_vesc;
Servo bl_vesc;
Servo br_vesc;

float front_torque_factor = 1; // what percentage of commanded torque the front motors get
float rear_torque_factor = 1;  // what percentage of commanded torque the rear motors get
int player = 0;

void onConnect() {
    Serial.println("Connected.");
}

int32_t map_joystick_to_angle(float joystick) {
  return map(joystick, 127, -128, 0, 180);
}

void setup() {
    // setup peripherals
    Serial.begin(115200);

    //setup PPM (servo) outputs to VESCs
    ESP32PWM::allocateTimer(0); //this might conflict with LED timing - but that's for later!
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);

    fl_vesc.setPeriodHertz(50);
    fr_vesc.setPeriodHertz(50);
    bl_vesc.setPeriodHertz(50);
    br_vesc.setPeriodHertz(50);

    fl_vesc.attach(12, 1000, 2000); // 1000us min, 2000us max
    fr_vesc.attach(13, 1000, 2000);
    bl_vesc.attach(14, 1000, 2000);
    br_vesc.attach(15, 1000, 2000);

    fl_vesc.write(map_joystick_to_angle(0));
    fr_vesc.write(map_joystick_to_angle(0));
    bl_vesc.write(map_joystick_to_angle(0));
    br_vesc.write(map_joystick_to_angle(0));

    // setup PS3 controller
    Ps3.attachOnConnect(onConnect);
    Ps3.begin("31:41:59:26:53:58");

    Serial.println("Ready.");
}

void loop() {
    if(!Ps3.isConnected()) {
        Serial.println("No Controller Connected!");
        return;
    }

    // Use bumpers as dead man's switch:
    if(Ps3.data.button.l1 && Ps3.data.button.r1) {
      // Write joystick info to VESCs
      int16_t l_torque = Ps3.data.analog.stick.ly;
      int16_t r_torque = Ps3.data.analog.stick.ry;
      fl_vesc.write(map_joystick_to_angle(front_torque_factor * l_torque));
      fr_vesc.write(map_joystick_to_angle(front_torque_factor * r_torque));
      bl_vesc.write(map_joystick_to_angle(rear_torque_factor * l_torque));
      br_vesc.write(map_joystick_to_angle(rear_torque_factor * r_torque));
    }
    else {
      fl_vesc.write(90);
      fr_vesc.write(90);
      bl_vesc.write(90);
      br_vesc.write(90);
    }

    // Print joystick info
    Serial.print("LJX:");
    Serial.print(Ps3.data.analog.stick.lx, DEC);

    Serial.print(" RJX: ");
    Serial.print(Ps3.data.analog.stick.rx, DEC);

    //Serial.print(" FL: "); Serial.print(map_joystick_to_angle(front_torque_factor * l_torque));
    //Serial.print(" FR: "); Serial.print(map_joystick_to_angle(front_torque_factor * r_torque));
    //Serial.print(" BL: "); Serial.print(map_joystick_to_angle(rear_torque_factor * l_torque));
    //Serial.print(" FR: "); Serial.print(map_joystick_to_angle(rear_torque_factor * r_torque));


    // Print battery info
    int battery = Ps3.data.status.battery;
    Serial.print("  The controller battery is ");
    if( battery == ps3_status_battery_charging )      Serial.println("charging");
    else if( battery == ps3_status_battery_full )     Serial.println("FULL");
    else if( battery == ps3_status_battery_high )     Serial.println("HIGH");
    else if( battery == ps3_status_battery_low)       Serial.println("LOW");
    else if( battery == ps3_status_battery_dying )    Serial.println("DYING");
    else if( battery == ps3_status_battery_shutdown ) Serial.println("SHUTDOWN");
    else Serial.println("UNDEFINED");

    //Ps3.setPlayer(player);
    delay(10);
}
