/*
  Connect 5VT to D9 and 5VR to D10
  Feed GSM SIM900A with Arduino's 5V
  Code by Piyush Aggarwal
*/

#include <SoftwareSerial.h>
#include <math.h>
#include <Keypad.h>
// pins reserved by Keypad
byte rowPins[4] = { 9, 7, 8, 6 };
byte colPins[4] = { 5, 4, 3, 2 };
char keys[4][4] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, 4, 4 );

// pin reserved by Lock
int Lock = A0;
// pins reserved by SIM900A
SoftwareSerial SIM900A(10, 11);
SoftwareSerial GPS(4, 3); // TX,RX (as written on module)
// FOR GPS
bool first_loop_exec;
#define GPS_INFO_BUFFER_SIZE 128
char GPS_info_char;
char GPS_info_buffer[GPS_INFO_BUFFER_SIZE];
unsigned int received_char;
int i; // counter
bool message_started;
char gps_loc[29];
char location[30];
int position = 0;
char ph_no[14] = {'\0'};
char check;
char otp[7] = "XXXXXX\0";
int number;
void LockedPosition(int lock_it)
{
  if (lock_it) digitalWrite(Lock, LOW);
  else digitalWrite(Lock, HIGH);
}
void set_gps_module() {
  GPS.begin(9600);
  GPS.println("Connected");

  first_loop_exec = true;
  i = 0;
  message_started = false;
}


void set_sim_module() {
  SIM900A.begin(9600);   // Setting the baud rate of GSM Module
  // command one : AT+COPS?
  SIM900A.println("AT+COPS?");
  delay(1000);
  // SIM900A speaks
  while ((SIM900A.available() > 0)) Serial.write(SIM900A.read());
  delay(100);

  // command two : AT+CLCC=1
  SIM900A.println("AT+CLCC=1");
  delay(1000);
  // SIM900A speaks
  while ((SIM900A.available() > 0)) Serial.write(SIM900A.read());
  delay(100);
  Serial.println("SIM900A ready!");
}

void set_otp_module() {
  randomSeed(analogRead(0)); // make sure this pin is UNCONNECTED AND ANALOG
}
// gets the phone number from incoming call as "+91XXXXXXXXXX"
void get_ph_no() {

  Serial.println("Inside get_ph_no()");
  while (true) {
    check = SIM900A.read(); // store each character in a char named `check`
    delay(50);
    if (check == '\"') {
      Serial.println("Hey I ran!");
      for (int i = 0; i < 13; i++) {
        ph_no[i] = SIM900A.read();
        delay(50);
      }
      break;
    }
  }

  ph_no[13] = '\0';
  // once you got the phone number, hang the call and flush the output from SIM900A
  SIM900A.println("ATH");
  delay(1000);
  while (SIM900A.available()) Serial.write(SIM900A.read());
}
// calculates a purely random OTP every time
void calc_otp() {

  Serial.println("Inside calc_otp()");
  number = random(99, 1000);
  int n = 6;
  int i;
  for ( i = 0; i < 3; i++)
  {
    otp[i] = number % 10 + '0';
    number /= 10;
  }
  number = random(99, 1000);
  for ( i = 3; i < 6; i++)
  {
    otp[i] = number % 10 + '0';
    number /= 10;
  }

  otp[n] = '\0';
  //  Serial.println("Sending you the OTP in 10...");
  delay(1000);
  Serial.println(otp);
}
// Sends an OTP to the caller
void send_otp() {

  Serial.println("Inside send_otp()");
  SIM900A.println("AT+CMGF=1");
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  char dest_set[21] = "AT+CMGS=\"XXXXXXXXXX\"\r";
  for (int i = 0; i < 10; i++) {
    dest_set[i + 9] = ph_no[i + 3];
  }
  SIM900A.println(dest_set);
  delay(1000);
  SIM900A.print("Your OTP for the scooter number DL-8SBV-4470 is: ");
  SIM900A.println(otp);
  delay(100);
  SIM900A.println((char)26);
  delay(1000);
}
void challenge() {
  Serial.println("Inside challenge()");
  while (SIM900A.available()) Serial.write(SIM900A.read());
  if (check_otp_input()) {
    LockedPosition(false);
    Serial.println("Lock has been unlocked!");
    unlock_sms(true);
  }
  else Serial.println("LOCK KEY NOT CORRECT!!");
  unlock_sms(false);
}
bool check_otp_input() {
  Serial.println("Inside check_otp_input()");
  //  Serial.println("Waiting for User OTP input");
  int t = 1200;
  position = 0;
  int pressed = 0;
  while (t--) {
    //// get keys from Keypad
    char key = Serial.read();
    if ((key >= '0' && key <= '9') || (key >= 'A' && key <= 'D') || key == '*' || key == '#') {
      //      Serial.print(key);
      pressed++;
      if (key == otp[position]) position++;
    }

    if (position == 6 && pressed == 6) {
      return true;
    }
    if (pressed >= 6) break;
    delay(100);
  }
  return false;
}
void unlock_sms(bool success) {

  SIM900A.println("AT+CMGF=1");
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  char dest_set[21] = "AT+CMGS=\"XXXXXXXXXX\"\r";
  for (int i = 0; i < 10; i++) {
    dest_set[i + 9] = ph_no[i + 3];
  }
  SIM900A.println(dest_set);
  delay(1000);
  if (success)SIM900A.println("Time to ride!");
  else SIM900A.println("You entered incorrect OTP. Please call again.");
  delay(100);
  SIM900A.println((char)26);
  delay(1000);
  while (SIM900A.available()) Serial.write(SIM900A.read());
}

