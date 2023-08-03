Was developed on Arduino micro

Send following commands from Serial Power (baud 38400):
  "Set one" to set one channel
  "Set all" to set all channels
  "Read back" to read back from one channel
  "Incr one" to increase value of one channel
  "Decr one" to derease value of one channel

CS = Pin A4
CS/MOSI/SCKL all shifted from 5V to 3V3
It is assumed that max value 0xFFFF = 5V
It is assumed that Logic Voltage = 3V3
Powered 6V 
