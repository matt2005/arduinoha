**Browse the source-code!!!**

With this project i'm aiming to create an intelligent Home Automation solution around the Arduino platform for my own home. By using an 433mhz radioreceiver and transmitter i'm able to receive and transmit RF signals to communicate with RF-based devices. The Arduino is the node which does the decoding and encoding of the signals. The Arduino also is the orchestrator which does all the scheduling and has the controlling-logic.

Although i'm talking about the Arduino-platform, i'm actually using the Sanguino myself because this one has much more memory. At this moment the sourcecode compiles to around 23kb of code. The Sanguino and Arduino are much alike, but use different processors. The Arduino i had at first used the Atmega168-cpu. The Sanguino uses the Atmega644-cpu.

There is quite a lot of code in the solution. The solution-code can be broken down into three parts: the orchestrator-part (.pde), the protocol-de/en-coders (**.cpp/**.h) and support classes (**.cpp/**.h).

Because of the limited resources on the platform i've had to implement many efficient techniques to not overload the processor which would interfere with the time-critical sending and receive-process.

To control the timing of the pulse-lengths which are sent being sent is done by using the Timer-overflow-interrupt.

Determining the duration of pulses which are being received is done by using the Pin-change-interrupt and calculating the number of ticks which have passed.

In the foreground the received pulses are being decoded. Also the scheduling of commands which needs to be sent is done in the foreground.

The flow-path of RF-pulses through code:
RF Receiver->Timing pulses->Store pulse in buffer->Fetch pulse from buffer->Decode pulse->Trigger event->Schedule command->Queue command->Sending Command->RF Transmitter

The following protocols are/will be supported:
  * X10 (Recognizing: OK, Deconding: Not complete, Encoding: Not implemented)
  * Klik aan Klik uit - Klik on Klik off (Recognizing: OK, Decoding OK, Encoding OK)
  * Skytronic Homelink (Recognizing: OK, Deoding: OK, Encoding: Not implemented)
  * McVoice (Recognizing: OK, Decoding OK) http://export.skytronic.com/product/product.php?s=350.125
  * Elro AB600 (Status: Decoding OK)
  * La Crosse TX3 / TX4 (Status: Decoding OK)
  * FranElec (Recognizing: OK, Status: Decoding OK)
http://export.skytronic.com/product/product.php?s=499.907
  * Ranex (Status: Decoding OK, Encoding OK)
  * Elro AB600 (Status: Decoding OK)
  * Skytronic (Status: Decoding OK)
http://export.skytronic.com/product/product.php?s=350.221
  * ELV (Status: Decoding OK)
  * Siemens KeyFob (Status: Decoding OK)
  * La Crosse WS-2300 (Recognizing: OK)
http://www.lacrossetechnology.fr/en/WS2300.html

The solution isn't finished yet. The current state is that most protocols will be received and decoded. Also some protocols have encoding implemented. These protocols can also send commands via RF.

The protocol code is implemented in a Object Oriented fashion (classes). The reason for this: "less code==less memory usage".

My goal is to achieve the following:
  * Have lights automatically turn on and off depending on the motion-sensors around my house.
  * Have my home ventilation turn on and off depending on the temperature and humidity in my bathroom.
  * Be able to control many devices with my URC MX-3000 remote.