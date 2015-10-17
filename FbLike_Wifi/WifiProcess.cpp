#include "WifiProcess.h"

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
}

WifiProcess::~WifiProcess()
{
    
}

int GetRequestValue( int Type )
{
      return 0;
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
    m_Wifi.begin();    
    if( !m_Wifi.Initialize( AP, Ssid, PW ) )
    {
        DBGL( "Wifi Server Init error" );
        return false;
    }    
    delay(1000);
        
    String IpString  = m_Wifi.showIP();
    DBGL( IpString );
        
    m_Wifi.confMux(1);
  
    delay(100);
    if( !m_Wifi.confServer( 1, 80 ) )
    {
        DBGL( "Server set up error!" );
        return false;
    }
    
    return true;
}

int  WifiServerProcess::Process()
{
    char Buf[ 128 ];
    int Len = m_Wifi.ReceiveMessage( Buf );
    if( Len > 0 )
    {        
        String Str = Buf;
        if( Str.indexOf("Setting") != -1 )
        {
            DBGL( "Receive Setting\n" );
            //解析資料內容，格式為: Setting?SSID=XXXX&PW=XXXX&FBID=XXXX其中XXXX則為所需設定資料;//
            int Index = Str.indexOf('=');
            int EndIndex = Str.indexOf('&');
            if( Index != -1 && EndIndex != -1 )
            {
                //取得SSID設定字串;//
                m_ClientSetSSID = Str.substring( Index+1, EndIndex );
                                
                Index = Str.indexOf( '=', Index+1 );
                EndIndex = Str.indexOf('&', EndIndex+1 );
                if( Index != -1 && EndIndex != -1 )
                {
                    //取得PW設定字串;//
                    m_ClientSetPW = Str.substring( Index+1, EndIndex );
                    
                    Index = Str.indexOf( '=', Index+1 );
                    if( Index != -1 )
                    {
                        m_ClientSetFBID = Str.substring( Index+1 );

                        //取得設定資料,回傳設定成功頁面;//
                        if( ReplyPageSucces() )
                        {                            
                            //延遲一段時間確保資料傳送完畢;//
                            delay(2000);
                            
                            m_Wifi.closeMux(m_Wifi.m_ClientId);
                            
                            return WIFI_CLIENT_SETUPED; 
                        }                           

                        m_Wifi.closeMux(m_Wifi.m_ClientId);                        
                    }
                }                
            }
            
            //關閉連線;//
            m_Wifi.closeMux(m_Wifi.m_ClientId);
        }
        else if( Len == 1 && Str[0] == ' ' )
        {
            DBGL( "Receive Index" );
            
            //有Client連結，回傳設定頁面;//
            ReplyPageSetting();

            //關閉連線;//
            m_Wifi.closeMux(m_Wifi.m_ClientId);
        }
        /*
        else
        {
            //關閉連線;//
            m_Wifi.closeMux(m_Wifi.m_ClientId);
        }
        */
    }
    
    return WIFI_SERVER_WAIT;
}

bool WifiServerProcess::ReplyHead()
{
    if( m_Wifi.Send(m_Wifi.m_ClientId, F("HTTP/1.1 200 OK\r\n")) )  
        if( m_Wifi.Send(m_Wifi.m_ClientId, F("Content-Type:text/html\r\n")) )
            if( m_Wifi.Send(m_Wifi.m_ClientId, F("Connection: close\r\n\r\n")) )
                return true;
   
    return false;
}

bool WifiServerProcess::ReplyPageSetting()
{        
    if( !ReplyHead() )
        return false;
/*
    if( m_Wifi.Send(m_Wifi.m_ClientId, F("<!DOCTYPE html><html><head><meta charset=\"UTF-8\"/><title>Facebook Likes Setting</title></head>"  )) )
        if( m_Wifi.Send(m_Wifi.m_ClientId, F("<body><form id=\"signup\" method=\"GET\" action=\"http://192.168.4.1/Setting\"><label>SSID:<br></label><input type=\"text\" id=\"SSID\" name=\"SSID\" MAXLENGTH=24 SIZE=\"24\"/><br><br>")) )
            if( m_Wifi.Send(m_Wifi.m_ClientId, F("<label>Password:<br></label><input type=\"text\" id=\"PW\" name=\"PW\" MAXLENGTH=24 SIZE=\"24\"/><br><br><label>FaceBook ID:<br></label><input type=\"text\" id=\"FBID\" name=\"FBID\" MAXLENGTH=24 SIZE=\"24\"/><br><br>")) )
                if( m_Wifi.Send(m_Wifi.m_ClientId, F("<input type=\"submit\" value=\"Apply\"/><input TYPE=\"RESET\" VALUE=\"Reset\"/></form></body></html>")) )
                    return true;
*/
    if( m_Wifi.Send(m_Wifi.m_ClientId, F("<!DOCTYPE html><html><head><meta charset=\"UTF-8\"/>")))
    if( m_Wifi.Send(m_Wifi.m_ClientId, F("<title>Facebook Likes Setting</title></head><body>")))
    if( m_Wifi.Send(m_Wifi.m_ClientId, F("<form id=\"signup\" method=\"GET\" action=\"http://192.168.4.1/Setting\">")))
    if( m_Wifi.Send(m_Wifi.m_ClientId, F("<label>SSID:<br></label>")))
    if( m_Wifi.Send(m_Wifi.m_ClientId, F("<input type=\"text\" id=\"SSID\" name=\"SSID\" MAXLENGTH=24 SIZE=\"24\"/>")))
    if( m_Wifi.Send(m_Wifi.m_ClientId, F("<br><br><label>Password:<br></label>")))
    if( m_Wifi.Send(m_Wifi.m_ClientId, F("<input type=\"text\" id=\"PW\" name=\"PW\" MAXLENGTH=24 SIZE=\"24\"/>")))
    if( m_Wifi.Send(m_Wifi.m_ClientId, F("<br><br><label>FaceBook ID:<br></label>")))
    if( m_Wifi.Send(m_Wifi.m_ClientId, F("<input type=\"text\" id=\"FBID\" name=\"FBID\" MAXLENGTH=24 SIZE=\"24\"/>")))
    if( m_Wifi.Send(m_Wifi.m_ClientId, F("<br><br><input type=\"submit\" value=\"Apply\"/>")))
    if( m_Wifi.Send(m_Wifi.m_ClientId, F("<input TYPE=\"RESET\" VALUE=\"Reset\"/></form></body></html>\r\n")))
        return true;

    return false;            
}

