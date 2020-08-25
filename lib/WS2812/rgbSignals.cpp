#include "Arduino.h"
#include "rgbSignals.h"
#include "ledPinDef.h"
#include "frameMath.h"

CRGB signal_leds[NUM_LEDS];
// UINT8 brightness;
UINT8 frame = 1;
const UINT8 MAX_FRAME = 90; //1.5s
UINT8 sigState = SIG_STATE_SLEEP;
UINT8 prevSigState = sigState;
// bool inSleep = true;
// bool newAnimation = true;

void sigInit(){
  FastLED.addLeds<WS2812, PIN_LED_DATA, GRB>(signal_leds, NUM_LEDS);
  frame = 1;
  sigState = SIG_STATE_SLEEP;
  prevSigState = sigState;
  // newAnimation = true;
}

UINT8 updateSignal(UINT8 sigState){
  //当状态发生转变时重置动画帧
  if(prevSigState!=sigState)
    frame = 1;
  //保持睡眠时不进行读写降低压力
  if(prevSigState==SIG_STATE_SLEEP && sigState==SIG_STATE_SLEEP)
    return sigState;
  //保存上一帧状态以供判断
  prevSigState = sigState;
  //动画播放完毕则进入睡眠
  if(frame>=MAX_FRAME)
    sigState = SIG_STATE_SLEEP;
  switch (sigState)
  {
  case SIG_STATE_SUCCESS:
    /* code */
    sigSuccess(frame);
    break;
  case SIG_STATE_FAILED:
    /* code */
    sigFailed(frame);
    break;
  case SIG_STATE_WAITTING:
    /* code */
    sigWaitting(frame);
    break;
  case SIG_STATE_WARNING:
    /* code */
    sigWarning(frame);
    break;
  case SIG_STATE_LOADING:
    /* code */
    sigLoading(frame);
    break;
  case SIG_STATE_SLEEP:
    sigSleep();
    break;
  default:
    fill_solid(signal_leds, NUM_LEDS, CRGB::Black);
    break;
  }
  frame++;
  return sigState;
}

UINT8 sigSuccess(UINT8 frame){
  CHSV color = COL_SUCCESS;
  if(1<=frame && frame<10){
    color.v = BRIGHT_TABLE[frameMap(frame, 1, 10, 0, 255)];
    fill_solid(signal_leds, NUM_LEDS, color);
  }
  else if(10<=frame && frame<55){
    fill_solid(signal_leds, NUM_LEDS, COL_SUCCESS);
  }
  else if(55<=frame && frame<90){
    color.v = BRIGHT_TABLE[iframeMap(frame, 55, 90, 0, 255)];
    fill_solid(signal_leds, NUM_LEDS, color);
  }
  Serial.println(color.v);
  return 0;
}
UINT8 sigFailed(UINT8 frame){
  return 0;
}
UINT8 sigWaitting(UINT8 frame){
  return 0;
}
UINT8 sigWarning(UINT8 frame){
  return 0;
}
UINT8 sigLoading(UINT8 frame){
  return 0;
}
UINT8 sigSleep(void){
  fill_solid(signal_leds, NUM_LEDS, CRGB::Black);
}