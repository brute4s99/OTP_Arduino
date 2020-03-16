# LOCKATOR
An arduino based OTP lock system with live tracking

## The H/W:-
- SIM900A GSM Module
- 4x4 Keypad
- Servo Lock with external power source
- Arduino UNO
- NEO-6M GPS Module

# Note
SIM90A Does Not Support voLTE - you can, however use a 4G SIM over 2G connection rate.
## The What
So you connected everything to the correct pins (documented in code), inserted a SIM in GSM module and now you want to know how it works.

0. power it up and wait for about 1 minute so that everthing is set up nicely.
   > GSM (and GPS) take the most chunk of time in boot.

1. Call the phone number of the SIM you inserted in the GSM module.
    > Expected: The call gets hung up in a short while (~2 seconds at max)
2. You will receive an OTP on the phone from the lock system.
    > Now you will have ~2 minutes to enter the OTP in the keypad, otherwise the OTP will expire.
3. Enter that OTP in the lock system through the keypad.
    > Just type the 6 chars in, no Submit button.

<p align='center'><b>***To lock again, press <u>#</u> on the keypad.***</b></p>

### <b><u>Extended Instructions [For [OTPLockWithGPS.ino](OTPLockWithGPS.ino)]</u></b>

4. Now, expect GPS Coordinates on your phone from the lock system at regular intervals.
   > Locking the system by pressing # will stop GPS tracking too.


## The Why
Tinkered with some hardware from folks at [ILUGD](https://ilugd.github.io)! 

you may find me on IRC Freenode : `brute4s99`

or just mail me at `piyush aggarwal 002 at gmail dot com`
