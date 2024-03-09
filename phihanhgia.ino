#include <TFT_eSPI.h>
#include <WiFi.h>
#include "phihanhgia.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <PubSubClient.h>

//khai báo các biến wifi
const char *id[] ={"Hoang","Trung Kien","Trung Kien 1","Highlands Coffee","Tuấn"};
const char *mk[] = {"20002000","0983068244","0983068244","","manucian"};
char *tenWifi[20];

//khai báo biến bàn phím
/*
char banPhim[4][10]={
  {'1','2','3','4','5','6','7','8','9','0'},
  {'q','w','e','r','t','y','u','i','o','p'},
  {'a','s','d','f','g','h','j','k','l',';'},
  {'z','x','c','v','b','n','m',',','.','/'}
};

char banPhim1[4][11]={
  {'!','@','#','$','%','^','&','*','(',')'},
  {'Q','W','E','R','T','Y','U','I','O','P'},
  {'A','S','D','F','G','H','J','K','L',':'},
  {'Z','X','C','V','B','N','M','<','>','?'}
};
*/
// khởi tạo biến update time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
//
WiFiClient espClient;
PubSubClient client(espClient);
//************************************
const uint16_t mqttBufferSize = 4096;
char mqttBuffer[mqttBufferSize];

const char* mqtt_server = "test.mosquitto.org"; // Địa chỉ IP của máy chủ Mosquitto
const int mqtt_port = 1883;
const char* mqtt_client_id = "ESP32Client";


// định nghĩa và các biến trạng thái nút ấn - các nút ấn thực hiện lệnh sau mỗi lần nhấn xuống
#define nut1 13
#define nut2 12
#define nut3 14
int lastState1 = HIGH; // các biến trạng thái hiện tại của nút ấn
int lastState2 = HIGH;
int lastState3 = HIGH;
int currentState1;     // các biến trạng thái trước đó của nút ấn
int currentState2;
int currentState3;

// Thiết lập thông tin OpenWeatherMap
const char* openWeatherMapApiKey = "d0da8244546a51a627468321164e8210";
const char* openWeatherMapCity[] = {"Hanoi","Thai Binh","Ho Chi Minh City","Nam Dinh","Hung Yen","Hai Duong","Hai Phong","Ninh Binh",
                                    "Thanh Hoa","Hoa Binh","Bac Ninh","Bac Giang","Thai Nguyen","Lang Son","Bac Kan","Tuyen Quang",
                                    "Yen Bai","Lao Cai","Son La","Ha Giang"};
const char* vietTatMapCity[] = {"HN","TB","HCM","ND","HY","HD","HP","NB","TH","HB","BN","BG","TN","LS","BK","TQ","YB","LC","SL","HG"};
const char* openWeatherMapCountryCode = "VN"; // Mã quốc gia của Việt Nam
const char* openWeatherMapUnits = "metric"; // Đơn vị Celsius
// biến thời tiết
double nhietDo[20];
double doAm[20];
double huongGio[20];
double tocDoGio[20];
double may[20];

// biến lưu vị trí hiển thị tỉnh
int xTinh[20];
int yTinh[20];

// Địa chỉ IP của OpenWeatherMap
const char* openWeatherMapServer = "api.openweathermap.org";

// định nghĩa các biến đồng hồ
#define UPDATE_TIME_INTERVAL 60000
unsigned long currentTime = 0;
unsigned long prevClockTime = 0;
const char* const weekString[] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
int phut = 0; // biến phút
int gio = 0;  // biến giờ
int tuan = 0; // biến chỉ thứ trong tuần
int ngay = 1; // biến ngày

//biến vẽ đồng hồ
float mx[60];
float my[60];
float hx[60];
float hy[60];
int cx=80;
int cy=60;

// biến màu sắc
unsigned short background = TFT_BLACK;
const uint16_t pink = 0xb4e5d2;
int red=148;
int green=248;
int blue=252;
int chua[3];
int *ptrRed=&red;
int *ptrGreen=&green;
int *ptrBlue=&blue;

// biến màu nền
uint16_t bg1[] = {
  0x0000, 
  0x0000, 
  0x0000, 
  0x0000, 
  0xFFFF,
  0x0000, 
  0x0000,
  0x0000,
  0x0000,
};

// các biến chốt lựa chọn
int chonHienThiCaiDat=0;
int chonHienThiDongHo=0;
int chonHienThiMau=0;
int chonTinh=0;
int chonThoiTiet=0;
int chonHienThiTinh=0;

