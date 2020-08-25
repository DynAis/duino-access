/* DuinoAccess-MainCode
   by: Dynais
   date: 2020-08-14
   messages: 修改下方参数调整区域内的宏定义达到基本的参数调整
*/

//头文件
#include "DAinclude.h"
#include "DAstate.h"

//参数调整
#define PASSWORD "44687E64A8E2557DF7CFA83C989A4627" //这里填上你的密码
#define DETECT_PAUSE 1600                           //连续检测间隔 in ms
#define ARDUINO_BAUD 57600                          //Arduino usb串口通信波特率
#define DASTATE 0                                   //模块实现功能调整

extern UINT8 fpState;
extern UINT8 sigState;
extern CRGB signal_leds[];

UINT8 prevFpState;
UINT8 retval;
UINT8 prevRetval;

//定时中断内函数，处理指示灯光
void process()
{
  //灯光处理
  updateSignal(sigState);
}

//初始化函数
static void initialize(void)
{
  //引脚初始化
  pinMode(PIN_FP_RST, OUTPUT);

  //串口初始化
  Serial.begin(ARDUINO_BAUD); //This pipes to the serial monitor
  Serial1.begin(57600);       //This is the UART, pipes to sensors attached to board
  delay(100);

  //指纹模块初始化
  Serial.println("in Init");
  while (fpInit())
  {
    debug();
  }

  //唤醒指纹模块
  Serial.println("in WakeUP");
  while (fpWakeup())
  {
    debug();
  }

  //WS2812灯光初始化
  sigInit();

  //定时器中断初始化
  MsTimer2::set(16, process);
  //开始计时
  MsTimer2::start();
}

void setup()
{
  initialize();
}

void loop()
{

  switch (DASTATE)
  {
  case DA_STATE_PRESSTEST: //手指按压测试功能
    //检测手指
    retval = fpmDetectFinger(NULL);
    if(prevRetval==HZERR_NO_FINGER && retval == HZERR_SUCCESS)
      sigState = SIG_STATE_SUCCESS;
    else if (retval==HZERR_NO_FINGER && prevRetval == HZERR_SUCCESS)
      sigState = SIG_STATE_SLEEP;
    // Serial.println(retval);
    prevRetval = retval;
    delay(100);
    break;

  case DA_STATE_ENROLL: // 指纹注册功能
    Serial.println("--- start enroll finger ---");
    fpState = FP_STATE_START;
    while (1)
    {
      prevFpState = fpState; //保存前一次状态
      //2
      Serial.println("in fpenroll");
      delay(100);
      fpEnroll();
      // excute the INDICATION according to the fpState
      switch (fpState)
      {
      case FP_STATE_WAIT_FINGER_OFF2ENROLL:
      case FP_STATE_WAIT_FINGER_OFF2OK:
      case FP_STATE_WAIT_FINGER_OFF2ERR:
        if (FP_STATE_ENROLL_PROC == prevFpState)
        {
          Serial.println("--- please release your finger ---");
          waitting();
        }
        break;
      case FP_STATE_END_OK:
        Serial.println("--- enroll sucess ---");
        success();
        break;
      case FP_STATE_END_ERR:
        Serial.println("--- enroll failed ---");
        debug();
        break;
      }
      // one finger enrollment is finished
      if (FP_STATE_DEFAULT == fpState)
      {
        break;
      }
    }
    break;

  case DA_STATE_DETECT: // 验证指纹功能(最后留这一个)
    Keyboard.begin();
    while (1)
    {
      retval = fpIdentify();
      if (HZERR_SUCCESS == retval)
      {
        Keyboard.println(PASSWORD);
        success();
        Serial.println("--- identify successed ---");
        delay(DETECT_PAUSE);
      }

      else if (HZERR_NO_FINGER != retval)
      {
        debug();
        Serial.println("--- identify failed ---");
        delay(DETECT_PAUSE);
      }
    }
    Keyboard.end();
    break;

  case DA_STATE_INFO: //摁下指纹,输出硬件信息功能
    retval = fpmDetectFinger(NULL);
    if (retval == HZERR_SUCCESS)
    {
      success();
      ST_DEV_INFO_HOZO info;
      retval = fpmGetDeviceInfo(&info);
      Serial.print("当前波特率: ");
      Serial.println(info.baudrate);
      Serial.print("当前已注册指纹量: ");
      Serial.println(info.enrollCount);
      Serial.print("总指纹容纳量: ");
      Serial.println(info.maxCount);
      Serial.print("当前设备签名信息: ");
      Serial.println(info.signature);
      Serial.print("指纹采样数量: ");
      Serial.println(info.sampleSize);
      Serial.print("指纹识别门槛等级: ");
      Serial.println(info.matchingThreshold);
      Serial.print("指纹唯一性限制: ");
      Serial.println(info.uniqueConstrain);
      Serial.print("指纹严格模式: ");
      Serial.println(info.strictEnrollment);
      delay(2000);
    }

    break;

  case DA_STATE_CLEAR: //清除所有指纹信息功能
    retval = fpDeleteAll();
    Serial.print("尝试清除所有指纹信息: ");
    Serial.println(retval);
    while (1)
      ;
    break;

  default:
    break;
  }
}
