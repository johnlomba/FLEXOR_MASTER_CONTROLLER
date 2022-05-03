#include <Wire.h>

# define I2C_SLAVE1_ADDRESS 11
# define I2C_SLAVE2_ADDRESS 12

#define PAYLOAD_SIZE 2

int n=0;

//pin usage as follow:
//                  CS  DC/RS  RESET  SDI/MOSI  SDO/MISO  SCK  LED    VCC     GND    
//Arduino Mega2560  A5   A3     A4      51        50      52   A0   5V/3.3V   GND
//                 T_IRQ  T_DO  T_DIN  T_CS  T_CLK
//Arduino Mega2560  49     47    48     45    46

//Remember to set the pins to suit your display module!

/*********************************************************************************
* @attention
*
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
* TIME. AS A RESULT, QD electronic SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE 
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
**********************************************************************************/

#include <LCDWIKI_GUI.h> //Core graphics library
#include <LCDWIKI_SPI.h> //Hardware-specific library
#include <LCDWIKI_TOUCH.h> //touch screen library

//paramters define
#define MODEL ILI9341
#define CS   A5    
#define CD   A3
#define RST  A4
#define LED  A0   //if you don't need to control the LED pin,you should set it to -1 and set it to 3.3V

//touch screen paramters define
#define TCS   45
#define TCLK  46
#define TDOUT 47
#define TDIN  48
#define TIRQ  49

                             /*  r     g    b */
#define BLACK        0x0000  /*   0,   0,   0 */
#define BLUE         0x001F  /*   0,   0, 255 */
#define RED          0xF800  /* 255,   0,   0 */
#define GREEN        0x07E0  /*   0, 255,   0 */
#define CYAN         0x07FF  /*   0, 255, 255 */
#define MAGENTA      0xF81F  /* 255,   0, 255 */
#define YELLOW       0xFFE0  /* 255, 255,   0 */
#define WHITE        0xFFFF  /* 255, 255, 255 */
#define NAVY         0x000F  /*   0,   0, 128 */
#define DARKGREEN    0x03E0  /*   0, 128,   0 */
#define DARKCYAN     0x03EF  /*   0, 128, 128 */
#define MAROON       0x7800  /* 128,   0,   0 */
#define PURPLE       0x780F  /* 128,   0, 128 */
#define OLIVE        0x7BE0  /* 128, 128,   0 */
#define LIGHTGREY    0xC618  /* 192, 192, 192 */
#define DARKGREY     0x7BEF  /* 128, 128, 128 */
#define ORANGE       0xFD20  /* 255, 165,   0 */
#define GREENYELLOW  0xAFE5  /* 173, 255,  47 */
#define PINK         0xF81F  /* 255,   0, 255 */


//the definiens of hardware spi mode as follow:
//if the IC model is known or the modules is unreadable,you can use this constructed function
LCDWIKI_SPI mylcd(MODEL,CS,CD,RST,LED); //model,cs,dc,reset,led
//if the IC model is not known and the modules is readable,you can use this constructed function
//LCDWIKI_SPI my_lcd(240,320,CS,CD,RST,LED); //model,cs,dc,reset,led

//the definiens of touch mode as follow:
LCDWIKI_TOUCH mytouch(TCS,TCLK,TDOUT,TDIN,TIRQ); //tcs,tclk,tdout,tdin,tirq

uint16_t px, py;

uint8_t buttonsCount = 7;

long inputTimer = 0;
long inputTimerStart = 0;
long inputTimerDelay = 200;
bool waitingForDelayTimer = false;

typedef struct _button_info
{
   uint8_t button_name[10];
     uint8_t button_name_size;
     uint16_t button_name_colour;
     uint16_t button_colour;
     uint16_t button_x;
     uint16_t button_y;
     uint16_t button_width;
     uint16_t button_height;
          
}button_info;

button_info myButtons[7] = 
{
  "STOP",2,BLACK,RED,0,0,90,40,
  "Start",2,BLACK,GREEN,0,50,90,40,
  "Home",2,BLACK,ORANGE,0,100,90,40,
  "Travel+",2,BLACK,CYAN,0,150,90,40,
  "Travel-",2,BLACK,PURPLE,0,200,90,40,
  "Speed+",2,BLACK,CYAN,100,150,90,40,
  "Speed-",2,BLACK,PURPLE ,100,200,90,40  
};