// biến vị trí màn hình con
int xLC;
int yLC;
int xTHT;
int yTHT;

// biến cung lựa chọn
float x[360];
float y[360];
float xi[360];
float yi[360];
float tx[360];
float ty[360];
float lx[360];
float ly[360];
float shx[360];
float shy[360];
float shxi[360];
float shyi[360];
int angle=15,a,chon;
int b=-a;
double rad=0.01745;
double pi=3.14159;
double r=63;
//*********************************

// khởi tạo màn hình chính và các màn hình con
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite ttt = TFT_eSprite(&tft);
TFT_eSprite tht = TFT_eSprite(&tft); //thanh hiển thị thông tin (ngày/tháng/năm/giờ/thời tiết)
TFT_eSprite lc = TFT_eSprite(&tft);  //màn hình lựa chọn 
TFT_eSprite tt = TFT_eSprite(&tft);  // màn hình hiển thị thời tiết
//*********************************

//
HTTPClient http;
//

// biến màu chính
uint16_t color = tft.color565(red,green,blue);
uint16_t color1 = tft.color565(255-red,255-green,255-blue);
//*********************************

//***< các chế độ lựa >***
// lưa chọn chính
char *luaChonChinh[] = {
  "Ket noi Wifi",
  "IOT",
  "Do luong",
  "Dong ho",
  "Cai dat",
  "Tro lai"
};
// lựa chọn cài đặt
char *luaChonCaiDat[] = {
  "Toan man hinh",
  "Tat thong bao tin nhan",
  "Tat thong bao cuoc goi",
  "Chinh mau",
  "Che do sang/toi",
  "Tro lai"
};
// lựa chọn đồng hồ
char *luaChonDongHo[] = {
  "Update time",
  "bao thuc",
  "bam gio",
  "hen gio",
  "Tro lai"
};
//*********************************

//***< Chương trình khởi tạo chung >***
void setup(){
  pinMode(nut1, INPUT_PULLUP);   // cấu hình nút ấn 
  pinMode(nut2, INPUT_PULLUP);
  pinMode(nut3, INPUT_PULLUP);
  // khởi tảo màn hình chính
  tft.init();
  tft.setRotation(1);
  // màn hình khởi động
  tft.fillScreen(background);
  tft.pushImage(74,0,172,172,logo);
  delay(5000);
  tft.fillScreen(background);
  // cấu hình màn hình con
  tht.createSprite(320,25);
  lc.createSprite(320,147);
  tt.createSprite(320,147);
  // setup toạ đọ vẽ cung tròn
  for(int i=0;i<360;i++){
    x[i]=(r-15)*cos(i*rad)-10;
    y[i]=(r-15)*sin(i*rad)+73;
    shx[i]=(r-5)*cos(i*rad)-10;
    shy[i]=(r-5)*sin(i*rad)+73;
    lx[i]=(r+2)*cos(i*rad)-10;
    ly[i]=(r+2)*sin(i*rad)+73;
    tx[i]=(r+10)*cos(i*rad)-10;
    ty[i]=(r+5)*sin(i*rad)+73;
    xi[i]=(r-35)*cos(i*rad)-10;
    yi[i]=(r-35)*sin(i*rad)+73;
    shxi[i]=(r-25)*cos(i*rad)-10;
    shyi[i]=(r-25)*sin(i*rad)+73;
  }
  // toạ độ vẽ đồng hồ
  for(int i=0;i<60;i++){
    mx[i]=42*cos(i*6*rad-pi/2)+cx;
    my[i]=42*sin(i*6*rad-pi/2)+cy;
    hx[i]=32*cos(i*6*rad-pi/2)+cx;
    hy[i]=32*sin(i*6*rad-pi/2)+cy;
  }
  //khởi chạy thời gian từ NTP
  yTinh[0]=63;
  
}
//*********************************

