<p align="left"><h2>Railway Crossing Simulation<h2><br>Overview<br>This project is a multi-threaded C application designed to run on the BeagleBone Black. It simulates a railway crossing system using pushbuttons, LEDs, a servo motor, and a piezo buzzer. The application models the behavior of a railway crossing guard, signal lights, and an alarm system to handle trains approaching from either direction, including handling collision scenarios.<br><br>Features<br>Train Detection: Utilizes pushbuttons to simulate sensors for detecting trains approaching or leaving the crossing from both directions.<br>Crossing Guard: A servo motor with an attached arm simulates the crossing guard, which lowers when a train approaches and raises when the train clears the crossing.<br>Signal Lights: Two red LEDs simulate the crossing signal, blinking in an alternating pattern as a train approaches and ceasing to blink when the train clears the crossing.<br>Collision Detection: In the event of a possible collision scenario (trains approaching from both directions), the system ensures the crossing guard is lowered, both LEDs flash simultaneously in a rapid pattern, and an alarm is triggered using a piezo buzzer.<br>Hardware Requirements<br>BeagleBone Black<br>2 Pushbuttons (train sensors)<br>2 Red LEDs (crossing signal)<br>1 Servo Motor (crossing guard)<br>1 Piezo Buzzer (alarm)<br>Software Requirements<br>Linux operating system running on the BeagleBone Black<br>GCC compiler for building the C application<br>Access to GPIO and PWM controls on the BeagleBone Black<br>Configuration<br>The GPIO pins used for the pushbuttons, LEDs, servo motor, and piezo buzzer are configurable and the user is prompted to enter the pin numbers when the application starts.<br><br>Testing<br>The code has been tested using an emulator to simulate the BeagleBone Black environment. During initialization, the application prints the utsname information for verification.<br><br>Usage<br>To run the simulation, compile the C code with the -pthread flag to enable multi-threading, and execute the resulting binary. Follow the prompts to configure the GPIO pins, and then interact with the pushbuttons to simulate trains approaching and leaving the crossing.</p>

###

<p align="left">Technologies used:</p>

###

<div align="left">
  <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/c/c-original.svg" height="40" alt="c logo"  />
  <img width="12" />
  <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/linux/linux-original.svg" height="40" alt="linux logo"  />
</div>

###

<p align="left">Here's my Linkedin if you want to connect !</p>

###

<div align="left">
  <a href="https://www.linkedin.com/in/soham-patil07/" target="_blank">
    <img src="https://raw.githubusercontent.com/maurodesouza/profile-readme-generator/master/src/assets/icons/social/linkedin/default.svg" width="52" height="40" alt="linkedin logo"  />
  </a>
</div>

###
 