void send_gps() {

  SIM900A.println("AT+CMGF=1");
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  char dest_set[21] = "AT+CMGS=\"8010070210\"\r"; // OWNER NUMBER HARSH
  delay(1000);
  SIM900A.print("Location : ");
  SIM900A.print(location);
  SIM900A.println("Scooter : DL-8SBV-4470");
  delay(100);
  SIM900A.println((char)26);
  delay(1000);
  while (SIM900A.available()) Serial.write(SIM900A.read());

}
void setup() {

  // put your setup code here, to run once:
  Serial.begin(9600);    // Setting the baud rate of Serial Monitor (Arduino)
  //  set_sim_module();
  set_otp_module();
  set_gps_module();
  // pinMode(LED_BUILTIN, OUTPUT);
  //  pinMode(Lock, OUTPUT);
  //  LockedPosition(true);
  Serial.println("READY!!!!");
}
int timer = 0;
void loop() {
  set_gps_module();
  if (first_loop_exec == true) {
    delay(2000);
    GPS.println(F("$PUBX,40,RMC,0,0,0,0*47")); //RMC OFF
    delay(100);
    GPS.println(F("$PUBX,40,VTG,0,0,0,0*5E")); //VTG OFF
    delay(100);
    GPS.println(F("$PUBX,40,GGA,0,0,0,0*5A")); //CGA OFF
    delay(100);
    GPS.println(F("$PUBX,40,GSA,0,0,0,0*4E")); //GSA OFF
    delay(100);
    GPS.println(F("$PUBX,40,GSV,0,0,0,0*59")); //GSV OFF
    delay(100);
    GPS.println(F("$PUBX,40,GLL,0,0,0,0*5C")); //GLL OFF
    delay(1000);
    first_loop_exec = false;
  }
  GPS.println("$PUBX,00*33"); // data polling to the GPS
    delay(100);
  // MANAGES THE CHARACTERS RECEIVED BY GPS
  while (GPS.available()) {
    GPS_info_char = GPS.read();
    if (GPS_info_char == '$') { // start of message
      message_started = true;
      received_char = 0;
    } else if (GPS_info_char == '*') { // end of message
      int commas = 0;
      int loc = 0;
      // PROCESSING ON OUTPUT AND EXTRACTING COORDINATES INTO BEAUTIFUL STRING HERE !!!!
      for (i = 0; i < received_char; i++) {
        //        Serial.write(GPS_info_buffer[i]); // writes the message to the PC once it has been completely received
        if (GPS_info_buffer[i] == ',') commas++;
        if (commas >= 3) gps_loc[loc++] = GPS_info_buffer[i];
        if (commas == 7) break;
      }
      gps_loc[loc] = '\0';
      int j = 0;
      // PROCESSING ON CONVERSION INTO READABLE FORMAT HERE !
      location[j++] = gps_loc[1];
      location[j++] = gps_loc[2];
      location[j++] = '°';
      location[j++] = gps_loc[3];
      location[j++] = gps_loc[4];
      location[j++] = '\'';
      location[j++] = gps_loc[6];
      location[j++] = gps_loc[7];
      location[j++] = '.';
      location[j++] = gps_loc[8];
      location[j++] = gps_loc[9];
      location[j++] = '\"';
      location[j++] = gps_loc[12];
      location[j++] = ' ';
      location[j++] = gps_loc[14];
      location[j++] = gps_loc[15];
      location[j++] = gps_loc[16];
      location[j++] = '°';
      location[j++] = gps_loc[17];
      location[j++] = gps_loc[18];
      location[j++] = '\'';
      location[j++] = gps_loc[20];
      location[j++] = gps_loc[21];
      location[j++] = '.';
      location[j++] = gps_loc[22];
      location[j++] = gps_loc[23];
      location[j++] = gps_loc[26];
            location[j] = '\0';

//      Serial.println();
      message_started = false; // ready for the new message
    } else if (message_started == true) { // the message is already started and I got a new character
      if (received_char <= GPS_INFO_BUFFER_SIZE) { // to avoid buffer overflow
        GPS_info_buffer[received_char] = GPS_info_char;
        received_char++;
      } else { // resets everything (overflow happened)
        message_started = false;
        received_char = 0;
      }
    }
  }
//  set_sim_module();
  delay(100);
  //    if (Serial.read() == '#') LockedPosition(true);
  //    if (SIM900A.available()) { // read output of SIM900A char by char
  //      Serial.println("heyo imma runnin yo!");
  //      get_ph_no();
  //      calc_otp();
  //      send_otp();
  //      challenge();
  //    }
  timer++;
  if (timer >= 10*60) {
    timer = 0;
    Serial.println(location);
    //  send_gps();
  }
}
