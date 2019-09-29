#include <M5StickC.h>
#include <Wire.h>
#include "time.h"
#include <WiFi.h>

// Set RTC time
RTC_DateTypeDef DateStruct;
RTC_TimeTypeDef TimeStruct;
RTC_TimeTypeDef RTC_TimeStruct;
RTC_DateTypeDef RTC_DateStruct;

/*  Custom setting area */
/* TIMEZONE setting  */
#define TIMEZONE 9
/* TIMEZONE setting  */

/* wifi or Smartphone tethering ssid and pass */
const char* ssid = "xxxxxxx";
const char* password = "xxxxxxx";
const char* ntpServer =  "ntp.jst.mfeed.ad.jp";
/*  Custom setting area */

struct tm timeInfo;
const char menu1[]="1:Manual sleep mode!";
const char menu2[]="2:Clock Mode";
const char menu3[]="3:Date&Clock Mode!!";
const int timeout = 120000;


#define DEEP_SEEP 1
#define NOMAL_CLOCK 2
#define NOMAL_DATE_CLOCK 3

#define DISP_DIMMER 4
#define LOW_BAT_DISP 10
#define BAT_MAX 100
#define DISP_DELAY 200
#define LINE_CL 59
#define FALSE 0
#define TRUE 1
#define SLEEP_TIME 1500

hw_timer_t * timer = NULL;
uint64_t  sleep_timer = 0;
char menu_cnt    = 0;
bool bat_flg     = FALSE;
bool line_flg    = FALSE;
bool disp_dimmer = FALSE;
bool time_disp_flg = FALSE;
bool timedate_disp_flg = FALSE;
bool deep_sleep_flg    = FALSE;
bool deep_sleep_cl     = FALSE;
bool move_flg = FALSE;
double vbat   = 0;

void IRAM_ATTR onTimer(){
  if(move_flg == TRUE){
    move_flg    = FALSE;
    disp_dimmer = FALSE;
  }else if(move_flg == FALSE){
    disp_dimmer = TRUE;
  }
}

void setup() {
  sleep_wake();
  pinMode(M5_BUTTON_HOME, INPUT);
  pinMode(M5_BUTTON_RST, INPUT);
  M5.begin();
  wifi_get();
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  M5.Axp.ScreenBreath(8);
  setCpuFrequencyMhz(21);
  time_m5();
  WiFi.disconnect(true);
  M5.Lcd.setCursor(0, 35, 4);
  M5.Lcd.printf("PUSH_M5!!");
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, timeout * 1000, true);
  timerAlarmEnable(timer);
}
void loop(){
  if(digitalRead(M5_BUTTON_HOME) == LOW){
    clear_txt(); 
    time_disp_flg     = FALSE;
    timedate_disp_flg = FALSE;
    deep_sleep_flg    = FALSE;
    menu_cnt++;
    if(menu_cnt > NOMAL_DATE_CLOCK){
      menu_cnt = DEEP_SEEP;
    }
    else{
      ;
    }
    switch (menu_cnt) {
    case DEEP_SEEP:
       for(int i=30;i>0;i--){
        M5.Lcd.setCursor(i, 30, 1);
        M5.Lcd.printf(&menu1[0]);
        delay(20);
      }
      delay(DISP_DELAY);
      break;
    case NOMAL_CLOCK:
      for(int i=30;i>0;i--){
        M5.Lcd.setCursor(i, 40, 1);
        M5.Lcd.printf(&menu2[0]);
        delay(20);
      }
      delay(DISP_DELAY);
      break;
    case NOMAL_DATE_CLOCK:
      for(int i=30;i>0;i--){
        M5.Lcd.setCursor(i, 50, 1);
        M5.Lcd.printf(&menu3[0]);
        delay(20);
      }
      delay(DISP_DELAY);
      break;
    default:
      break;
  }
  move_flg = TRUE;  
  menu_but();
 }
  if(digitalRead(M5_BUTTON_RST) == LOW){
    move_flg = TRUE;
    clear_txt();
  if(menu_cnt == NOMAL_CLOCK){
    time_disp_flg = TRUE;
  }else if(menu_cnt == NOMAL_DATE_CLOCK){
    timedate_disp_flg = TRUE;
  }else if(menu_cnt == DEEP_SEEP){
    deep_sleep_flg = TRUE;
  }
  else{
    def_disp();
  }
  delay(DISP_DELAY);
 }

  if(time_disp_flg == TRUE){
    time_disp();
  }else if(timedate_disp_flg == TRUE){
    time_date_disp();
  }else if(deep_sleep_flg == TRUE){
    deep_sleep();
  }
  sleep_bar();
  bat_disp();
  tft_state();
}
void wifi_get(void){ 
  WiFi.begin(ssid, password);
   while (WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print(".");
  }
}
void time_m5(void){
  configTime(TIMEZONE * 3600, 0, ntpServer);
  getLocalTime(&timeInfo); 
  TimeStruct.Hours   = timeInfo.tm_hour;
  TimeStruct.Minutes = timeInfo.tm_min;
  TimeStruct.Seconds = timeInfo.tm_sec;
  M5.Rtc.SetTime(&TimeStruct);

  DateStruct.WeekDay = timeInfo.tm_wday;
  DateStruct.Month = timeInfo.tm_mon + 1;
  DateStruct.Date = timeInfo.tm_mday;
  DateStruct.Year = timeInfo.tm_year + 1900;
  M5.Rtc.SetData(&DateStruct);
}
void clear_txt(void){
  M5.Lcd.fillScreen(BLACK);
}
void time_disp(void){
  int linedisp = 0;
  M5.Rtc.GetTime(&RTC_TimeStruct);
  M5.Lcd.setCursor(0, 12, 2);
  M5.Lcd.printf("Let's check the time");
  M5.Lcd.setCursor(90, 40, 2);
  M5.Lcd.printf("%02d",RTC_TimeStruct.Seconds);
  M5.Lcd.setCursor(0, 33, 4);
  M5.Lcd.printf("%02d : %02d :",RTC_TimeStruct.Hours, RTC_TimeStruct.Minutes);

  linedisp = map(RTC_TimeStruct.Seconds, 0, 60, 0, 160);

  M5.Lcd.drawFastHLine(0, 64, linedisp-4, RED);
  M5.Lcd.drawFastHLine(0, 65, linedisp, RED);
  M5.Lcd.drawFastHLine(0, 66, linedisp-4, RED);

  if(RTC_TimeStruct.Seconds >= LINE_CL){
      line_flg = TRUE;
  }else{
      if( line_flg == TRUE){
        clear_txt();
        line_flg = FALSE;
      }
  }
}

