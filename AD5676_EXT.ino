#include <SPI.h>

const int CS = A4;
const int MAX_V = 5;//Change if 2.5 is set

void setup() {
  Serial.begin(38400);

  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);

  SPI.begin();
  SPI.beginTransaction(SPISettings(125000, MSBFIRST, SPI_MODE1));
}

void loop() {

  digitalWrite(CS, HIGH);

  String req;

  if (Serial.available() > 0) {  //Read command from buffer

    while (Serial.available()) {
      char c = Serial.read();  //gets one byte from serial buffer
      req += c; //makes the String readString
      delay(2);  //slow looping to allow buffer to fill with next character
    }

    if (req.indexOf("Set one") >= 0) {
      unsigned long selectChannel = getUserCh();
      float setTo = getUserReq(0);
      byteSwapNewDacValue(convertValue(setTo), selectChannel);
    }

    else if (req.indexOf("Set all") >= 0) {
      uint32_t tempbuf [] = {0x00000000, 0x00B00000}; // [0] is generic mask [1] is mask for select all
      float setTo = getUserReq(0);

      tempbuf[0] = (convertValue(setTo) | tempbuf[0]) << 16; //Enter Value in Mask and shift in position First byte
      tempbuf[0] = (convertValue(setTo) | tempbuf[0]) >> 8 << 8; //Enter Value in Mask and shift in position First byte
      tempbuf[0] = tempbuf[1] >> 16 | tempbuf[0] << 8>>8;

      executeSPI((uint32_t*)tempbuf); // Requests read for selected channel
    }

    else if (req.indexOf("Read back") >= 0) {
      unsigned long selectChannel = getUserCh();
      float currentvalue = ReadBack(selectChannel);
      Serial.println(currentvalue);
    }

    else if (req.indexOf("Incr one") >= 0) {
      unsigned long selectChannel = getUserCh();
      float current = ReadBack(selectChannel);
      Serial.print("Current value is ");
      Serial.println(current);
      /* **************************/

      float setTo = getUserReq(1);
      unsigned short newValue = convertValue(current + setTo); // Creates new value
      byteSwapNewDacValue(newValue, selectChannel);

    }
    else if (req.indexOf("Decr one") >= 0) {
      unsigned long selectChannel = getUserCh();
      float current = ReadBack(selectChannel);
      Serial.print("Current value is ");
      Serial.println(current);
      /* **************************/

      float setTo = getUserReq(2);
      unsigned short newValue = convertValue(current - setTo); // Creates new value
      byteSwapNewDacValue(newValue, selectChannel);
    }
    else
    {
      Serial.println("Invalid command");
    }
    Serial.println("Execution done");
  }

}

void byteSwapNewDacValue (unsigned short newValue, unsigned long selectChannel) // Byte swap is required to use the spi function of arduino with 3 byte
{
  uint32_t tempbuf [] = {0x00000000, 0x00300000}; // [0] is generic mask [1] is mask for select channel and value
  tempbuf[1] = tempbuf[1] | selectChannel << 16;
  tempbuf[0] = (newValue | tempbuf[0]) << 16; //Enter Value in Mask and shift in position First byte.
  tempbuf[0] = (newValue | tempbuf[0]) >> 8 << 8; //Enter Value in Mask and shift for clearing bytes
  tempbuf[0] = tempbuf[1] >> 16 | tempbuf[0] << 8>>8;

  executeSPI((uint32_t*)tempbuf); // Requests read for selected channel
}

float ReadBack (unsigned long selectChannel) // Byte swap is required to use the spi function of arduino with 3 byte
{
  uint32_t tempbuf [] = {0x00000000, 0x900000}; //[0] is generic mask [1] is mask for read back
  tempbuf[1] = tempbuf[1] | selectChannel << 16;
  tempbuf[0] = tempbuf[1] >> 16 | tempbuf[0] << 8>>8;

  executeSPI((uint32_t*)tempbuf); // Requests read for selected channel

  unsigned short received = executeReadCmSPI(tempbuf, 0); // Executes read and uses it

  float currentvalue = reverseConvertValue(received);

  return currentvalue;
}

unsigned long getUserCh()
{
  Serial.println("Select channel 0-7");
  while (Serial.available() == 0);
  unsigned long selCh = Serial.parseInt(SKIP_ALL, '\n');
  Serial.println(selCh);
  return selCh;
}

float getUserReq(int CommandSel)
{
  String commands[3] = { "Enter set value 0-MAX", "Enter increment value", "Enter decrement value"};
  Serial.println(commands[CommandSel]);
  while (Serial.available() == 0);
  float setTo = Serial.parseFloat(SKIP_ALL, '\n');
  Serial.println(setTo);
  return setTo;
}

unsigned short convertValue (float value)
{
  unsigned short MAX_OUTP = 0xFFFF;

  unsigned short NEW_VALUE = (MAX_OUTP / (MAX_V / value));
  return NEW_VALUE;
}

float reverseConvertValue (unsigned long value)
{
  unsigned long MAX_OUTP = 0xFFFF;
  unsigned long MAX_V_L = MAX_V;

  float NEW_VALUE = ((float)(value * MAX_V) / (float)MAX_OUTP);
  return NEW_VALUE;
}

unsigned short executeReadCmSPI (uint32_t buf[], int elementToTransf)
{
  unsigned short receivedVal16 = 0;

  digitalWrite(CS, LOW);
  delayMicroseconds(10);
  SPI.transfer(0x00); //0x000000 is the command to execute read.
  receivedVal16 = SPI.transfer16(0x0000);
  digitalWrite(CS, HIGH);

  return receivedVal16;
}

void executeSPI (uint32_t *buf)
{
  digitalWrite(CS, LOW);
  delayMicroseconds(10);
  SPI.transfer(buf, 3);
  digitalWrite(CS, HIGH);
}