// chương trình màn hình chính (chỉ hiển thị thời gian và thời tiết )
void mainScreen(){
  while(true){
    updateTime();
    tht.fillSprite(background);
    tht.setTextColor(color);
    tht.setTextDatum(ML_DATUM);
    tht.setCursor(20,5);
    tht.setTextSize(2);
    tht.print(weekString[tuan]);
    tht.print("  ");
    tht.print(gio);
    tht.print(":");
    tht.print(phut);
    tht.setCursor(180,5);
    tht.print(vietTatMapCity[chonTinh]);
    if(WiFi.status() == WL_CONNECTED){
      tht.setCursor(220,5);
      tht.print(nhietDo[chonTinh]);
      tht.print("C");
    }else{
      tht.setCursor(240,5);
      tht.print("!!!");
    }
    tht.pushSprite(0,0);
    currentState1 = digitalRead(nut1);
    if(currentState1 == LOW && lastState1 == HIGH){
      chuongTrinhThoiTiet();
    }
    lastState1 = currentState1;
    currentState2 = digitalRead(nut2);
    if(currentState2 == LOW && lastState2 == HIGH){
      nhatin();
    }
    lastState2 = currentState2;
    currentState3 = digitalRead(nut3);
    if(currentState3 == LOW && lastState3 == HIGH){
      chuongTrinhChinh();
    }
    lastState3 = currentState3;
  }
}
//
void callback(char* topic, byte* payload, unsigned int length) {
  lc.fillScreen(background);
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  for (int i = 0; i < length; i++) {
  Serial.print((char)payload[i]);
  lc.print((char)payload[i]);
  }
  Serial.println();
  lc.pushSprite(0,25);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(mqtt_client_id)) {
      client.subscribe("hello/minh");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void nhatin(){
  tt.fillScreen(background);
  tt.setCursor(0,25);
  tt.setTextSize(2);
  tt.setTextColor(color);
  tt.print("tin nhan:");
  tt.pushSprite(0,25);
  while(1){
    //if (!client.connected()) {
    //reconnect();
    //}
    //client.loop();
    currentState2 = digitalRead(nut2);
    if(currentState2 == LOW && lastState2 == HIGH){
      break;
    }
    lastState2 = currentState2;
  }
  tt.fillScreen(background);
  tt.pushSprite(0,25);
}
//***< chương trình đồng hồ >***
void dongHo(){
  updateTime();
  tht.setTextDatum(ML_DATUM);
  tht.setTextColor(TFT_BLACK);
  tht.setCursor(20,5);
  tht.setTextSize(2);
  tht.print(weekString[tuan]);
  tht.print("  ");
  tht.print(gio);
  tht.print(":");
  tht.print(phut);
  tht.setCursor(180,5);
  tht.print(vietTatMapCity[chonTinh]);
  tht.pushSprite(0,0);
}

void thoiTiet(){
  if (WiFi.status() == WL_CONNECTED) {
    String url = "/data/2.5/weather?q=" + String(openWeatherMapCity[chonTinh]) + "," + String(openWeatherMapCountryCode) + "&units=" + String(openWeatherMapUnits) + "&appid=" + String(openWeatherMapApiKey);
    if (http.begin(openWeatherMapServer, 80, url)) {
            int httpCode = http.GET();
            if (httpCode > 0) {
                String payload = http.getString();
                // Phân tích dữ liệu JSON
                DynamicJsonDocument doc(1024);
                deserializeJson(doc, payload);
                // Lấy thông tin thời tiết từ JSON
                String weatherDescription = doc["weather"][0]["description"];
                nhietDo[chonTinh]=doc["main"]["temp"];
                doAm[chonTinh]=doc["main"]["humidity"];
                huongGio[chonTinh]=doc["wind"]["speed"];
                tocDoGio[chonTinh]=doc["wind"]["deg"];
                may[chonTinh]=doc["clouds"]["all"];
            }
            http.end();
    }
  }
}

void updateTime(){
  currentTime = millis();
  if(currentTime - prevClockTime > UPDATE_TIME_INTERVAL){
    prevClockTime = currentTime -120;
    phut++;
    if(phut>=60){
      phut=0;
      gio++;
      if(gio>=24){
        gio=0; 
        tuan++;
        if(tuan>7){
          tuan=1;
        }
        ngay++;
        if(ngay>30){
          ngay=1;
        }
        
      }
    }
    tht.fillSprite(color);
    thoiTiet();
    if(WiFi.status() == WL_CONNECTED){
      tht.setCursor(220,5);
      tht.setTextColor(TFT_BLACK);
      tht.setTextSize(2);
      tht.print(nhietDo[chonTinh]);
      tht.print("C");
    }else{
      tht.setCursor(240,5);
      tht.setTextColor(TFT_BLACK);
      tht.print("!!!");
    }
  }
}

void chuongTrinhThoiTiet(){
  lc.fillSprite(background);
  if (WiFi.status() == WL_CONNECTED) {
    chuongTrinhHienThiThoiTiet();
    chuongTrinhHienThiLuaChonThoiTiet();
  }else{
    lc.setCursor(30,50);
    lc.setTextSize(4);
    lc.setTextColor(color);
    lc.print("DISCONECTED");
  }
  for(int i=319;i>=0;i=i-2){
    lc.pushSprite(i,25);
    delay(1);
  }
  tht.fillSprite(color);
  if(WiFi.status() == WL_CONNECTED){
    while(true){
      dongHo();
      chuongTrinhHienThiThoiTiet();
      chuongTrinhHienThiLuaChonThoiTiet();
      lc.pushSprite(0,25);
      currentState1 = digitalRead(nut1);
      if(currentState1 == LOW && lastState1 == HIGH){
        chonThoiTiet++;
        if(chonThoiTiet>1){
          chonThoiTiet=0;
        }
        chuongTrinhHienThiThoiTiet();
        chuongTrinhHienThiLuaChonThoiTiet();
        lc.pushSprite(0,25);
      }
      lastState1 = currentState1;
      currentState3 = digitalRead(nut3);
      if(currentState3 == LOW && lastState3 == HIGH){
        chonThoiTiet--;
        if(chonThoiTiet<0){
          chonThoiTiet=1;
        }
        chuongTrinhHienThiThoiTiet();
        chuongTrinhHienThiLuaChonThoiTiet();
        lc.pushSprite(0,25);
      }
      lastState3 = currentState3;
      currentState2 = digitalRead(nut2);
      if(currentState2 == LOW && lastState2 == HIGH){
        if(chonThoiTiet==0){
          break;
        }
        if(chonThoiTiet==1){
          chuongTrinhHienThiLuaChonTinh();
        }
      }
      lastState2 = currentState2;
    }
  }else{
    while(true){
      tht.setCursor(240,5);
      tht.setTextColor(TFT_BLACK);
      tht.print("!!!");
      dongHo();
      currentState2 = digitalRead(nut2);
      if(currentState2 == LOW && lastState2 == HIGH){
        break;
      }
    lastState2 = currentState2;
    }
  }
  for(int i=1;i<320;i=i+2){
    lc.pushSprite(i,25);
    delay(1);
  }
}

void chuongTrinhHienThiThoiTiet(){
  lc.setTextSize(2);
  lc.setTextColor(color);
  lc.setCursor(10,45);
  lc.print("Nhiet do:");
  lc.print(nhietDo[chonTinh]);
  lc.print(" C");
  lc.setCursor(10,65);
  lc.print("Do am:");
  lc.print(doAm[chonTinh]);
  lc.print("%");
  lc.setCursor(10,85);
  lc.print("Van toc gio:");
  lc.print(tocDoGio[chonTinh]);
  lc.print("m/s");
  lc.setCursor(10,105);
  lc.print("Huong gio:");
  lc.print(huongGio[chonTinh]);
  lc.print(" C");
  lc.setCursor(10,125);
  lc.print("may:");
  lc.print(may[chonTinh]);
  lc.print("%");
}

void chuongTrinhHienThiLuaChonThoiTiet(){
  if(chonThoiTiet==0){
    lc.fillSmoothRoundRect(60,20 , 200, 20,3, color);
    lc.setCursor(80,22);
    lc.setTextColor(color1);
    lc.setTextSize(2);
    lc.print(openWeatherMapCity[chonTinh]);
    lc.drawWedgeLine(290,45,290,145,1,1,color1,background);
  }
  if(chonThoiTiet==1){
    lc.fillSmoothRoundRect(60,20 , 200, 20,3, color1);
    lc.setCursor(80,22);
    lc.setTextColor(color);
    lc.setTextSize(2);
    lc.print(openWeatherMapCity[chonTinh]);
    lc.drawWedgeLine(290,45,290,145,1,1,color,background);
  }
}

void chuongTrinhHienThiLuaChonTinh(){
  lc.fillSprite(background);
  hienThiLuaChonTinh(openWeatherMapCity);
  delay(1000);
  while(true){
    dongHo();
    currentState1 = digitalRead(nut1);
    if(currentState1 == LOW && yTinh[19]>60){
      yTinh[0]=yTinh[0]-4;
      hienThiLuaChonTinh(openWeatherMapCity);
    }
    currentState3 = digitalRead(nut3);
    if(currentState3 == LOW && yTinh[0]<65){
      yTinh[0]=yTinh[0]+4;
      hienThiLuaChonTinh(openWeatherMapCity);
    }
    currentState2 = digitalRead(nut2);
    if(currentState2 == LOW && lastState2 == HIGH){
      for(int i=0;i<20;i++){
        if(yTinh[i]<=73 && yTinh[i]>=51){
          tht.fillRect(180, 0, 40, 25, color);
          chonTinh=i;
        }
      }
      lc.fillSprite(background);
      break;
    }
    lastState2 = currentState2;
  }
}

void hienThiLuaChonTinh(const char* tinh[]){
  lc.fillSprite(background);
  for(int i=0;i<19;i++){
    yTinh[i+1]=yTinh[i]+24;
  }
  for(int i=0;i<20;i++){
    if(yTinh[i]<70 && yTinh[i]>54){
      lc.fillSmoothRoundRect(30,yTinh[i], 260, 20,3, TFT_WHITE);
      lc.setCursor(80,yTinh[i]+2);
      lc.setTextColor(TFT_BLACK);
      lc.setTextSize(2);
      lc.print(openWeatherMapCity[i]);
    }else{
      lc.fillSmoothRoundRect(40,yTinh[i], 240, 20,3, color);
      lc.setCursor(80,yTinh[i]+2);
      lc.setTextColor(color1);
      lc.setTextSize(2);
      lc.print(openWeatherMapCity[i]);
    }
  }
  lc.pushSprite(0,25);
}

//***< chương trình cung trong lựa chọn >***
void luaChon(){
  lc.fillSprite(background);
  veCungTron();
  for(int i=0;i<320;i=i+2){
    lc.pushSprite(i-319,25);
    delay(1);
  }
  tht.fillSprite(color);
  tht.setCursor(240,5);
  tht.setTextColor(TFT_BLACK);
  tht.print("!!!");
  while(true){
    dongHo();
    currentState1 = digitalRead(nut1);
    if(currentState1 == LOW){
      angle=angle+4;
      if(angle>359){
        angle = 0;
      }
      lc.fillSprite(background);
      veCungTron();
      lc.pushSprite(0,25);
      delay(30);
    }
    currentState3 = digitalRead(nut3);
    if(currentState3 == LOW){
      angle=angle-4;
      if(angle<0){
        angle = 359;
      }
      lc.fillSprite(background);
      veCungTron();
      lc.pushSprite(0,25);
      delay(30);
    }
    currentState2 = digitalRead(nut2);
    if(currentState2 == LOW){
      break;
    }
    lastState2 = currentState2;
  }
}

void veCungTron(){
  for(int i=0;i<120;i++)
  {
   a=angle+(i*3);
   if(a>359){
   a=(angle+(i*3))-360;
   }
   int pos=ty[a];
   if(i%5==0 && ty[a]>0 && ty[a]<147){
    lc.drawWedgeLine(x[a],y[a],shx[a],shy[a],1,2,TFT_WHITE,TFT_BLACK);
    lc.drawWedgeLine(xi[359-a],yi[359-a],shxi[359-a],shyi[359-a],1,2,TFT_WHITE,TFT_BLACK);
   }
   if(i%10==0 && ty[a]>0 && ty[a]<147 ){
      lc.drawWedgeLine(x[a],y[a],lx[a],ly[a],4,1,TFT_WHITE,TFT_BLACK);
      if(tx[a]>0){
        if(ty[a]<=82 && ty[a]>=62){
          lc.setTextColor(0x0000);
          lc.fillSmoothRoundRect(lx[a], ly[a]-8, 200, 20,3, TFT_WHITE);
        }else{
          lc.setTextColor(color);
        }
      lc.setCursor(tx[a],ty[a]-4);
      chon = i/10;
      if(chon>5){
      chon = chon-6;
      }
      lc.setTextSize(2);
      lc.print(luaChonChinh[chon]);
      }
   }
  }
}
//*********************************

//***< chương trình chính >***

void chuongTrinhChinh(){
  while(true){

    luaChon();
    if((angle<=15 && angle>=0)||(angle>345 && angle<=359)||(angle<=195) && (angle>165)){
      chuongTrinhWifi();
    }
    if((angle<=165 && angle>135)||(angle<=345) && (angle>315)){
      chuongTrinhIot();
    }
    if((angle<=135 && angle>105)||(angle<=315) && (angle>285)){
      chuongTrinhDoLuong();
    }
    if((angle<=105 && angle>75)||(angle<=285) && (angle>255)){
      chuongTrinhDongHo();
    }
    if((angle<=75 && angle>45)||(angle<=255) && (angle>225)){
      chuongTrinhCaiDatChinh();
    }
    if((angle<=45 && angle>15)||(angle<=225) && (angle>195)){
      break;
    }
  }
  for(int i=319;i>=0;i=i-2){
    lc.pushSprite(i-319,25);
    delay(1);
  }
  angle=15;
}
//*********************************

//***< chương trình kêt nối Wifi >***
void chuongTrinhWifi(){
  lc.fillSprite(background);
  lc.setCursor(0,50);
  lc.setTextSize(2);
  lc.setTextColor(color);
  int numNetworks = WiFi.scanNetworks();
  WiFi.begin(id[1],mk[1]);
  lc.print("DANG KET NOI:\n");
  lc.pushSprite(0,25);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    lc.print(".");
    lc.pushSprite(0,25);
  }
  lc.print("\nKET NOI THANH CONG:");
  lc.print("\nDIA CHI IP:");
  lc.print(WiFi.localIP());
  lc.pushSprite(0,25);
  while(true){
  currentState2 = digitalRead(nut2);
  if(currentState2 == LOW && lastState2 == HIGH){
    break;
  }
  lastState2 = currentState2;
  }
}

