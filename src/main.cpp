/* DuinoAccess-MainCode
   by: Dynais
   date: 2020-08-14
   messages: 修改下方参数调整区域内的宏定义达到基本的参数调整
*/

//头文件
#include "DAinclude.h"
#include "DAstate.h"

//参数调整
#define PASSWORD "44687E64A8E2557DF7CFA83C989A4627" //在这里填上你的密码
#define DETECT_PAUSE 2000                           //连续检测间隔 (ms)(只作用于身份验证模块)
#define ARDUINO_BAUD 57600                          //Arduino USB串口通信波特率
#define DASTATE 2                                   //调整参数以更改整体功能

extern UINT8 sigState;  //信号灯标志位
extern CRGB signal_leds[];  
extern UINT8 fpState; //指纹检测标志位
UINT8 prevFpState;  //前一次指纹检测标志位
UINT8 retval; //返回信息标志位
UINT8 prevRetval; //前一次返回信息标志位

//指示灯光中断处理
void process()
{
  //灯光处理
  updateSignal(sigState);
}

//串口输出指纹模块信息
void getFpInfo(){
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
}

//初始化
static void initialize()
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
    
//******************************************************************//
//  usage：用于测试硬件的完好性
//  signal：检测到有手指时，亮绿灯，没有手指时不亮灯
//  
//******************************************************************//
  case DA_STATE_PRESSTEST: 
    //检测手指
    retval = fpmDetectFinger(NULL);
    //通过对比前一次指纹探测状态是否跳变来控制灯光标志位的变化
    if (prevRetval == HZERR_NO_FINGER && retval == HZERR_SUCCESS)
      sigState = SIG_STATE_SUCCESS;
    else if (retval == HZERR_NO_FINGER && prevRetval == HZERR_SUCCESS)
      sigState = SIG_STATE_SLEEP;
    //结束后重新复制
    prevRetval = retval;
    delay(100);
    break;

//******************************************************************//
//  usage：用于写入指纹到指纹模块
//  signal：等待黄灯亮起，示意等待手指接触，接触后亮蓝灯表示检测到手指
//          如果手指长时间未移开，亮橙色灯示意，成功完成一次录入亮绿灯，重复
//******************************************************************//

  case DA_STATE_ENROLL: 
    Serial.println("--- start enroll finger ---");
    fpState = FP_STATE_START;
    sigState = SIG_STATE_SLEEP;
    //设置指纹过长防止提醒时间
    UINT32 warningPressTime;
    delay(2000);
    //进入注册循环
    while (1)
    {
      prevFpState = fpState; //保存前一次状态
      Serial.println("in fpenroll");
      fpEnroll();

      // 根据指纹标志位来判断当前状态
      switch (fpState)
      {
      case FP_STATE_WAIT_FINGER_OFF2ENROLL:
      case FP_STATE_WAIT_FINGER_OFF2OK:
      case FP_STATE_WAIT_FINGER_OFF2ERR:
        if (FP_STATE_ENROLL_PROC == prevFpState)
        {
          Serial.println("--- please release your finger ---");
          if (prevFpState != fpState)
            //状态转换时记录一次时间，用来判断手指过长时间放置
            warningPressTime = getEndTime(3000);
          sigState = SIG_STATE_PROCESSING;
        }
        if (warningPressTime < getCurTime() && sigState != SIG_STATE_WARNING)
          sigState = SIG_STATE_WARNING;
        break;
      case FP_STATE_END_OK:
        Serial.println("--- enroll sucess ---");
        sigState = SIG_STATE_SUCCESS;
        delay(1000);
        break;
      case FP_STATE_END_ERR:
        Serial.println("--- enroll failed ---");
        sigState = SIG_STATE_FAILED;
        delay(1000);
        break;
      default:
        if (prevFpState != fpState)
          sigState = SIG_STATE_WAITTING;
        break;
      }
      //一次指纹录入结束
      if (FP_STATE_DEFAULT == fpState)
      {
        if (prevFpState != fpState)
          sigState = SIG_STATE_WAITTING;
        break;
      }
      //查看录入状态
      // Serial.print(getCurTime());
      // Serial.print(" ");
      // Serial.print(warningPressTime);
      // Serial.print(" ");
      // Serial.println(sigState);
    }
    break;

//******************************************************************//
//  usage：身份验证，用于最后使用，需要先录入指纹
//  signal：不适用时呈休眠状态不亮灯，检测到手指并且验证成功时亮绿灯，否则
//          亮红灯
//******************************************************************//

  case DA_STATE_DETECT: 
    Keyboard.begin();
    UINT32 cdEndTime = getCurTime();
    while (1)
    {
      retval = fpIdentify(); //身份验证
      //验证成功
      if (HZERR_SUCCESS == retval)
      {
        sigState = SIG_STATE_SUCCESS;
        //两次成功的验证之间不得小于指定时间
        if(cdEndTime < getCurTime()){
          Keyboard.println(PASSWORD);
          Serial.println("--- identify successed ---");
          cdEndTime = getEndTime(DETECT_PAUSE);
        }
      }
      //验证失败
      else if (HZERR_NO_FINGER != retval)
      {
        sigState = SIG_STATE_FAILED;
        Serial.println("--- identify failed ---");
      }
      //休眠
      else
      {
        sigState = SIG_STATE_SLEEP;
      }
    }
    Keyboard.end();
    break;

//******************************************************************//
//  usage：查看硬件信息,使用时需要配合指纹模块摁手指才能输出到串口
//  signal：无
//******************************************************************//

  case DA_STATE_INFO: 
    retval = fpmDetectFinger(NULL);
    if (retval == HZERR_SUCCESS)
    {
      sigState = SIG_STATE_SUCCESS;
      getFpInfo();
      delay(2000);
    }
    sigState = SIG_STATE_SLEEP;
    break;

//******************************************************************//
//  usage：清除指纹数据库
//  signal：无
//******************************************************************//

  case DA_STATE_CLEAR: //清除所有指纹信息功能
    retval = fpDeleteAll();
    Serial.print("尝试清除所有指纹信息: ");
    Serial.println(retval);
    while (1)
      ;
    break;
    
//******************************************************************//
//  usage：无
//  signal：无
//******************************************************************//
  default:
    break;
  }
}
