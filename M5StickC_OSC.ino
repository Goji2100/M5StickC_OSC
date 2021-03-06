

//=========================================================================
// M5StickC.ino : chrmlinux / ThreeD / examples / M5StickC
// Copyright (c) 2022 chrmlinux
//-------------------------------------------------------------------------
// @chrmlinux03
//      MIT https://github.com/chrmlinux/ThreeD
// @goji2100
//      MIT https://github.com/Goji2100/M5StickC_OSC
//      Modified for Oscilloscope
//=========================================================================

// espressif / arduino-esp32  1.0.4
// m5stack   / M5StickC       0.2.5
// chrmlinux / ThreeD         0.0.6
// M5StickC_OSC               0.0.2

#include <M5StickC.h>
TFT_eSprite spr(&M5.Lcd);

#include <ThreeD.hpp>
#include "mdl.h"
static ThreeD td;

#define SPR_WW    128
#define SPR_HH    128

void draw(float fact, uint16_t mdlpt, MDL2D_T *mdl, uint16_t lnkpt, LNK_T *lnk) {
  for (int i = 0; i < lnkpt; i++) {
    uint16_t x1 = (SPR_WW / 2) + (mdl[lnk[i].start].x * fact);
    uint16_t y1 = (SPR_HH / 2) + (mdl[lnk[i].start].y * fact);
    uint16_t x2 = (SPR_WW / 2) + (mdl[lnk[i].end].x * fact);
    uint16_t y2 = (SPR_HH / 2) + (mdl[lnk[i].end].y * fact);
    spr.drawLine(x1, y1, x2, y2, lnk[i].color);
    spr.setCursor(x1, y1); spr.print(lnk[i].start);
  }
  spr.setCursor(8, 18); spr.print(td.getfps()); spr.print("fps");
  sendVideo();
}

#define XYZ 14      // pin# for z
#define MAJIC_NUM   800

#if defined ( XYZ )

#define GPIO_OUT_SET_REG *(volatile uint32_t*)0x3FF44008
#define GPIO_OUT_CLR_REG *(volatile uint32_t*)0x3FF4400C
#define XYZ_INIT()  pinMode(XYZ, OUTPUT)
#define XYZ_ON()    GPIO_OUT_SET_REG = BIT(XYZ)
#define XYZ_OFF()   GPIO_OUT_CLR_REG = BIT(XYZ)
#else

#define XYZ_INIT()  
#define XYZ_ON()    
#define XYZ_OFF()   
#endif

static uint8_t outBuf[SPR_HH][SPR_WW + 1];

void sendVideo() {
  uint16_t *videoFb = (uint16_t *)spr.frameBuffer(1);
  int p = 0;

  for (int i = 0, v = 0; i < SPR_HH; i++, v += SPR_WW) {
    outBuf[i][0] = 0;
    for (int j = 0, k = 1; j < SPR_WW; j++) {
      if ( *(videoFb + v + j) ) {
        outBuf[i][0]++;
        outBuf[i][k++] = j;
        p++;
      }
    }
  }

  for (int i = 0; i < SPR_HH; i++) {
    if (outBuf[i][0]) {
      for (int j = 0; j < outBuf[i][0]; j++) {
        if (j == 0) dacWrite(26, 255 - (i << 1));
        for (int l = 0; (l < (MAJIC_NUM / p)); l++)
          dacWrite(25, (outBuf[i][j + 1] << 1));
        if (j == 0) XYZ_ON();
      }
    }
    XYZ_OFF();
  }
  dacWrite(25, 0);
  dacWrite(26, 0);
}

void setup(void) {
  M5.begin();
  Serial.begin( 115200 ); delay(500);
  Serial.println("\n");

  M5.Lcd.setRotation(1);
  spr.createSprite(SPR_WW, SPR_HH);
  Serial.printf("SPR : %d x %d\n", spr.width(), spr.height());
  td.begin();
  XYZ_INIT();
}

void loop(void) {
  float fact = 100.0;
  static uint16_t degy =  0;

  td.rot(ROTY, (degy % 360), mdlpt,   mdl, dst3d);
  td.rot(ROTZ,   24,         mdlpt, dst3d, dst3d);
  td.cnv(view,               mdlpt, dst3d, dst2d);
  spr.fillRect(0, 0, spr.width(), spr.height(), TFT_BLACK);
//spr.setCursor(8, 26); spr.print("y:"); spr.print(degy);
  draw(fact, mdlpt, dst2d, lnkpt, lnk);
//spr.pushSprite(0, 0);

#if (0)
  ++degy %= 360;
#else
  static int rsw = 1;
  degy += rsw;
  if ((degy == 0) || (degy >= 360)) rsw = -rsw;
#endif

}
