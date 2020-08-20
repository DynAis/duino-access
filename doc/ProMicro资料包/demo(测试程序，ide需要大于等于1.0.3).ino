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
  //串口0 只能和pc通讯（只限于pc交互），发Hello world 给pc机
 Serial.println("Hello world");  // Print "Hello World" to the Serial Monitor
 
 //发Hello给tx0，适合链接后端扩展板等（等价于常规单片机到单片机或者模块的串口通讯）
 Serial1.println("Hello!");  // Print "Hello!" over hardware UART

 //2个灯都亮起来
 digitalWrite(RXLED, HIGH);   // set the LED on
 TXLED1; //TX LED is not tied to a normally controlled pin  
 //新出的这种写法。TXLED1 or TXLED0 已经在lib内定义了所以才能这样写，RXLED没有在lib内定义还是传统写法！
 
 
 delay(1000);              // wait for a second
 
 //2个灯都熄灭
 //digitalWrite(RXLED, LOW);    // set the LED off
 //TXLED0;
 delay(1000);              // wait for a second
}
