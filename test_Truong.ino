#include <SoftwareSerial.h> //header file of software serial port
SoftwareSerial Serial1(2, 3); //define software serial port name as Serial1 and define pin2 as RX and pin3 as TX

int dist; //actual distance measurements of LiDAR
//int strength; //signal strength of LiDAR
//float temprature;
int check; //save check value
int ch;
int i;
int uart[9]; //save data measured by LiDAR
int rtu[9];
unsigned char res[9];
const int HEADER = 0x59; //frame header of data package
const int ADDR = 0x02;
const unsigned int CRC16_POLYNOMIAL = 0x8005;
const unsigned int CRC16_INIT = 0xFFFF;
unsigned char crc[2] = "";

unsigned int crc16(char *crcData, unsigned int dataLen)
{
  unsigned char i;
  unsigned int iByte;
  unsigned long crc = CRC16_INIT;

  if (dataLen == 0)
    return (~crc);
  do
  {
    for (i = 0, iByte = ((unsigned int)0xff & *crcData++) << 8;
         i < 8;
         i++, iByte <<= 1)
    {
      if ((crc & 0x8000) ^ (iByte & 0x8000))
        crc = (crc << 1) ^ CRC16_POLYNOMIAL;
      else
        crc <<= 1;
    }
  } while (--dataLen);

  return (crc & 0xFFFF);
}

unsigned char reflect_byte(unsigned char *byte, unsigned char *byte_ref, unsigned short index)
{
  unsigned short k, j;
  unsigned char temp;
  for (j = 0; j < index; j++)
  {
    for (k = 0; k < 8; k++)
    {
      temp = byte[j] >> k;
      byte_ref[index - 1 - j] += (temp & 0x01) << (7 - k);
    }
  }
}

void Create_CRC(unsigned char *input, unsigned short num_byte, unsigned char *output_ref)
{
  /*******Reflect Frame******/
  unsigned char input_ref[5] = "";
  unsigned char txt[5] = "";
  unsigned int crc = 0;
  unsigned char output[2] = "";
  unsigned short i = 0;

  reflect_byte(input, input_ref, num_byte);
  /*****Tinh CRC*****/
  for (i = 0; i < num_byte; i++)
  {
    txt[i] = input_ref[num_byte - 1 - i];
  }

  crc = crc16(txt, num_byte);
  output[0] = crc & 0xff;
  output[1] = (crc >> 8) & 0xff;

  /**********Reflect CRC**********/
  reflect_byte(output, output_ref, 2);
}
//================== ham xoa serial3 ==================
void clear_buffer_Serial() {
  while (Serial.available() > 0) {
    char t = Serial.read();
  }
}

//================== ham xoa serial3 ==================
void clear_buffer_Serial1() {
  while (Serial1.available() > 0) {
    char k = Serial.read();
  }
}
void setup() {
  Serial.begin(9600); //set bit rate of serial port connecting Arduino with computer
  Serial1.begin(9600); //set bit rate of serial port connecting LiDAR with Arduino
  delay(100);
  Serial1.write(0x5A);
  Serial1.write(0x06);
  Serial1.write(0x03);
  Serial1.write(0x01);
  Serial1.write('\0');
  Serial1.write(0x64);
  delay(100);
  Serial1.write(0x5A);
  Serial1.write(0x08);
  Serial1.write(0x06);
  Serial1.write(0x80);
  Serial1.write(0x25);
  Serial1.write('\0');
  Serial1.write('\0');
  Serial1.write(0x0D);

  delay(100);
  Serial1.write('\0');
  Serial1.write(0x64);
  delay(100);
  Serial1.write(0x5A04116F);
  delay(100);
}
void loop() {
  if (Serial1.available()) { //check if serial port has data input
    delay (10) ;
    if (Serial1.read() == HEADER) { //assess data package frame header 0x59
      uart[0] = HEADER;
      if (Serial1.read() == HEADER) { //assess data package frame header 0x59
        uart[1] = HEADER;
        for (i = 2; i < 9; i++) { //save data in array
          uart[i] = Serial1.read();
        }
        check = uart[0] + uart[1] + uart[2] + uart[3] + uart[4] + uart[5] + uart[6] + uart[7];
        if (uart[8] == (check & 0xff)) { //verify the received data as per protocol
          dist = uart[2] + uart[3] * 256; //calculate distance value
        }
      }
    }
    clear_buffer_Serial1()  ;
  }

  if (Serial.available()) {
    delay (10) ;
    rtu[0] = Serial.read();
    if (rtu[0] == 0x02) {
      for (i = 0; i < 8; i++) {
        rtu[i] = Serial.read();
        delay (1) ;
      }
      res[0] = ADDR;
      res[1] = 0x03;
      res[2] = 0x02;
      res[3] = 0x00;
      res[4] = 0x00;
      res[5] = uart[3];
      res[6] = uart[2];
      Create_CRC(res, 5, crc);
      res[7] = crc[0];
      res[8] = crc[1];
      for (i = 0; i < 9; i++) {
        Serial.write(res[i]);
      }
    }
    clear_buffer_Serial()     ;
  }
}
