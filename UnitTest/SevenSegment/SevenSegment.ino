#include <LedControl.h>

///////////////////////////////////////////////////////////////////
// 七段顯示器相關
///////////////////////////////////////////////////////////////////
const byte g_Digis = 6;         //七段顯示器位數;//
const int g_DeviceNum = 1;      //使用的 MAX7219 數量;//
#define PIN_DATA 8              //接MAX7219的P1(左上第一隻腳);//
#define PIN_CLK  9              //接MAX7219的P13;//
#define PIN_CS   7              //接MAX7219的P12;//

LedControl g_Led = LedControl( PIN_DATA, PIN_CLK, PIN_CS, g_DeviceNum );

void setup() 
{
  // put your setup code here, to run once:
    //初始化七段顯示器;//
    g_Led.shutdown(0,false);       //The MAX72XX is in power-saving mode on startup
    g_Led.setIntensity(0,7);      // Set the brightness to default value
    g_Led.clearDisplay(0);         // and clear the display
}

//顯示指定數字;//
void LedPrintNumber( unsigned int Num ) 
{
    //照順序從個位數往高位數點亮;//
    for( int i=0; i<g_Digis; i++ )
    { 
        g_Led.setDigit( 0, i, (byte)(Num%10), false );
        Num = Num / 10;
    }
}

//顯示字串;//
//支援的字元限以下幾種;//
//'0','1','2','3','4','5','6','7','8','9','0',
//'A','b','c','d','E','F','H','L','P',
//'.','-','_',' ' 
void LedPrintString( String Str )
{
    if( Str.length() > g_Digis ) 
        return;        
    
    for( int i=0; i<g_Digis; i++ )
    {
        if( i < Str.length() )
            g_Led.setChar( 0, Str.length()-1-i, Str[i], false );
        else
            g_Led.setChar( 0, i, ' ', false );
    }
}

void loop() 
{
  // put your main code here, to run repeatedly:  
  LedPrintString("111111");
  delay(1000);
  LedPrintString("222222");
  delay(1000);
  LedPrintString("333333");
  delay(1000);
  LedPrintString("444444");
  delay(1000);
  LedPrintString("555555");
  delay(1000);
  LedPrintString("666666");
  delay(1000);
  LedPrintString("777777");
  delay(1000);
  //LedPrintString("888888");
  //delay(1000);
  LedPrintString("999999");
  delay(1000);
  LedPrintString("000000");
  delay(1000);
  

    for( int i=0; i<10000; i++ )
    {
      LedPrintNumber( i );
      delay(100);
    }

}