void time_date_disp(void){
  int linedisp = 0;
  M5.Lcd.setCursor(0, 13, 2);
  M5.Lcd.printf("Let's check the time");
  M5.Rtc.GetTime(&RTC_TimeStruct);
  M5.Rtc.GetData(&RTC_DateStruct);
  M5.Lcd.setCursor(0, 29, 4);
  M5.Lcd.printf("%04d / %02d / %02d\n",RTC_DateStruct.Year, RTC_DateStruct.Month,RTC_DateStruct.Date);
  M5.Lcd.setCursor(0, 52, 2);
  M5.Lcd.printf("%02d : %02d : %02d\n",RTC_TimeStruct.Hours, RTC_TimeStruct.Minutes, RTC_TimeStruct.Seconds);
  linedisp = map(RTC_TimeStruct.Seconds, 0, 70, 0, 160);
  M5.Lcd.drawFastHLine(0, 70, linedisp-4, RED);
  M5.Lcd.drawFastHLine(0, 71, linedisp, RED);
  M5.Lcd.drawFastHLine(0, 72, linedisp-4, RED);

  if(RTC_TimeStruct.Seconds >= LINE_CL){
      line_flg = TRUE;
  }else{
      if( line_flg == TRUE){
        clear_txt();
        line_flg = FALSE;
      }
  }  
}

void bat_disp(void){
  int bat_disp_print;
  vbat = (M5.Axp.GetVapsData() * 1.4);
  bat_disp_print = map(vbat, 3300, 4110, 0, 100);
  if(bat_disp_print > BAT_MAX ){
    bat_disp_print = BAT_MAX;
  }

  if(bat_disp_print < LOW_BAT_DISP){
    M5.Lcd.setCursor(113, 0, 1);
    M5.Lcd.printf("Bat:LoW");
  }else if(bat_disp_print >= BAT_MAX){
    M5.Lcd.setCursor(113, 0, 1);
    M5.Lcd.printf("Bat:Ful");
  }else{
    M5.Lcd.setCursor(113, 0, 1);
    M5.Lcd.printf("Bat:%d%%\n",bat_disp_print);
  }
  M5.Lcd.drawFastHLine(0, 8, 160, RED);
  M5.Lcd.drawFastHLine(0, 9, 160, RED);
  M5.Lcd.drawFastHLine(0, 10, 160, RED);
}

void sleep_wake(void){
  pinMode(GPIO_NUM_37, INPUT_PULLUP);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_37, LOW);
}

void menu_but(void){
  M5.Lcd.setCursor(0, 10, 2);
  M5.Lcd.printf("Simple Clock Menu No:%d",menu_cnt);
  M5.Lcd.setCursor(0, 60, 2);
  M5.Lcd.printf("PUSH_B_BUTTON!!");
}
void tft_state(void){
  if(disp_dimmer == FALSE){
    disp_tft_up();
  }else if(disp_dimmer == TRUE){
    deep_sleep();
  }
}

void disp_tft_up(void){
    M5.Axp.ScreenBreath(8);
}

void disp_tft_dw(void){
    delay(1000);
    M5.Axp.ScreenBreath(0);
}

void deep_sleep(void){
  clear_txt();
  M5.Lcd.setCursor(0, 30, 2);
  M5.Lcd.printf("Go into deep sleep!!\n");
  delay(1000);
  clear_txt();
  M5.Lcd.setCursor(0, 30, 2);
  M5.Lcd.printf("Get up by press the M5!!\n");
  disp_tft_dw();
  delay(3900);
  clear_txt();  
  esp_deep_sleep_start();
}

void sleep_bar(void){
  sleep_timer =  timerRead(timer);
  sleep_timer =  SLEEP_TIME - (sleep_timer/1000/60);
  M5.Lcd.setCursor(0, 0 ,1);
  if((sleep_timer < 50 )&&(move_flg == FALSE)){
    M5.Lcd.printf("Go To Sleep!!\n");
  }else if((sleep_timer < 50 )&&(move_flg == TRUE)){
    M5.Lcd.printf("Can I sleep next\n");
  }else if(sleep_timer < 100 ){
      M5.Lcd.printf("******");
  }else if(sleep_timer < 200 ){
      M5.Lcd.printf("*****");
  }else if(sleep_timer < 500 ){
      M5.Lcd.printf("****");
  }else if(sleep_timer < 750 ){
      M5.Lcd.printf("***");
  }else if(sleep_timer < 1000 ){
      M5.Lcd.printf("**");
  }else if(sleep_timer < 1250 ){
      M5.Lcd.printf("*");
  }
}

void def_disp(void){
    M5.Lcd.setCursor(0, 35, 4);
    M5.Lcd.printf("PUSH_M5!!"); 
}
