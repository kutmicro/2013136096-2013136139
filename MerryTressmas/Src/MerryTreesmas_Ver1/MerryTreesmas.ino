/*
 # This sample codes is for testing the LCD12864 shield.
 # Editor : Mickey
 # Date   : 2013.11.27
 # Ver    : 0.1
 # Product: LCD12864 shield
 # SKU    : DFR0287
*/


#include "U8glib.h"
uint8_t draw_state = 0;
// setup u8g object, please remove comment from one of the following constructor calls
// IMPORTANT NOTE: The complete list of supported devices is here: http://code.google.com/p/u8glib/wiki/device

U8GLIB_NHD_C12864 u8g(13, 11, 10, 9, 8);	// SPI Com: SCK = 13, MOSI = 11, CS = 10, A0 = 9, RST = 8

#define KEY_NONE 0
#define KEY_PREV 1
#define KEY_NEXT 2
#define KEY_SELECT 3
#define KEY_BACK 4
int abc=0;
bool case1=false;
bool case2=false;
bool case3=false;
bool case4=false;
uint8_t uiKeyCodeFirst = KEY_NONE;
uint8_t uiKeyCodeSecond = KEY_NONE;
uint8_t uiKeyCode = KEY_NONE;

int adc_key_in;
int key=-1;
int oldkey=-1;


// Convert ADC value to key number
//         4
//         |
//   0 --  1 -- 3
//         |
//         2

//-->180도 회전하면
//            prev(1)
//               |
//   back(4) -- select(3) -- none(0)
//               |
//             next(2)
int get_key(unsigned int input)
{   
    if (input < 100) return 0;//오른쪽
    else  if (input < 300) return 1;//위
    else  if (input < 500) return 2;//아래
    else  if (input < 700) return 3;//중앙
    else  if (input < 900) return 4; //왼쪽
    else  return -1;//나머지
}

void uiStep(void) {
  
  adc_key_in = analogRead(0);    // read the value from the sensor  
  key = get_key(adc_key_in);	 // convert into key press	
  Serial.print(key);
  if (key != oldkey)	  // if keypress is detected
   {
    delay(50);		// wait for debounce time
    adc_key_in = analogRead(0);    // read the value from the sensor  
    key = get_key(adc_key_in);	   // convert into key press
    if (key != oldkey)				
    {			
      oldkey = key;
      if (key >=0){
             //Serial.println(key);
             if ( key == 0 )
               uiKeyCodeFirst = KEY_BACK;
             else if ( key == 1 )
               uiKeyCodeFirst = KEY_SELECT;
             else if ( key == 2 )
               uiKeyCodeFirst = KEY_NEXT;
             else if ( key == 4 )
               uiKeyCodeFirst = KEY_PREV;
             else 
               uiKeyCodeFirst = KEY_NONE;
  
             uiKeyCode = uiKeyCodeFirst;           
      }
    }
  }
 delay(100);
}


#define MENU_ITEMS 4
char *menu_strings[MENU_ITEMS] = { "Hello World", "manual", "Start Game", "exit" };

uint8_t menu_current = 0;
uint8_t menu_redraw_required = 0;
uint8_t last_key_code = KEY_NONE;


void drawMenu(void) {
  uint8_t i, h;
  u8g_uint_t w, d;

  u8g.setFont(u8g_font_6x12);//4x6 5x7 5x8 6x10 6x12 6x13
  u8g.setFontRefHeightText();
  u8g.setFontPosTop();
  
  h = u8g.getFontAscent()-u8g.getFontDescent();
  w = u8g.getWidth();
  for( i = 0; i < MENU_ITEMS; i++ ) {
    d = (w-u8g.getStrWidth(menu_strings[i]))/2;
    u8g.setDefaultForegroundColor();
    if ( i == menu_current ) {  
      u8g.drawBox(0, i*h+1, w, h);
      u8g.setDefaultBackgroundColor();
    }
    u8g.drawStr(d, i*h+1, menu_strings[i]);
  }
}
void escape(void) {
  switch( uiKeyCode ){
  case KEY_BACK:
  draw_state = 0;
  abc=0;
  break;
  }
}
void mydd(void){
   u8g_prepare();
   u8g.drawStr( 0, 0, "how to play");
   u8g.drawStr( 0, 10, "play like this");
   u8g.drawStr( 0, 20, "Have Fun");
}
void updateMenu(void) 
{
  switch ( uiKeyCode ) {
    case KEY_NEXT:
      menu_current++;
      if ( menu_current >= MENU_ITEMS )menu_current = 0;
      menu_redraw_required = 1;
      break;
    case KEY_PREV:
      if ( menu_current == 0 )menu_current = MENU_ITEMS;
      menu_current--;
      menu_redraw_required = 1;
      break;
    case KEY_SELECT:
    if(menu_current!=0)
      abc=1;
    break;
  }
  uiKeyCode = KEY_NONE;
}


void u8g_prepare(void) {
  u8g.setFont(u8g_font_6x10);
  u8g.setFontRefHeightExtendedText();
  u8g.setDefaultForegroundColor();
  u8g.setFontPosTop();
}

void u8g_box_frame(uint8_t a) {
  u8g.drawStr( 0, 0, "drawBox");
  u8g.drawBox(5,10,20,10);
  u8g.drawBox(10+a,15,30,7);
  u8g.drawStr( 0, 30, "drawFrame");
  u8g.drawFrame(5,10+30,20,10);
  u8g.drawFrame(10+a,15+30,30,7);
}

