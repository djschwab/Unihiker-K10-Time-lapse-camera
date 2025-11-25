#include <GT30L24A3W.h>
#include <unihiker_k10.h>

/******************************************************
Unihiker K10 time lapse capture sequence of camera images
 to SD card or stream camera to screen.
Time lapse capture images have selectable reolution. 
DJS - 11/1/2025
*******************************************************/
//these are already present in unihiker_k10.h
//#include "esp_camera.h"
//#include <SD.h>
//#include <TFT_eSPI.h> 
//#include <SPI.h>
#include "unihiker_k10.h"
UNIHIKER_K10    k10;

// forward declaration of button handlers
void onButtonAPressed();
void onButtonBPressed();

// Pin assignments for Unihiker K10 camera - these are
// needed for esp_camera configuration for variable image size
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      7
#define SIOD_GPIO_NUM     47
#define SIOC_GPIO_NUM     48

#define Y9_GPIO_NUM       6
#define Y8_GPIO_NUM       15
#define Y7_GPIO_NUM       16
#define Y6_GPIO_NUM       18
#define Y5_GPIO_NUM       9
#define Y4_GPIO_NUM       11
#define Y3_GPIO_NUM       10
#define Y2_GPIO_NUM        8
#define VSYNC_GPIO_NUM    4
#define HREF_GPIO_NUM     5
#define PCLK_GPIO_NUM     17

// possible camera resolutions
const char * res_desc[] = {
"240x320",
"640x480",
"800x600",
"1280x720",
"1280x1024"};

// framesize descriptors for various resolutions
// Note: the resulting frame sizes with K10 camera
// are NOT what is usually associated with these descriptors
framesize_t res_index[] = {
FRAMESIZE_QVGA,
FRAMESIZE_SVGA,
FRAMESIZE_XGA,
FRAMESIZE_SXGA,
FRAMESIZE_UXGA};

int nres=sizeof(res_index)/sizeof(res_index[0]); // number of different resolutions
bool btna; // button a pressed
bool btnb; // button b pressed
int mode=0; // 0=streaming, 1=timelapse
int ires=0; // selected resolution index
camera_fb_t * fb = NULL; // frame buffer pointer
size_t _jpg_buf_len = 0; // jpeg buffer length
uint8_t * _jpg_buf = NULL; // jpeg buffer pointer
uint32_t _jpg_width = NULL; // frame width
uint32_t _jpg_height = NULL; // frame height
uint32_t delta_t = 0; // time interval between images
uint32_t last_t = 0; // time of last image
uint32_t start_t = 0; // time of first image
uint32_t img_count = 0; // count of images
char filename[50]; // filename on SD card
char buffer[50]; // character buffer
int n;
File file; // filename of image

TFT_eSPI tft = TFT_eSPI(); // eSPI tft screen handler

