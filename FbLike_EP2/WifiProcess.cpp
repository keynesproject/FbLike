#include "WifiProcess.h"
#include "MyUntil.h"

#ifdef DEBUG
#ifndef DBG(message)
#define DBG(message)    DebugSerial.print(message)
#define DBGL(message)   DebugSerial.println(message)
#define DBGW(message)   DebugSerial.write(message)
#endif
#else
#define DBG(message)
#define DBGL(message)
#define DBGW(message)
#endif // DEBUG

//////////////////////////////////////////////////////////
// WifiProcess
//////////////////////////////////////////////////////////
WifiProcess::WifiProcess()
{
    m_IsInitial = false;
}

WifiProcess::~WifiProcess()
{
    
}

bool WifiProcess::Initial( byte PinES, byte PinReset )
{
      if( m_Wifi.Initial( PinES, PinReset ) == false )
      {
          m_IsInitial = false;
          return false;
      }
      
      m_IsInitial = true;
      return true;
}

unsigned long WifiProcess::GetRequestValue( int Type )
{
    return 0;
}

void WifiProcess::Reset()
{
    m_Wifi.HardwareReset();
}

//////////////////////////////////////////////////////////
// WifiServerProcess
// Wifi模組作為Server的程序
//////////////////////////////////////////////////////////
WifiServerProcess::WifiServerProcess()
{
    m_ClientSetSSID = "";
    m_ClientSetPW = "";
    m_ClientSetFBID = "";
}

WifiServerProcess::~WifiServerProcess()
{
    
}

bool WifiServerProcess::Setup( String Ssid, String PW, String FbID )
{   
    if( m_IsInitial == false )
    {
        DBGL( "Wifi don't initialise" );
        return false;
    }
        
    if( !m_Wifi.SetupAp( Ssid, PW, "192,168,100,1", 8080 ) )
    {
        DBGL( "Wifi setup Ap error" );
        return false;
    }

    String IpString  = m_Wifi.GetIP( 0 );
    int Port = m_Wifi.GetPort( 0 );
    DBG( "Wifi AP IP : " );
    DBGL( IpString );
    DBG( "Wifi AP Port : " );
    DBGL( Port );

    m_ServerIP = IpString + ":";
    m_ServerIP.concat( Port );

    //開始傳輸;//
    m_Wifi.StartTransport( true );
    
    return true;
}

int  WifiServerProcess::Process()
{
    String Str;
    if( Uart.available() > 0 )
    {
        //尋找"GET /"字串;//
        if( !Uart.find("GET /") )
        {
            Uart.flush();
            return WIFI_SERVER_WAIT;
        }

        Str = Uart.readStringUntil(0x20);
        if( Uart.find("HTTP/") )
        {
            if( Str.indexOf("Setting") != -1 )
            {
                DBGL( "Receive Setting\n" );
                if( AnalyzeSet( Str ) == true )
                {
                    //取得設定資料,回傳設定成功頁面;//
                    ReplyPageSucess();
            
                    //延遲一段時間確保資料傳送完畢;//
                    delay(1000);

                    return WIFI_CLIENT_SETUPED;
                }
            }
            else if( Str.indexOf("SetApp") != -1 )
            {
                DBGL( "Receive SetApp\n" );
                if( AnalyzeSet( Str ) == true )
                {
                    ReplyOLikeResult( true );

                    return WIFI_CLIENT_SETUPED;
                }
                else
                {
                    ReplyOLikeResult( false );
                }
            }
            else if( Str.indexOf("OLike") != -1 )
            {
                DBGL( "Receive OLike" );

                //回傳Clinet連線資訊;//
                ReplyOLikeLink();
            }
            else if( Str.indexOf("OLike_Receive1") != -1 )
            {
                DBGL( "Receive OLike_Receive1" );
            }
            else if( Str.indexOf("OLike_Receive2") != -1 )
            {
                DBGL( "Receive OLike_Receive2" );
            }
            else
            {
                DBGL( "Receive Index" );
            
                //有Client連結，回傳設定頁面;//
                ReplyPageSetting();
            }
            
            //延遲0.1秒讓資料都傳送完畢;//            
            delay(100);    
            Uart.flush();        
        }
    }
    
    return WIFI_SERVER_WAIT;
}