bool WifiServerProcess::ReplyPageSucces()
{
    if( !ReplyHead() )
       return false;
   
    if( m_Wifi.Send(m_Wifi.m_ClientId, F("<!DOCTYPE html><html><head> <meta charset=\"UTF-8\"/><title>Facebook Likes Setting Success</title></head>")) )
    {
        //if( m_Wifi.Send(m_Wifi.m_ClientId, F("<body><table border=\"0\" align=\"center\" width='240'><tbody><tr><td align=\"left\">")) )
        if( m_Wifi.Send(m_Wifi.m_ClientId, F("<body><table border=\"0\" width='240'><tbody><tr><td align=\"left\">")) )
        {
            String Str = "SSID:" + m_ClientSetSSID + "<br><br>";
            if( m_Wifi.Send(m_Wifi.m_ClientId, Str ) )
            {
                Str = "Password:" + m_ClientSetPW + "<br><br>";
                if( m_Wifi.Send(m_Wifi.m_ClientId, Str ) )
                {             
                    Str = "FaceBook ID:" + m_ClientSetFBID + "<br><br>";
                    if( m_Wifi.Send(m_Wifi.m_ClientId, Str ) )
                    {
                        //Str = "</td></tr><tr><td align=\"center\"><font color=\"#ff0000\"><br>Setting Success!<br>";
                        Str = "</td></tr><tr><font color=\"#ff0000\"><br>Setting Success!<br>";
                        if( m_Wifi.Send(m_Wifi.m_ClientId, Str) )
                        {
                            Str = "Now System reboot!</font></td></tr></tbody></table></body></html>\r\n";
                            if( m_Wifi.Send(m_Wifi.m_ClientId, Str ) )
                            {
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }     
     
    return false;
}

int  WifiServerProcess::GetRequestValue( int Type )
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
    m_FbJsonField = F("likes");
    m_FbHost = F("graph.facebook.com");
    m_FbPort = 80;
}

WifiClientProcess::~WifiClientProcess()
{
    
}

bool WifiClientProcess::Setup( String Ssid, String PW, String FbID )
{
    m_Wifi.begin(); 
     
    if( !m_Wifi.Initialize( STA, Ssid, PW ) )
    {
        DBG( "Wifi Client Init error\n" );
        return false;
    }
    
    delay(8000);  //Delay一小段時間確保能取得配置的IP範例為8秒;//

    String IpString  = m_Wifi.showIP();
    DBGL( IpString );

    m_FbID = FbID;
    
    return true;
}

int  WifiClientProcess::Process()
{    
    if( !FbRequest() )
    {    
        return WIFI_ERR_FB_REQ;
    }
    
    char Buf[32];
    if( m_Wifi.ReceiveJsonMsg( m_FbJsonField, Buf ) )
    {
        if( Buf[0] != '\"' )
        {            
            m_RequestValue = atoi( Buf );
        }
        return WIFI_REQ_SUCESS;        
    }
 
    return WIFI_ERR_NO_DATA;
}

int  WifiClientProcess::GetRequestValue( int Type )
{
    if( Type != WIFI_REQ_FB_FIELD_NUM )
        return 0;
    
    return m_RequestValue;
}

String  WifiClientProcess::GetRequestString( int Type )
{
    return "";  
}

bool WifiClientProcess::FbRequest()
{
    DBG( "\nFb Request!\n" );
    
    // if there's a successful connection:
    if (m_Wifi.ipConfig( TCP, m_FbHost, m_FbPort ) ) 
    {
        DBG("connecting...\n");
                
        char Buffer[64];
        sprintf( Buffer, "GET %s?fields=%s HTTP/1.1\r\n", m_FbID.c_str(), m_FbJsonField.c_str() );
        m_Wifi.Send( Buffer );
        sprintf( Buffer, "HOST: %s\r\n", m_FbHost.c_str() );
        m_Wifi.Send( Buffer );
        m_Wifi.Send(F("Connection: close\r\n\r\n"));
        
        return true;
    } 
    else 
    {
        // if you couldn't make a connection:
        DBG("connection failed\n");
        DBG("disconnecting.\n");
                
        m_Wifi.closeMux();        
    }       
    return  false;
}


