#include "Arduino.h"
#include "rgbSignals.h"
#include "ledPinDef.h"
#include "frameMath.h"

CRGB signal_leds[NUM_LEDS];
UINT8 brightness;
UINT8 frame = 1;
const UINT8 MAX_FRAME = 15;
UINT8 sigState = SIG_STATE_SLEEP;
UINT8 prevSigState = sigState;

void sigInit()
{
  FastLED.addLeds<WS2812, PIN_LED_DATA, GRB>(signal_leds, NUM_LEDS);
  frame = 1;
  sigState = SIG_STATE_SLEEP;
  prevSigState = sigState;
}

UINT8 updateSignal(UINT8 sigState)
{
  //当状态发生转变时重置动画帧
  if (prevSigState != sigState)
    frame = 1;
  //当动画播放完毕，且状态不变时不进一步处理
  else if (frame >= 16 && prevSigState == sigState)
    return 0;
  //保存上一帧状态以供判断
  prevSigState = sigState;

  switch (sigState)
  {
  case SIG_STATE_SUCCESS:
    sigSuccess(frame);
    break;
  case SIG_STATE_FAILED:
    sigFailed(frame);
    break;
  case SIG_STATE_WAITTING:
    sigWaitting(frame);
    break;
  case SIG_STATE_WARNING:
    sigWarning(frame);
    break;
  case SIG_STATE_LOADING:
    sigLoading(frame);
    break;
  case SIG_STATE_PROCESSING:
    sigProcessing(frame);
    break;
  case SIG_STATE_SLEEP:
    sigSleep(frame);
    break;
  default:
    fill_solid(signal_leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    break;
  }
  frame++;
  return 0;
}

UINT8 sigSuccess(UINT8 frame)
{
  fill_solid(signal_leds, NUM_LEDS, COL_SUCCESS);
  brightness = BRIGHT_TABLE[frameMap(frame, 1, 15, 0, 255)];
  FastLED.setBrightness(brightness);
  FastLED.show();
  // Serial.println(brightness);
  return 0;
}
UINT8 sigFailed(UINT8 frame)
{
  fill_solid(signal_leds, NUM_LEDS, COL_FAILED);
  brightness = BRIGHT_TABLE[frameMap(frame, 1, 15, 0, 255)];
  FastLED.setBrightness(brightness);
  FastLED.show();
  // Serial.println(brightness);
  return 0;
}
UINT8 sigWaitting(UINT8 frame)
{
  fill_solid(signal_leds, NUM_LEDS, COL_WAITING);
  brightness = BRIGHT_TABLE[frameMap(frame, 1, 15, 0, 255)];
  FastLED.setBrightness(brightness);
  FastLED.show();
  // Serial.println(brightness);
  return 0;
}
UINT8 sigWarning(UINT8 frame)
{
  fill_solid(signal_leds, NUM_LEDS, COL_WARNING);
  brightness = BRIGHT_TABLE[frameMap(frame, 1, 15, 0, 255)];
  FastLED.setBrightness(brightness);
  FastLED.show();
  // Serial.println(brightness);
  return 0;
}
UINT8 sigLoading(UINT8 frame)
{
  fill_solid(signal_leds, NUM_LEDS, COL_WAITING);
  brightness = BRIGHT_TABLE[frameMap(frame, 1, 15, 0, 255)];
  FastLED.setBrightness(brightness);
  FastLED.show();
  // Serial.println(brightness);
  return 0;
}
UINT8 sigProcessing(UINT8 frame)
{
  fill_solid(signal_leds, NUM_LEDS, COL_PROCESSING);
  brightness = BRIGHT_TABLE[frameMap(frame, 1, 15, 0, 255)];
  FastLED.setBrightness(brightness);
  FastLED.show();
  // Serial.println(brightness);
  return 0;
}
UINT8 sigSleep(UINT8 frame)
{
  brightness = BRIGHT_TABLE[iframeMap(frame, 1, 15, 0, 255)];
  FastLED.setBrightness(brightness);
  FastLED.show();
  // Serial.println(brightness);
  return 0;
}