bool WifiServerProcess::AnalyzeSet( String Data )
{
    //解析資料內容，格式為: Setting?SSID=XXXX&PW=XXXX&FBID=XXXX其中XXXX則為所需設定資料;//
    int Index = Data.indexOf('=');
    int EndIndex = Data.indexOf('&');
    if( Index != -1 && EndIndex != -1 )
    {
        //取得SSID設定字串;//
        m_ClientSetSSID = Data.substring( Index+1, EndIndex );
    
        Index = Data.indexOf( '=', Index+1 );
        EndIndex = Data.indexOf('&', EndIndex+1 );
        if( Index != -1 && EndIndex != -1 )
        {
            //取得PW設定字串;//
            m_ClientSetPW = Data.substring( Index+1, EndIndex );
            
            Index = Data.indexOf( '=', Index+1 );
            if( Index != -1 )
            {
                m_ClientSetFBID = Data.substring( Index+1 ); 
                    
                return true; 
            }
        }                
    }
    return false;
}

void WifiServerProcess::ReplyHead( int HtmlLength )
{
    String Data = F( "HTTP/1.1 200 Ok\r\nContent-Type: text/html\r\nConnection: close\r\n" );
    if( HtmlLength != 0 )
    {
        Data.concat( "Content-Length: " );
        Data.concat( HtmlLength );
        Data.concat( "\r\n");
        DBG( "Html Data Length " );
        DBGL( HtmlLength );
    }
    m_Wifi.SendL( Data );
    /*
    m_Wifi.SendL( "HTTP/1.1 200 Ok" );
    m_Wifi.SendL( "Content-Type: text/html" );
    m_Wifi.SendL( "Connection: close" );
    if( HtmlLength != 0 )
    {
        String Data = "Content-Length: ";
        Data.concat( HtmlLength );
        m_Wifi.SendL( Data );
        DBG( "Html Data Length " );
        DBGL( HtmlLength );
    }
    m_Wifi.SendL( "" );
    */
}

void WifiServerProcess::ReplyOLikeHead( int JsonLength )
{
    String Head = F("HTTP/1.1 200 OK\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: ");
    Head.concat( JsonLength );
    Head.concat( F("\r\nAccess-Control-Allow-Origin:*\r\nConnection:close\r\n") );
    m_Wifi.SendL( Head );    
}

void WifiServerProcess::ReplyOLikeLink()
{    
    //HTTP檔頭;//
    int JsonLength = 14 + 2;
    ReplyOLikeHead( JsonLength );
    
    //m_Wifi.SendL( F("HTTP/1.1 200 OK\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: 16\r\nAccess-Control-Allow-Origin:*\r\nConnection:close\r\n")); 
    /*m_Wifi.SendL( "HTTP/1.1 200 OK" );
    m_Wifi.SendL( "Content-Type: application/json; charset=utf-8" );
    m_Wifi.SendL( "Content-Length: 16" );
    m_Wifi.SendL( "Access-Control-Allow-Origin:*" );
    m_Wifi.SendL( "Connection: close" );
    m_Wifi.SendL( "" );
    */
    m_Wifi.SendL(F( "\{\"id\":\"OLike\"\}" ));
}

void WifiServerProcess::ReplyOLikeResult( bool IsSucess )
{
    int JsonLength;
    if( IsSucess )
    {
        JsonLength = 11 + 2;
        ReplyOLikeHead( JsonLength );   

        m_Wifi.SendL(F( "\{\"id\":\"go\"\}" ));
    }
    else
    {
        JsonLength = 14 + 2;
        ReplyOLikeHead( JsonLength );       
        
        m_Wifi.SendL(F( "\{\"id\":\"error\"\}" )); 
    }
}