void setup() {
  Serial.begin(115200);
  Serial.println("***Start***");
  k10.begin();
  k10.buttonA->setPressedCallback(onButtonAPressed);
  k10.buttonB->setPressedCallback(onButtonBPressed);
  // Init K10 screen - needed for streaming
  k10.initScreen(2);
  // Init eSPI screen handler
  tft.init();
  tft.setRotation(2);
  tft.setTextSize(1);
  tft.fillScreen(TFT_BLACK);
  // Select mode
  tft.setTextColor(TFT_BLUE, TFT_BLACK);
  tft.drawString("Select Camera Mode:", 0, 0, 4);
  btna=false;
  btnb=false;
  while(!btnb){
    if(btna){
      mode=(mode+1)%2;
      btna=false;
    }
    if(mode==0){
      tft.setTextColor(TFT_YELLOW, TFT_BLACK);
      tft.drawString("Stream to Display", 0, 30, 4);
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.drawString("Time lapse", 0, 55, 4);
    } else {
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.drawString("Stream to Display", 0, 30, 4);
      tft.setTextColor(TFT_YELLOW, TFT_BLACK);
      tft.drawString("Time lapse", 0, 55, 4);
    }
  }
  // if streaming mode is set, start camera streaming to screen
  if(mode==0){
    k10.initBgCamerImage();
    k10.setBgCamerImage();
  } else {
  // else select resolution for time lapse images
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_BLUE, TFT_BLACK);
    tft.drawString("Select Resolution:", 0, 0, 4);
    btna=false;
    btnb=false;
    while(!btnb){
      if(btna){
        ires=(ires+1)%nres;
        btna=false;
      }
      int y=30;
      int dy=25;
      for (int i=0; i<nres; i++){
        if(i != ires){
          tft.setTextColor(TFT_GREEN, TFT_BLACK);
        } else {
          tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        }
        tft.drawString(res_desc[i], 0, y, 4);
        y=y+dy; 
      }
    }
    // Select time interval (hrs, mns, sec)
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_BLUE, TFT_BLACK);
    tft.drawString("Set time lapse:", 0, 0, 4);
    int hrs=0;
    btna=false;
    btnb=false;
    while(!btnb){
      if(btna){
        hrs=(hrs+1)%24;
        btna=false;
      }
      sprintf(buffer, "Hours: %d     ", hrs);
      tft.setTextColor(TFT_YELLOW, TFT_BLACK);
      tft.drawString(buffer, 0, 35, 4);
    }
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString(buffer, 0, 35, 4);
    int mns=0;
    btna=false;
    btnb=false;
    while(!btnb){
      if(btna){
        mns=(mns+1)%60;
        btna=false;
      }
      sprintf(buffer, "Minutes: %d     ", mns);
      tft.setTextColor(TFT_YELLOW, TFT_BLACK);
      tft.drawString(buffer, 0, 60, 4);
    }
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString(buffer, 0, 60, 4);
    int sec=0;
    btna=false;
    btnb=false;
    while(!btnb){
      if(btna){
        sec=(sec+1)%60;
        btna=false;
      }
      sprintf(buffer, "Seconds: %d     ", sec);
      tft.setTextColor(TFT_YELLOW, TFT_BLACK);
      tft.drawString(buffer, 0, 85, 4);
    }
    // display final settings
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString(buffer, 0, 85, 4);

    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_BLUE, TFT_BLACK);
    tft.drawString("Settings:", 0, 0, 4);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    sprintf(buffer, "Res: %s", res_desc[ires]);
    tft.drawString(buffer, 0, 35, 4);
    sprintf(buffer, "Interval: %02d:%02d:%02d", hrs,mns,sec);
    tft.drawString(buffer, 0, 60, 4);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    sprintf(buffer, "OK to start?");
    tft.drawString(buffer, 0, 90, 4);
    btna=false;
    btnb=false;
    while(!btna & !btnb){delay (100);}
    tft.fillScreen(TFT_BLACK);
    start_t=millis();
    delta_t=1000*(3600*hrs+60*mns+sec);
 // }

  // Init SD File System
  SD.begin();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 10000000;
//  config.pixel_format = PIXFORMAT_JPEG;  // doesn't seem to work 
//  config.pixel_format = PIXFORMAT_YUV422; 
  config.pixel_format = PIXFORMAT_RGB565; 
  config.frame_size = res_index[ires];
  config.grab_mode = CAMERA_GRAB_LATEST;
  config.fb_count = 2;
  
  // ESP camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  sensor_t * s = esp_camera_sensor_get();
  s->set_hmirror(s, 1);        // 0 = disable , 1 = enable       // required to get colors right
}
}
void loop() {
  // if in streaming mode, do nothing
  if(mode==0){
      delay(1000);
  } else {
    if(millis()-last_t>delta_t){
      fb = esp_camera_fb_get();
      if (fb) {
        bool jpeg_converted = fmt2jpg(fb->buf, fb->len, fb->width, fb->height,
                    fb->format, 32, &_jpg_buf, &_jpg_buf_len);
        _jpg_width=fb->width;
        _jpg_height=fb->height;
        esp_camera_fb_return(fb);
        fb = NULL;
        img_count = img_count+1;
        n=sprintf (filename, "/img%05d.jpg", img_count);
        Serial.printf("Filename: %s\n",filename);
        file = SD.open(filename, FILE_WRITE);
        file.write(_jpg_buf,_jpg_buf_len);
        file.close();
        tft.setTextColor(TFT_BLUE, TFT_BLACK);
        sprintf(buffer, "Time: %d     ", (millis()-start_t)/1000);
        tft.drawString(buffer, 0, 0, 4);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.drawString(filename, 0, 35, 4);

        Serial.printf(" width: %d height: %d buffer length: %d\n",_jpg_width,_jpg_height,
          (uint32_t)(_jpg_buf_len));
        free(_jpg_buf);
        _jpg_buf = NULL;
      } else {
        Serial.println("Camera capture failed");
      }
      last_t=millis();
    }

    delay(1);
  }
}

void onButtonAPressed(){
  btna=true;
};
void onButtonBPressed(){
  btnb=true;
};