void chuongTrinhNhapMatKhau(){

}
//*********************************

//***< chương trình bàn phím >***
void banPhim(){

}
//***********************************

//***< chương trình IOT >***
void chuongTrinhIot(){

}
//*********************************

//***< chương trình đo lường >***
void chuongTrinhDoLuong(){

}
//*********************************

//***< chương trình đồng hồ nhiều chế độ >***

void chuongTrinhDongHo(){
  while(true){
    chuongTrinhLuaChonDongHo();
    if(chonHienThiDongHo==0){
      chuongTrinhUpdateTime();
      delay(100);
    }
    if(chonHienThiDongHo==1){
      
    }
    if(chonHienThiDongHo==2){
      
    }
    if(chonHienThiDongHo==3){
      
    }
    if(chonHienThiDongHo==4){
      break;
    }
  }
}



void chuongTrinhLuaChonDongHo(){
  veDongHo();
  delay(500);
  while(true){
    veDongHo();
    currentState1 = digitalRead(nut1);
    if(currentState1 == LOW && lastState1 == HIGH){
      chonHienThiDongHo++;
      if(chonHienThiDongHo>4){
        chonHienThiDongHo=0;
      }
      veDongHo();
      delay(30);
    }
    lastState1 = currentState1;
    currentState2 = digitalRead(nut2);
    if(currentState2 == LOW && lastState2 == HIGH){
      break;
    }
    lastState2 = currentState2;
    currentState3 = digitalRead(nut3);
    if(currentState3 == LOW && lastState3 == HIGH){
      chonHienThiDongHo--;
      if(chonHienThiDongHo<0){
        chonHienThiDongHo=4;
      }
      veDongHo();
      delay(30);
    }
    lastState3 = currentState3;
  }
}

