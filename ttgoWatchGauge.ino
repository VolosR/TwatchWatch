#include "config.h"
#include "fonts.h"
#include "time.h"
#include <EEPROM.h>

// define the number of bytes you want to access
#define EEPROM_SIZE 1

TTGOClass *ttgo;
AXP20X_Class *power;

TFT_eSPI *tft;
TFT_eSprite *img= nullptr;

#define color1 TFT_WHITE
#define color2  0x8410
#define color3 0x5ACB
#define color4 0x15B3
#define color5 TFT_BLACK

bool irq = false;
volatile int counter = 0;
float VALUE=0;
float lastValue=0;

double rad=0.01745;
float x[360];
float y[360];

float px[360];
float py[360];
float lx[360];
float ly[360];

int r=104;
int sx=120;
int sy=120;

String cc[12]={"45","40","35","30","25","20","15","10","05","0","55","50"};
String days[]={"SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY"};
int start[12];
int startP[60];

const int pwmFreq = 5000;
const int pwmResolution = 8;
const int pwmLedChannelTFT = 0;

int angle=0;
bool onOff=0;
bool debounce=0;

String h,m,s,d1,d2,m1,m2;
String t;

int times;

void setup() {

    EEPROM.begin(EEPROM_SIZE);
    times=EEPROM.read(0);

    ttgo = TTGOClass::getWatch();
    ttgo->begin();
    ttgo->openBL();
     power = ttgo->power;

     power->adc1Enable(
        AXP202_VBUS_VOL_ADC1 |
        AXP202_VBUS_CUR_ADC1 |
        AXP202_BATT_CUR_ADC1 |
        AXP202_BATT_VOL_ADC1,
        true);

    ledcSetup(pwmLedChannelTFT, pwmFreq, pwmResolution);
    ledcAttachPin(12, pwmLedChannelTFT);
    ledcWrite(pwmLedChannelTFT, 90);

      pinMode(AXP202_INT, INPUT_PULLUP);
    attachInterrupt(AXP202_INT, [] {
        irq = true;
    }, FALLING);

     power->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ,
                     true);

    //  Clear interrupt status
    power->clearIRQ();

    
    
   img = new TFT_eSprite(ttgo->tft);
    ttgo->tft->setSwapBytes(true);
        
    img->setSwapBytes(true);
    img->createSprite(240, 240);
    img->setTextDatum(4);

int b=0;
int b2=0;


    for(int i=0;i<360;i++)
    {
       x[i]=(r*cos(rad*i))+sx;
       y[i]=(r*sin(rad*i))+sy;
       px[i]=((r-16)*cos(rad*i))+sx;
       py[i]=((r-16)*sin(rad*i))+sy;

       lx[i]=((r-26)*cos(rad*i))+sx;
       ly[i]=((r-26)*sin(rad*i))+sy;
       
       if(i%30==0){
       start[b]=i;
       b++;
       }

       if(i%6==0){
       startP[b2]=i;
       b2++;
       }
      }

}

int lastAngle=0;
float circle=100;
bool dir=0;
int rAngle=359;
long ttt=0;
String battery="32%";
int brightness=90;

void loop() {

   int16_t xt, yt;
    if (ttgo->getTouch(xt, yt))
    {
      ledcWrite(pwmLedChannelTFT, yt);
      brightness=yt;
      ttt=millis();
      }
  if(millis()>ttt+10000)
  {
    times++;
    EEPROM.write(0, times);
    EEPROM.commit();
    ttgo->powerOff();
   esp_sleep_enable_ext1_wakeup(GPIO_SEL_35, ESP_EXT1_WAKEUP_ALL_LOW);

    esp_deep_sleep_start();
  }

 rAngle=rAngle-2;
 

  

  t=String(ttgo->rtc->formatDateTime());
  String daTe=String(ttgo->rtc->formatDateTime(PCF_TIMEFORMAT_DD_MM_YYYY));
  s=t.substring(6,8);
  m=t.substring(3,5);
  h=t.substring(0,2);

  angle=s.toInt()*6;

  d1=daTe.substring(0,1);
  d2=daTe.substring(1,2);
  m1=daTe.substring(3,4);
  m2=daTe.substring(4,5);

  
  if(angle>=360)
  angle=0;

   if(rAngle<=0)
  rAngle=359;



  if(dir==0)
  circle=circle+0.5;
  else
  circle=circle-0.5;
  
  if(circle>140)
  dir=!dir;

  if(circle<100)
  dir=!dir;



  if(angle>-1)
  {
     lastAngle=angle;      

     VALUE=((angle-270)/3.60)*-1;
     if(VALUE<0)
     VALUE=VALUE+100;
 
     
     
 img->fillSprite(TFT_BLACK);
 img->fillCircle(sx,sy,124,color5);
 
 img->setTextColor(TFT_WHITE,color5);
 
img->drawString("Thursday",circle,120,2);

 
 for(int i=0;i<12;i++)
 if(start[i]+angle<360){
 img->drawString(cc[i],x[start[i]+angle],y[start[i]+angle],2);
 img->drawLine(px[start[i]+angle],py[start[i]+angle],lx[start[i]+angle],ly[start[i]+angle],color1);
 }
 else
 {
 img->drawString(cc[i],x[(start[i]+angle)-360],y[(start[i]+angle)-360],2);
 img->drawLine(px[(start[i]+angle)-360],py[(start[i]+angle)-360],lx[(start[i]+angle)-360],ly[(start[i]+angle)-360],color1);
 }

 

  
 img->setFreeFont(&DSEG7_Modern_Bold_20);
 img->drawString(s,sx,sy-36);
 img->setFreeFont(&DSEG7_Classic_Regular_28);
 img->drawString(h+":"+m,sx,sy+28);
 img->setTextFont(0);

 img->fillRect(70,86,12,20,color3);
 img->fillRect(84,86,12,20,color3);
  img->fillRect(150,86,12,20,color3);
 img->fillRect(164,86,12,20,color3);

 img->setTextColor(0x35D7,TFT_BLACK);
 img->drawString("MONTH",84,78);
 img->drawString("DAY",162,78);
  img->setTextColor(TFT_ORANGE,TFT_BLACK);
  img->drawString("VOLOS PROJECTS",120,174);
  img->drawString("***",120,104);

  img->setTextColor(TFT_WHITE,color3);
  img->drawString(m1,77,96,2);
  img->drawString(m2,91,96,2);

   img->drawString(d1,157,96,2);
  img->drawString(d2,171,96,2);

  img->setTextColor(0x35D7,TFT_BLACK);
  img->drawString("TIMES:"+String(times),10,15);
  img->drawString(String((int)power->getBattVoltage()),204,15);
  img->drawString("BRIGHT:"+String(brightness),10,230);
 
  for(int i=0;i<60;i++)
  if(startP[i]+angle<360)
 img->fillCircle(px[startP[i]+angle],py[startP[i]+angle],1,color1);
 else
 img->fillCircle(px[(startP[i]+angle)-360],py[(startP[i]+angle)-360],1,color1);
 
 img->fillTriangle(sx-1,sy-70,sx-5,sy-56,sx+4,sy-56,TFT_ORANGE);
 img->fillCircle(px[rAngle],py[rAngle],6,TFT_RED);

 img->drawRect(186,8,36,14,0x35D7);
 img->fillRect(222,12,4,6,0x35D7);
 img->pushSprite(0, 0);

 
}

}
