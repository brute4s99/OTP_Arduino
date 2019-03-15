/*THIS TUTORIAL USED GSM SIM900A S2-1040V-Z1K0B

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
// pin reserved by Lock
int Lock = 13;
// pins reserved by SIM900A
SoftwareSerial SIM900A(10, 11);

char keys[4][4] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, 4, 4 );

int position = 0;
char ph_no[14] = {'\0'};
char check;
char otp[7] = "XXXXXX\0";
int number;
void LockedPosition(int locked)
{
if (locked) digitalWrite(Lock, LOW);
else digitalWrite(Lock, HIGH);
}
void set_sim_module() {
  SIM900A.begin(9600);   // Setting the baud rate of GSM Module
 // command one : AT+COPS?
  SIM900A.println("AT+COPS?");
  delay(1000);
 // SIM900A speaks
  while ((SIM900A.available() > 0)) SIM900A.read();
  delay(100);
  
  // command two : AT+CLCC=1
  SIM900A.println("AT+CLCC=1");
  delay(1000);
  // SIM900A speaks
  while ((SIM900A.available() > 0)) SIM900A.read();
  delay(100);

}

void set_otp_module() {
  randomSeed(analogRead(0)); // make sure this pin is UNCONNECTED AND ANALOG
}
// gets the phone number from incoming call as "+91XXXXXXXXXX"
void get_ph_no() {
  check = SIM900A.read(); // store each character in a char named `check`
  delay(50);
  if (check == '\"') {
    for (int i = 0; i < 13; i++) {
      ph_no[i] = SIM900A.read();
      delay(50);
    }
    ph_no[13] = '\0';
    // once you got the phone number, hang the call and flush the output from SIM900A
    SIM900A.println("ATH");
    delay(1000);
    while (SIM900A.available()) SIM900A.read();
    calc_otp();
  }
}
// calculates a purely random OTP every time
void calc_otp() {
//  Serial.println("Calculated random OTP");
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
  
  otp[n]='\0';
//  Serial.println("Sending you the OTP in 10...");
  delay(1000);
  send_otp();
}
// Sends an OTP to the caller
void send_otp() {
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
  while (SIM900A.available()) SIM900A.read();
  if (check_otp_input()) {
      LockedPosition(false);
    send_success();  
    }
    else send_failure();
}
// Sends a failure message to the caller
void send_failure() {
  SIM900A.println("AT+CMGF=1");
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  char dest_set[21] = "AT+CMGS=\"XXXXXXXXXX\"\r";
  for (int i = 0; i < 10; i++) {
    dest_set[i + 9] = ph_no[i + 3];
  }
  SIM900A.println(dest_set);
  delay(1000);
  SIM900A.println("You entered incorrect OTP. Please call again.");
  delay(100);
  SIM900A.println((char)26);
  delay(1000);
  while (SIM900A.available()) SIM900A.read();
}
// Sends a success message to the caller
void send_success() {
  SIM900A.println("AT+CMGF=1");
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  char dest_set[21] = "AT+CMGS=\"XXXXXXXXXX\"\r";
  for (int i = 0; i < 10; i++) {
    dest_set[i + 9] = ph_no[i + 3];
  }
  SIM900A.println(dest_set);
  delay(1000);
  SIM900A.println("Time to ride!");
  delay(100);
  SIM900A.println((char)26);
  delay(1000);
  while (SIM900A.available()) SIM900A.read();
}
bool check_otp_input() {
//  Serial.println("Waiting for User OTP input");
  int t = 1200;
  position = 0;
  int pressed = 0;
  while (t--) {
    //// get keys from Keypad
    char key = keypad.getKey();
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
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);    // Setting the baud rate of Serial Monitor (Arduino)
  set_sim_module();
  set_otp_module();
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(Lock, OUTPUT);
  LockedPosition(true);
//  Serial.println("READY!!!!");
  // now we are ready to loop on SIM900A, waiting for call continuously
}

void loop() {
//  Serial.write(SIM900A.read());
  if (keypad.getKey() == '#') LockedPosition(true);
  if (SIM900A.available()) { // read output of SIM900A char by char
//    Serial.println("I ran!");
    get_ph_no();
  }
}