void veDongHo(){
  lc.fillScreen(background);
  dongHo();
  lc.fillCircle(cx, cy, 50, TFT_GREEN);
  lc.fillCircle(cx, cy, 48, TFT_WHITE);
  if(gio>=12){
    lc.drawWedgeLine(cx,cy,hx[(gio-12)*5],hy[(gio-12)*5],3,1,TFT_RED);
  }else{
    lc.drawWedgeLine(cx,cy,hx[gio*5],hy[gio*5],3,1,TFT_RED);
  }
  lc.drawWedgeLine(cx,cy,mx[phut],my[phut],2,1,TFT_BLACK);
  lc.setTextDatum(MC_DATUM);
  lc.setTextColor(TFT_WHITE);
  lc.drawString(String(gio)+":"+String(phut),cx,130,2);
  veHienThiLuaChonThanhNgang(5,150,6,160,24,0xfcb8f8,color,2,chonHienThiDongHo,luaChonDongHo);
  lc.pushSprite(0,25);
}

void chuongTrinhUpdateTime(){
  if (WiFi.status() == WL_CONNECTED) {
    timeClient.setTimeOffset(7 * 3600); // 7 giờ * 3600 giây/giờ
    timeClient.begin();
    timeClient.update();
    phut=timeClient.getMinutes();
    gio=timeClient.getHours();
    tuan=timeClient.getDay();
    tht.fillSprite(color);
    dongHo();
    thoiTiet();
  }else{
    lc.fillRoundRect(150, 6, 160 , 24, 2, 0xD90C);
    lc.setTextColor(TFT_BLACK);
    lc.setCursor(160,10);
    lc.setTextSize(2);
    lc.print("Disconected");
    lc.pushSprite(0,25);
    delay(1000);
  }
}
//*********************************