void WifiServerProcess::ReplyPageSetting( )
{
    int HtmlLength = 567 + 2 + m_ServerIP.length();
    ReplyHead( HtmlLength );

    m_Wifi.Send( F("<!DOCTYPE html><html><head><meta charset=\"UTF-8\" /><title>Facebook Likes Setting</title></head><body><form id=\"signup\" align=\"center\" method=\"GET\" action=\"http://"));
    m_Wifi.Send( m_ServerIP );
    m_Wifi.Send( F("/Setting\"><label>SSID:<br></label><input type=\"text\" id=\"SSID\" name=\"SSID\" MAXLENGTH=24 SIZE=\"24\" /><br><br><label>Password:<br></label>"));
    m_Wifi.Send( F("<input type=\"text\" id=\"PW\" name=\"PW\" MAXLENGTH=24 SIZE=\"24\"/><br><br><label>FaceBook ID:<br></label><input type=\"text\" id=\"FBID\" name=\"FBID\" MAXLENGTH=24 SIZE=\"24\" /><br><br>"));
    m_Wifi.Send( F("<input type=\"submit\" value=\"Apply\" /><input TYPE=\"RESET\" VALUE=\"Reset\" /></form></body></html>\r\n\r\n"));     
}

void WifiServerProcess::ReplyPageSucess()
{
    int DataLength =  385 + 2 + m_ClientSetSSID.length() + m_ClientSetPW.length() + m_ClientSetFBID.length();
    ReplyHead( DataLength );

    m_Wifi.Send( F("<!DOCTYPE html><html><head><meta charset=\"UTF-8\" /><title>Facebook Likes Setting Success</title></head><body><table border=\"0\" align=\"center\" width='240'><tbody><tr><td align=\"left\">"));
    
    String Str = "SSID: " + m_ClientSetSSID + "<br><br>";
    m_Wifi.Send( Str );
        
    Str = "Password: " + m_ClientSetPW + "<br><br>";
    m_Wifi.Send( Str );
        
    Str = "FaceBook ID: " + m_ClientSetFBID + "<br><br>";
    m_Wifi.Send( Str );
    m_Wifi.Send( F("</td></tr><tr><td align=\"center\" ><font color=\"#ff0000\"><br>Setting Success!<br>Now System reboot!</font></td></tr></tbody></table></body></html>\r\n\r\n"));
}

unsigned long WifiServerProcess::GetRequestValue( int Type )
{    
    return 0;
}

String  WifiServerProcess::GetRequestString( int Type )
{
      switch( Type )
      {
          case WIFI_REQ_SSID:
              return m_ClientSetSSID;
          break;
          
          case WIFI_REQ_PW:
              return m_ClientSetPW;
          break;
          
          case WIFI_REQ_FBID:
              return m_ClientSetFBID;
          break;
      }
      
      return "";
}


//////////////////////////////////////////////////////////
// WifiClientProcess
// //Wifi模組作為Client的程序;//
//////////////////////////////////////////////////////////
WifiClientProcess::WifiClientProcess()
{
    m_RequestValue = 0;
    m_FbField = F("fan_count");
    m_FbHost = F("api-read.facebook.com");
    m_FbPort = 80;
    m_WifiRequestErrCount = 0;
    m_FbDataErrCount = 0;
}

WifiClientProcess::~WifiClientProcess()
{
    
}

bool WifiClientProcess::Setup( String Ssid, String PW, String FbID )
{
    if( m_IsInitial == false )
    {
        DBGL( "Wifi don't initialise" );
        return false;
    }

    if( !m_Wifi.SetupSta( Ssid, PW, m_FbHost, m_FbPort ) )
    {
        DBGL( "Wifi setup client error!" );
        return false;
    }

    m_FbID = FbID;

    //開始傳輸;//
    m_Wifi.StartTransport( true );
    
    return true;
}