void u8g_disc_circle(uint8_t a) {
  u8g.drawStr( 0, 0, "drawDisc");
  u8g.drawDisc(10,18,9);
  u8g.drawDisc(24+a,16,7);
  u8g.drawStr( 0, 30, "drawCircle");
  u8g.drawCircle(10,18+30,9);
  u8g.drawCircle(24+a,16+30,7);
}

void u8g_r_frame(uint8_t a) {
  u8g.drawStr( 0, 0, "drawRFrame/Box");
  u8g.drawRFrame(5, 10,40,30, a+1);
  u8g.drawRBox(50, 10,25,40, a+1);
}

void u8g_string(uint8_t a) {
  u8g.drawStr(30+a,31, " how to play");
  u8g.drawStr90(30,31+a, " fffffffff");
  u8g.drawStr180(30-a,31, " 1fffffffff");
  u8g.drawStr270(30,31-a, " ssssssssssssssss");
}

void u8g_line(uint8_t a) {
  u8g.drawStr( 0, 0, "drawLine");
  u8g.drawLine(7+a, 10, 40, 55);
  u8g.drawLine(7+a*2, 10, 60, 55);
  u8g.drawLine(7+a*3, 10, 80, 55);
  u8g.drawLine(7+a*4, 10, 100, 55);
}

void u8g_ascii_1() {
  char s[2] = " ";
  uint8_t x, y;
  u8g.drawStr( 0, 0, "ASCII page 1");
  for( y = 0; y < 6; y++ ) {
    for( x = 0; x < 16; x++ ) {
      s[0] = y*16 + x + 32;
      u8g.drawStr(x*7, y*10+10, s);
    }
  }
}

void u8g_ascii_2() {
  char s[2] = " ";
  uint8_t x, y;
  u8g.drawStr( 0, 0, "ASCII page 2");
  for( y = 0; y < 6; y++ ) {
    for( x = 0; x < 16; x++ ) {
      s[0] = y*16 + x + 160;
      u8g.drawStr(x*7, y*10+10, s);
    }
  }
}

void u8g_extra_page(uint8_t a)
{
  if ( u8g.getMode() == U8G_MODE_HICOLOR || u8g.getMode() == U8G_MODE_R3G3B2) {
    /* draw background (area is 128x128) */
    u8g_uint_t r, g, b;
    b = a << 5;
    for( g = 0; g < 64; g++ )
    {
      for( r = 0; r < 64; r++ )
      {
  u8g.setRGB(r<<2, g<<2, b );
  u8g.drawPixel(g, r);
      }
    }
    u8g.setRGB(255,255,255);
    u8g.drawStr( 66, 0, "Color Page");
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT )
  {
    u8g.drawStr( 66, 0, "Gray Level");
    u8g.setColorIndex(1);
    u8g.drawBox(0, 4, 64, 32);    
    u8g.drawBox(70, 20, 4, 12);
    u8g.setColorIndex(2);
    u8g.drawBox(0+1*a, 4+1*a, 64-2*a, 32-2*a);
    u8g.drawBox(74, 20, 4, 12);
    u8g.setColorIndex(3);
    u8g.drawBox(0+2*a, 4+2*a, 64-4*a, 32-4*a);
    u8g.drawBox(78, 20, 4, 12);
    
  }
  else
  {
    u8g.drawStr( 0, 12, "setScale2x2");
    u8g.setScale2x2();
    u8g.drawStr( 0, 6+a, "setScale2x2");
    u8g.undoScale();
  }
}



void draw(void) {
  u8g_prepare();
  switch(draw_state >> 3) {
    case 0: u8g_box_frame(draw_state&7); break;
    case 1: u8g_disc_circle(draw_state&7); break;
    case 2: u8g_r_frame(draw_state&7); break;
    case 3: u8g_string(draw_state&7); break;
    case 4: u8g_line(draw_state&7); break;
    case 5: u8g_ascii_1(); break;
    case 6: u8g_ascii_2(); break;
    case 7: u8g_extra_page(draw_state&7); break;
  }
}




void setup() {
  
  u8g.setRot180();// rotate screen, if required
  menu_redraw_required = 1;     // force initial redraw
  //Serial.begin(9600);
    u8g.setContrast(0);
  //pinMode(13, OUTPUT);           
 // digitalWrite(13, HIGH);  
}

void loop() {  

if(abc==0 ){//메뉴!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    uiStep();                                // check for key press
  updateMenu();                            // update menu bar    
  
    u8g.firstPage();
    do  {
      drawMenu();
    } while( u8g.nextPage() );
  }
if(abc==1&&menu_current==1){//첫번째 메뉴얼!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
          uiStep();                                // check for key press
        escape();
             u8g.firstPage();  
          do {
            mydd();
  } while( u8g.nextPage() );
}
  
  if(abc==1&&menu_current==2){//게임 실행!!!!!!!!!!!!!!!!!!!!!!!!
        uiStep();                                // check for key press
        escape();
      u8g.firstPage();  
  do {
    draw();
  } while( u8g.nextPage() );
  
  // increase the state
  draw_state++;
  if ( draw_state >= 8*8 )
    draw_state = 0;
  }
    if(abc==1&&menu_current==3){//종료!!!!!!!!!!!!!!!!!!!!!!!!
        uiStep();                                // check for key press
        escape();
      u8g.firstPage();  
  do {
      u8g_prepare();
      u8g.drawStr( 0, 20, "thank you to play");
  } while( u8g.nextPage() );
  }
}
/*
 if(조도 크면 >> ㅇ)
 이런그림(해 밤)
 원래그림
 * 
 * else
 * 밤
 * 원래
 * */
 */