//***< chương trình cài đặt >***
void chuongTrinhLuaChonCaiDat(){
  veHienThiLuaChonThanhNgang(6,20,6,275,20,0xfcb8f8,color,2,chonHienThiCaiDat,luaChonCaiDat);
  delay(1000);
  while(true){
    dongHo();
    currentState1 = digitalRead(nut1);
    if(currentState1 == LOW && lastState1 == HIGH){
      chonHienThiCaiDat++;
      if(chonHienThiCaiDat>5){
        chonHienThiCaiDat=0;
      }
      //lc.fillSprite(background);
      veHienThiLuaChonThanhNgang(6,20,6,275,20,0xfcb8f8,color,2,chonHienThiCaiDat,luaChonCaiDat);
      lc.pushSprite(0,25);
      delay(30);
    }
    lastState1 = currentState1;
    currentState2 = digitalRead(nut2);
    if(currentState2 == LOW && lastState2 == HIGH){
      break;
    }
    lastState2 = currentState2;
    currentState3 = digitalRead(nut3);
    if(currentState3 == LOW && lastState3 == HIGH){
      chonHienThiCaiDat--;
      if(chonHienThiCaiDat<0){
        chonHienThiCaiDat=5;
      }
      //lc.fillSprite(background);
      veHienThiLuaChonThanhNgang(6,20,6,275,20,0xfcb8f8,color,2,chonHienThiCaiDat,luaChonCaiDat);
      lc.pushSprite(0,25);
      delay(30);
    }
    lastState3 = currentState3;
    lc.fillSprite(background);
  }
}