int  WifiClientProcess::Process()
{   
    //每3秒請求一次資料;//
    delay( 3000 );
    
    FbRequest();
    
    unsigned long Start = millis();
    while( millis() - Start < 3000 ) 
    {
        while( Uart.available() > 0 )
        {     
            //表示有資料從WIFI模組回傳回來,所以將連線請求錯誤次數歸0;//
            m_WifiRequestErrCount = 0;
            
            //尋找指定欄位;//
            char Temp[32];
            short Num = 0;
            strcpy( Temp, m_FbField.c_str() );
            Temp[ m_FbField.length() ] = '>';
            Temp[ m_FbField.length()+1 ] = 0;
            if( !Uart.find( Temp ) )
            {
                break;
            }
            
            //表示有找到指定欄位,分析數值;//
            m_RequestStr = "";
            m_RequestStr = Uart.readStringUntil('<');
            if( m_RequestStr.length() != 0 )
            {
                m_RequestStr.replace( "<", "");
                m_RequestStr.replace( ">", "");
                DBGL( m_RequestStr );
                //分析數值;//
                m_RequestValue = 0;
                for( int i=m_RequestStr.length()-1; i>=0; i--)
                {
                    Num =  m_RequestStr[ m_RequestStr.length() - i -1 ];  
                    Num -= 48;               
                    m_RequestValue +=  ( Num * Pow( 10, i ) );
                }
                DBG( "Facebook Request Num :" );
                DBG( m_RequestValue );
 
               //將剩餘的資料讀取完畢;//
               while( Uart.available() > 0 )
               {
                   Uart.read();
               }
               Uart.flush();

               //表示有請求到正確資料,讀取資料錯誤計數歸0;//
               m_FbDataErrCount = 0;
               
               return WIFI_REQ_SUCESS;
            }
        }
    }

    //計算WIFI未請求到資料的次數,當此次數大於3次認定為WIFI已經中斷與AP的連線;//
    m_WifiRequestErrCount++;
    if( m_WifiRequestErrCount > 3 )
    {
        //指定數值的原因為防止數值溢位;//
        m_WifiRequestErrCount = 4;
        return WIFI_ERR_NO_LINK;
    }

    m_FbDataErrCount++;
    if( m_FbDataErrCount > 3 )
    { 
        //指定數值的原因為防止數值溢位;//
        m_FbDataErrCount = 4;
        return WIFI_ERR_NO_DATA;
    }

    return WIFI_NONE;
}

unsigned long  WifiClientProcess::GetRequestValue( int Type )
{
    if( Type != WIFI_REQ_FB_FIELD_NUM )
        return 0;
    
    return m_RequestValue;
}

String  WifiClientProcess::GetRequestString( int Type )
{
  if( Type != WIFI_REQ_FB_FIELD_NUM )
        return "";
        
    return m_RequestStr;  
}

void WifiClientProcess::FbRequest()
{     
    DBG( "\nFb Request!\n" );
/*
    這邊為完整的 HTTP REQUEST 字串
    GET /restserver.php?format=xml&method=fql.multiquery&pretty=0&queries=%7B%22page_info%22%3A%22select%20name%2Cfan_count%20from%20page%20where%20page_id%20IN%20(764858306921449)%22%7D HTTP/1.1
    Host: api-read.facebook.com
    Connection: keep-alive
*/
    m_Wifi.Send( F("GET /restserver.php?format=xml&method=fql.multiquery&pretty=0&queries=%7B%22page_info%22%3A%22") );
    m_Wifi.Send( F("select%20name%2Cfan_count%20from%20page%20where%20page_id%20IN%20(") );
    m_Wifi.Send( m_FbID );
    m_Wifi.SendL( F(")%22%7D HTTP/1.1") );
    m_Wifi.SendL( F("Host: api-read.facebook.com") );
    m_Wifi.Send( F("Connection: keep-alive\r\n\r\n") );   //結尾傳送需要有兩個換行字串;//
}

