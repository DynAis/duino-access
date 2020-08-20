/* Pro Micro Test Code
   by: Nathan Seidle
   modified by: Jim Lindblom
   SparkFun Electronics
   date: January 20, 2012
   license: Public Domain - please use this code however you'd like.
   It's provided as a learning tool.
   
   This code is provided to show how to control the SparkFun
   ProMicro's TX and RX LEDs within a sketch. It also serves
   to explain the difference between Serial.print() and
   Serial1.print().
*/
int RXLED = 17;  // The RX LED has a defined Arduino pin
// The TX LED was not so lucky, we'll need to use pre-defined
// macros (TXLED1, TXLED0) to control that.

void setup()
{
 pinMode(RXLED, OUTPUT);  // Set RX LED as an output
 // TX LED is set as an output behind the scenes

 Serial.begin(9600); //This pipes to the serial monitor
 Serial1.begin(9600); //This is the UART, pipes to sensors attached to board
}

void loop()
{
  //����0 ֻ�ܺ�pcͨѶ��ֻ����pc����������Hello world ��pc��
 Serial.println("Hello world");  // Print "Hello World" to the Serial Monitor
 
 //��Hello��tx0���ʺ����Ӻ����չ��ȣ��ȼ��ڳ��浥Ƭ������Ƭ������ģ��Ĵ���ͨѶ��
 Serial1.println("Hello!");  // Print "Hello!" over hardware UART

 //2���ƶ�������
 digitalWrite(RXLED, HIGH);   // set the LED on
 TXLED1; //TX LED is not tied to a normally controlled pin  
 //�³�������д����TXLED1 or TXLED0 �Ѿ���lib�ڶ��������Բ�������д��RXLEDû����lib�ڶ��廹�Ǵ�ͳд����
 
 
 delay(1000);              // wait for a second
 
 //2���ƶ�Ϩ��
 //digitalWrite(RXLED, LOW);    // set the LED off
 //TXLED0;
 delay(1000);              // wait for a second
}
