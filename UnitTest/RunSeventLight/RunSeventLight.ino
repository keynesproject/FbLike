#include <Bounce.h>
#include <LedControl.h>

#include "MySevenSegment.h"

///////////////////////////////////////////////////////////////////
// 按鍵及RESET相關
///////////////////////////////////////////////////////////////////
#define PIN_ORIENTATION 13
Bounce g_CountBtn = Bounce( PIN_ORIENTATION, 50 );

///////////////////////////////////////////////////////////////////
// 七段顯示器相關
///////////////////////////////////////////////////////////////////
const byte g_Digis = 6;         //七段顯示器位數;//
const int g_DeviceNum = 1;      //使用的 MAX7219 數量;//
#define PIN_DATA 8              //接MAX7219的P1(左上第一隻腳);//
#define PIN_CLK  9              //接MAX7219的P13;//
#define PIN_CS   7              //接MAX7219的P12;//

MySevenSegment g_Led = MySevenSegment( PIN_DATA, PIN_CLK, PIN_CS, g_Digis, g_DeviceNum );

unsigned long g_TestNum = 0;

void setup() 
{    
    Serial.begin(9600);  
}

void ProcessBtn()
{
    g_CountBtn.update();
       
    // has our orientation changed for more the 3 second
    while( g_CountBtn.read() == HIGH )
    {
        g_CountBtn.update();
        
        if ( g_CountBtn.duration() > 100 ) 
        {  
            g_TestNum += 98;
            break;
            //g_TestNum++;
        }
    }
}

void loop() 
{
    ProcessBtn();

    Process7Seg();
}

void Process7Seg( )
{       
    g_Led.Effect( 2, g_TestNum );
}

