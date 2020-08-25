#include "frameMath.h"

UINT8 frameMap(UINT8 nowFrame, UINT8 startFrame, UINT8 endFrame, UINT8 min, UINT8 max){
  if(nowFrame>endFrame || startFrame>endFrame || min>max){
    return 1;
  }
  double result = (double)(nowFrame-startFrame)/(double)(endFrame-startFrame)*(double)(max-min);
  return (UINT8)result;
}

UINT8 iframeMap(UINT8 nowFrame, UINT8 startFrame, UINT8 endFrame, UINT8 min, UINT8 max){
  if(nowFrame>endFrame || startFrame>endFrame || min>max){
    return 1;
  }
  double result = (double)(endFrame-nowFrame)/(double)(endFrame-startFrame)*(double)(max-min);
  return (UINT8)result;
}