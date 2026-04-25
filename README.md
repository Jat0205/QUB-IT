# QUB-IT
QUB-IT: A Quantum Bop It Game <br><br>
**Contributors:** <br>
Jessica Huang, UPitt EE<br>
Thomas Eble, UPitt EPhys<br>
Jannet Trabelsi, UPitt EPhys<br><br>
**Microcontroller:** <br> 
atmega328p <br><br>
**Programming Language:** <br>
C++<br><br>
**Pinout:** <br>
DO 0:Y ROT ENC <br>
DO 1: LED |+>  <br>
DO 2: X ROT ENC <br>
DO 3: LED |0> <br>
DO 4: AUDIO <br>
DO 5: AUDIO <br>
DO 6: LED |-> <br>
DO 7: Z ROT ENC <br>
DO 8: DEBUG LED <br>
DO 9: LED |1> <br>
DO 10: LED |i> <br>
DO 11: LED |-i> <br>
DO 12: <br>
DO 13: <br>
A0: BUTTON: Measure Z <br>
A1: BUTTON: Measure Y <br>
A2: BUTTON: Measure X <br>
A3:PHOTODIODE <br>
A4: LCD <br>
A5: LCD <br><br>

** Steps to reproduce our work:<br>
- Refer to the provided schematic to reproduce the work and test it on a breadboard first.<br>
- Order the PCB included through JLCPCB (we used their design rule check.<br>
- Order parts needed from digikey (BOM is provided).<br>
- Solder PCB.<br>
- 3D print sphere, sphere stand, and remote enclosure (stl files included).<br>
- place the LEDs and photodiode in the sphere such as the LEDs on the same axis correspond to the same basis.<br>
- Connect the wires from the sphere to the wires soldered on the PCB (color code them for easier debugging)<br>
- Add the provided audio files in a micro SD card. <br>
- Attach the SD card to the DF player. <br>
- Connect a usbtiny Program the 6 pin area on the PCB and load the Qubit_Main.ino file into ardiino IDE, burn the bootloader, compile the program, export the compiled library, then program the board. if you have mac, open a terminal at folder then paste the following: avrdude -c usbtiny -p m328p -P usb -e -U flash:w:/Users/USERNAME/Documents/Arduino/Qubit_Main/build/arduino.avr.uno/Qubit_Main.ino.hex (please replace USER by your username).<br>
- Connect a 9 V battery and use the switch to turn on the toy. (the reset button can also be used to reset the game). <br>
- Enjoy Qub-it!<br>

Please note: there are multiple ino files to test individual functions and debug your PCB, such as LCD.ino, MEASURE_IT.ino, ROT_ENC.ino, STATE_TRANSITION.ino, audio.ino, and photodiode.ino. Each file tests an individual function.<br>