void veHienThiLuaChonThanhNgang(int soThanh,int x,int y,int rong,int cao,uint16_t mauChon,int mauThuong,int coChu,int bienLuaChon,char* text[]){
  lc.fillRoundRect(x-20, 0, 300-x , 172, 1, background);
  for(int i=0;i<soThanh;i++){
    if(i==bienLuaChon){
      lc.fillRoundRect(x, y+i*(cao+4), rong , cao , 2, TFT_WHITE);
      lc.setTextColor(mauChon);
      lc.setCursor(x+10,(cao-16)/2+y+i*(cao+4));
      lc.setTextSize(coChu);
      lc.print(text[i]);
    }else{
      lc.fillRoundRect(x, y+i*(cao+4), rong , cao, 2, mauThuong);
      lc.setTextColor(TFT_BLACK);
      lc.setCursor(x+10,(cao-16)/2+y+i*(cao+4));
      lc.setTextSize(coChu);
      lc.print(text[i]);
    }
  }
  lc.pushSprite(0,25);
}

void chuongTrinhCaiDatChinh(){
  while(true){
    chuongTrinhLuaChonCaiDat();
    if(chonHienThiCaiDat==0){

    }
    if(chonHienThiCaiDat==1){
      
    }
    if(chonHienThiCaiDat==2){
      
    }
    if(chonHienThiCaiDat==3){
      chinhMau();
    }
    if(chonHienThiCaiDat==4){
      
    }
    if(chonHienThiCaiDat==5){
      break;
    }
  }
}

void chinhMau(){
  veChinhMau();
  delay(1000);
  while(true){
    dongHo();
    currentState1 = digitalRead(nut1);
    if(currentState1 == LOW && lastState1 == HIGH){
      chonHienThiMau++;
      if(chonHienThiMau>4){
        chonHienThiMau=0;
      }
      veChinhMau();
    }
    lastState1 = currentState1;
    currentState3 = digitalRead(nut3);
    if(currentState3 == LOW && lastState3 == HIGH){
      chonHienThiMau--;
      if(chonHienThiMau<0){
        chonHienThiMau=4;
      }
      veChinhMau();
    }
    lastState3 = currentState3;
    currentState2 = digitalRead(nut2);
    if(currentState2 == LOW && lastState2 == HIGH){
      if(chonHienThiMau==0){
        bg1[chonHienThiMau+4]=0x26D6;
        lc.drawSmoothArc(80,85,33,35,40,320,bg1[chonHienThiMau+4],TFT_BLACK);
        lc.pushSprite(0,25);
        chua[chonHienThiMau]=red;
        chinhThongSoMau();
        bg1[chonHienThiMau+4]=0xffff;
        lc.drawSmoothArc(80,85,33,35,40,320,bg1[chonHienThiMau+4],TFT_BLACK);
        lc.pushSprite(0,25);
      }
      if(chonHienThiMau==1){
        bg1[chonHienThiMau+3]=0x26D6;
        lc.drawSmoothArc(160,85,33,35,40,320,bg1[chonHienThiMau+3],TFT_BLACK);
        lc.pushSprite(0,25);
        chua[chonHienThiMau]=green;
        chinhThongSoMau();
        bg1[chonHienThiMau+3]=0xffff;
        lc.drawSmoothArc(160,85,33,35,40,320,bg1[chonHienThiMau+3],TFT_BLACK);
        lc.pushSprite(0,25);
      }
      if(chonHienThiMau==2){
        bg1[chonHienThiMau+2]=0x26D6;
        lc.drawSmoothArc(240,85,33,35,40,320,bg1[chonHienThiMau+2],TFT_BLACK);
        lc.pushSprite(0,25);
        chua[chonHienThiMau]=blue;
        chinhThongSoMau();
        bg1[chonHienThiMau+2]=0xffff;
        lc.drawSmoothArc(240,85,33,35,40,320,bg1[chonHienThiMau+2],TFT_BLACK);
        lc.pushSprite(0,25);
      }
      if(chonHienThiMau==3){
        mauMacDinh();
        veChinhMau();
      }
      if(chonHienThiMau==4){
         break;
      }
    }
    lastState2 = currentState2;
  }
}

