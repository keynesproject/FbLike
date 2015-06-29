////////////////////////////////////////////////////////////////////
//    FB like 計數
//    
//    此程式會向 http://graph.facebook.com 請求json檔案，
//    並且解析出Like的數值。
//    之後在顯示在七段顯示器上
////////////////////////////////////////////////////////////////////

#include <LedControl.h>
#include <SoftwareSerial.h>
#include <Bounce.h>
#include "WifiProcess.h"
#include "MyWIFI.h"
#include "MyEEPROM.h"

#ifdef DEBUG
#ifndef DBG(message)
#define DBG(message)    DebugSerial.print(message)
#define DBGW(message)   DebugSerial.write(message)
#endif
#else
#define DBG(message)
#define DBGW(message)
#endif // DEBUG

///////////////////////////////////////////////////////////////////
// EEPROM管理
///////////////////////////////////////////////////////////////////
MyEeprom g_Eeprom;

///////////////////////////////////////////////////////////////////
// 按鍵相關
///////////////////////////////////////////////////////////////////
const uint8_t ORIENTATION_PIN = 2;
Bounce g_BtnReset = Bounce( ORIENTATION_PIN, 50 );

///////////////////////////////////////////////////////////////////
// 七段顯示器相關
///////////////////////////////////////////////////////////////////
byte g_NetState = WIFI_DEFAULT; //七段顯示器顯示的狀態;//
const byte g_Digis = 5;         //七段顯示器位數;//
const int g_DeviceNum = 1;      //使用的 MAX7219 數量;//
#define PIN_DATA 11              //接MAX7219的P1(左上第一隻腳);//
#define PIN_CLK  12              //接MAX7219的P13;//
#define PIN_CS   13              //接MAX7219的P12;//

LedControl g_Led = LedControl( PIN_DATA, PIN_CLK, PIN_CS, g_DeviceNum );

///////////////////////////////////////////////////////////////////
// 網路相關
///////////////////////////////////////////////////////////////////
WifiProcess *g_WifiPro = 0;
WifiServerProcess g_WifiServer;
WifiClientProcess g_WifiClient;

void SetEEPROM()
{
/*
   g_Eeprom.ClearEEPROM();
   while(1)
   {};
*/
}

void Set7Seg()
{
    //初始化七段顯示器;//
    g_Led.shutdown(0,false);       //The MAX72XX is in power-saving mode on startup
    g_Led.setIntensity(0,15);      // Set the brightness to default value
    g_Led.clearDisplay(0);         // and clear the display
    LedPrintString(F("-_-_-"));
}

bool SetWifi()
{ 
    //通電後讓其他模組先運作再開始主程式;//
    delay(1000);  
  
    bool IsWifiSetup = false;
    
    //取得現在為Server或Client狀態;//
    byte BordState = g_Eeprom.GetBordState();
    //BordState = 0;
    if( BordState == 0 || BordState == 1 )
    {
        //g_WifiPro = new WifiServerProcess();
        g_WifiPro = &g_WifiServer;        
       
        IsWifiSetup = g_WifiPro->Setup( F("FB_LIKE"), F("1234567890") );
    }
    else
    {
        //g_WifiPro = new WifiClientProcess();
        g_WifiPro = &g_WifiClient;
        //取得FB ID;// 
        String FbQueryID = "/" + g_Eeprom.GetFbId();
        //String FbQueryID = "/itri.tw";
        
        //取得網路設定資訊;//
        String Ssid = g_Eeprom.GetSsid();
        String PW = g_Eeprom.GetPassWord();
        //String Ssid = "dlink";
        //String PW = "1234567890";
        
        //初始化WIFI連線;//
        IsWifiSetup = g_WifiPro->Setup( Ssid, PW,  FbQueryID );
    }
     
    if(!IsWifiSetup)
    {
        DBG("Wifi Init error\n");
        g_NetState = WIFI_ERR_SETUP;
        return false;
    } 
   
    return true;
}

void setup() 
{ 
    //初始化EEPROM;//
    SetEEPROM();      
    
    //初始化按鈕;//
    pinMode( ORIENTATION_PIN, INPUT );
    digitalWrite( ORIENTATION_PIN, LOW );
    
    //初始化7段顯示器;// 
    Set7Seg();
    
    //初始化wifi;//
    if( !SetWifi() )
    {
        Process7Seg( g_NetState );
    } 
}

void loop() 
{
    ProcessBtn();
    
    Process7Seg( g_NetState );
    
    ProcessNet();
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
    if( Str.length() > 5 )
        return;        
    
    for( int i=0; i<g_Digis; i++ )
    {
        if( i < Str.length() )
            g_Led.setChar( 0, Str.length()-1-i, Str[i], false );
        else
            g_Led.setChar( 0, i, ' ', false );
    }
}

void Process7Seg( int State )
{
    switch( State )
    {
    //初始化狀態;//
    case WIFI_DEFAULT:
        LedPrintString(F("_____"));
        break;
    
    //伺服器狀態等待設定;//           
    case WIFI_SERVER_WAIT:
        LedPrintString(F("SE UP"));
        break;
        
    //Clent回傳資料設定完畢;//           
    case WIFI_CLIENT_SETUPED:
        LedPrintString(F("  OFF"));
        break;
    
    //網路連線失敗;//
    case WIFI_ERR_SETUP:
        LedPrintString(F("EEEEE"));
        break;
        
    //FB數字顯示;//  
    case WIFI_REQ_SUCESS:
    case WIFI_ERR_NO_DATA:
        LedPrintNumber( g_WifiPro->GetRequestValue( WIFI_REQ_FB_FIELD_NUM ) );
        break;
      
    //FB請求錯誤;//  
    case WIFI_ERR_FB_REQ:
        LedPrintString(F("Fb EE"));
        break;
    }      
}

void ProcessNet()
{
    g_NetState = g_WifiPro->Process();
    if( g_NetState == WIFI_CLIENT_SETUPED )
    {
        //更改;//
        Process7Seg( WIFI_CLIENT_SETUPED );
      
        //取得設定資訊;//
        String Str = g_WifiPro->GetRequestString( WIFI_REQ_SSID );
        if( Str.length() > 0 )
        {
            g_Eeprom.SetSsid( Str );
            Str = g_WifiPro->GetRequestString( WIFI_REQ_PW );
            if( Str.length() > 0 )
            {
                g_Eeprom.SetPassWord( Str );
                Str = g_WifiPro->GetRequestString( WIFI_REQ_FBID );
                if( Str.length() > 0 )
                {
                    g_Eeprom.SetFbId( Str );
                    
                    //取得完整設定資訊，設定板子狀態;//
                    g_Eeprom.SetBoardState(2);
                    
                    //延遲一段時間確保資料寫入;//
                    delay(1000);
                    
                    //此行相當於按下Reset鍵;//
                    asm volatile ("  jmp 0");  
                }
            } 
        }
    }
}

void ProcessBtn()
{
    g_BtnReset.update();
       
    // has our orientation changed for more the 5 second
    while( g_BtnReset.read() == HIGH )
    {
        g_BtnReset.update();
        
        if ( g_BtnReset.duration() > 3000 ) 
        {  
                LedPrintString(F("  OFF"));
                
                //恢復EEPROM為原始設定;//
                g_Eeprom.ClearEEPROM();
                
                //延遲一段時間確保資料寫入;//
                delay(1000);
                        
                //此行相當於按下Reset鍵;//
                asm volatile ("  jmp 0");  
        }
    }
}