long delay_Micros = 1500; // Set value

long currentMicros = 0; long previousMicros = 0;

long currentTravel = 1000;
long currentSpeed = 1000;

bool isOn = true;

void Draw_UI(void)
{
  mylcd.Fill_Screen(BLACK);
  
  for(int i = 0; i < buttonsCount; i++){
    Draw_Button(myButtons[i]);
  }
  for(int i = 0; i < buttonsCount; i++){
    show_string(myButtons[i].button_name, myButtons[i].button_x, myButtons[i].button_y, myButtons[i].button_name_size,  myButtons[i].button_name_colour, 1);
  }
  
  DrawSpeed();
  DrawTravel();
}

void DrawSpeed(void){
    mylcd.Set_Draw_color(BLACK);
    mylcd.Fill_Rectangle(100, 250, 200, 290);
    mylcd.Set_Text_Mode(1);
    mylcd.Set_Text_Size(3);
    mylcd.Set_Text_colour(RED);
    mylcd.Print_String(String(currentSpeed),105,250);
}

void DrawTravel(void){
    mylcd.Set_Draw_color(BLACK);
    mylcd.Fill_Rectangle(0, 250, 105, 290);
    mylcd.Set_Text_Mode(1);
    mylcd.Set_Text_Size(3);
    mylcd.Set_Text_colour(RED);
    mylcd.Print_String(String(currentTravel),5,250);
}

void Draw_Button(button_info button){
  mylcd.Set_Draw_color(button.button_colour);
  mylcd.Fill_Rectangle(button.button_x,button.button_y,button.button_x + button.button_width,button.button_y + button.button_height);
}

void show_string(uint8_t *str,int16_t x,int16_t y,uint8_t csize,uint16_t fc,boolean mode)
{
    mylcd.Set_Text_Mode(mode);
    mylcd.Set_Text_Size(csize);
    mylcd.Set_Text_colour(fc);
    mylcd.Print_String(str,x,y);
}

boolean is_pressed(int16_t x1,int16_t y1,int16_t x2,int16_t y2,int16_t _px,int16_t _py)
{
    if((_px > x1 && _px < x2) && (_py > y1 && _py < y2))
    {
        return true;  
    } 
    else
    {
        return false;  
    }
 }

void setup()

{
  Wire.begin();   
    
  mylcd.Init_LCD();
  
  mytouch.TP_Init(mylcd.Get_Rotation(),mylcd.Get_Display_Width(),mylcd.Get_Display_Height()); 
  mytouch.TP_Set_Rotation(3);

  Draw_UI();

  Serial.begin(9600); // open the serial port at 9600 bps:
}


void loop()
{
  if(!waitingForDelayTimer)
  {
     mytouch.TP_Scan(0);
    
    if (mytouch.TP_Get_State()& TP_PRES_DOWN) 
    {
      int _btn = Button_Pressed(mytouch.x, mytouch.y);
      Button_Event_Update(_btn);
      Send_Command(_btn);
    }
  }
  else
  {
    inputTimer = millis();
    if(inputTimer - inputTimerStart > inputTimerDelay){ waitingForDelayTimer = false; }
  }
}

void Button_Event_Update(int _button)
{
  switch(_button)
  {
    case 3: currentTravel += 50; DrawTravel(); break;
    case 4: currentTravel -= 50; DrawTravel(); break;
    case 5: currentSpeed += 100; DrawSpeed(); break;
    case 6: currentSpeed -= 100; DrawSpeed(); break;
  }
}

void Send_Command(int _data)
{
  Wire.beginTransmission(I2C_SLAVE1_ADDRESS); 
  Wire.write(_data);            
  Wire.endTransmission();  
}


int Button_Pressed(int _px, int _py)
{
  for(int i = 0; i < buttonsCount; i++)
  {
    if(is_pressed(myButtons[i].button_x, myButtons[i].button_y, myButtons[i].button_x + myButtons[i].button_width, myButtons[i].button_y + myButtons[i].button_height, _px, _py))
    {               
      waitingForDelayTimer = true;
      inputTimerStart = millis();
      return i;
    }
  }

  return -1;
}