void mauMacDinh(){
  red=148;
  green=248;
  blue=252;
}

void chinhThongSoMau(){
  delay(1000);
  while(true){
    dongHo();
    currentState1 = digitalRead(nut1);
    if(currentState1 == LOW && chua[chonHienThiMau]<255){
      chua[chonHienThiMau]++;
      if(chonHienThiMau==0){
        red=chua[chonHienThiMau];
      }
      if(chonHienThiMau==1){
        green=chua[chonHienThiMau];
      }
      if(chonHienThiMau==2){
        blue=chua[chonHienThiMau];
      }
      veChinhMau();
    }
    lastState1 = currentState1;
    currentState2 = digitalRead(nut2);
    if(currentState2 == LOW && lastState2 == HIGH){
      break;
    }
    lastState2 = currentState2;
    currentState3 = digitalRead(nut3);
    if(currentState3 == LOW && chua[chonHienThiMau]>=0){
      chua[chonHienThiMau]--;
      if(chonHienThiMau==0){
        red=chua[chonHienThiMau];
      }
      if(chonHienThiMau==1){
        green=chua[chonHienThiMau];
      }
      if(chonHienThiMau==2){
        blue=chua[chonHienThiMau];
      }
      veChinhMau();
    }
    lastState3 = currentState3;

  }
}

void veChinhMau(){
  lc.fillScreen(background);
  lc.setTextColor(TFT_WHITE);
  lc.setTextDatum(MC_DATUM);
  color = tft.color565(red,green,blue);
  lc.fillSmoothRoundRect(60,20 , 200, 20,3, color);
  lc.drawSmoothArc(80,85,33,35,40,320,bg1[chonHienThiMau+4],TFT_BLACK);
  lc.drawSmoothArc(80,85,25,30,40,320,0x3B6D,TFT_BLACK);
  lc.drawSmoothArc(80,85,25,30,40,map(red,0,255,40,320),0xD90C,TFT_BLACK);
  lc.drawSmoothArc(160,85,33,35,40,320,bg1[chonHienThiMau+3],TFT_BLACK);
  lc.drawSmoothArc(160,85,25,30,40,320,0x3B6D,TFT_BLACK);
  lc.drawSmoothArc(160,85,25,30,40,map(green,0,255,40,320),0x26D6,TFT_BLACK);
  lc.drawSmoothArc(240,85,33,35,40,320,bg1[chonHienThiMau+2],TFT_BLACK);
  lc.drawSmoothArc(240,85,25,30,40,320,0x3B6D,TFT_BLACK);
  lc.drawSmoothArc(240,85,25,30,40,map(blue,0,255,40,320),0x1B96,TFT_BLACK);
  lc.setTextSize(2);
  lc.fillSmoothRoundRect(180,120, 120, 20,3, bg1[chonHienThiMau+1]);
  lc.setCursor(200,122);
  lc.print("mac dinh");
  lc.fillSmoothRoundRect(20,120, 120, 20,3, bg1[chonHienThiMau]);
  lc.setCursor(40,122);
  lc.print("tro lai");
  lc.pushSprite(0,25);
}
//*********************************

//***< vòng lặp chính >***
void loop(){
  mainScreen();
}
//*********************************