#ifndef __WIFI_PROCESS_H__
#define __WIFI_PROCESS_H__

#include <Arduino.h>
#include "WifiMt7681.h"

#define WIFI_REQ_SUCESS     0    //FB數字顯示;// 
#define WIFI_DEFAULT        1    //初始化狀態;//              ______
#define WIFI_SERVER_WAIT    2    //伺服器狀態等待設定;//      SEt UP
#define WIFI_CLIENT_SETUPED 3    //Clent回傳資料設定完畢;//      OFF
#define WIFI_ERR_INITIAL    4    //WIFI 模組初始化失敗;//     IF E01
#define WIFI_ERR_SETUP      5    //WIFI 模組連線失敗;//       IF E02
#define WIFI_ERR_NO_DATA    6    //沒請求到任何數字;// 
#define WIFI_ERR_FB_REQ     7    //FB請求錯誤;//              Fb EEE

#define WIFI_REQ_SSID         1
#define WIFI_REQ_PW           2
#define WIFI_REQ_FBID         3
#define WIFI_REQ_FB_FIELD_NUM 4

//////////////////////////////////////////////////////////
// WifiProcess
//////////////////////////////////////////////////////////
class WifiProcess
{
public:
    WifiProcess();
    ~WifiProcess();
    bool Initial( byte PinES, byte PinReset );
    virtual bool Setup( String Ssid, String PW, String FbID="" ) = 0;
    virtual int  Process() = 0;
    
    virtual unsigned long  GetRequestValue( int Type ) = 0; //取得請求欄位的值//
    virtual String  GetRequestString( int Type ) = 0;
 
    void Reset();
protected:
    WifiMt7681  m_Wifi;
    bool m_IsInitial;
    
};


//////////////////////////////////////////////////////////
// WifiServerProcess
// Wifi模組作為Server的程序
//////////////////////////////////////////////////////////
class WifiServerProcess : public WifiProcess
{
public:
    WifiServerProcess();
    ~WifiServerProcess();
    
    bool Setup( String Ssid, String PW, String FbID="" );
    int Process();
    
    unsigned long  GetRequestValue( int Type ); //取得請求欄位的值//
    String  GetRequestString( int Type );
private:            
    void ReplyHead( int HtmlLength = 0 );
    void ReplyPageSetting();     
    void ReplyPageSucces();
        
private:
    String m_ClientSetSSID;
    String m_ClientSetPW;
    String m_ClientSetFBID;    
    String m_ServerIP;
};

//////////////////////////////////////////////////////////
// WifiClientProcess
// //Wifi模組作為Client的程序;//
//////////////////////////////////////////////////////////
class WifiClientProcess : public WifiProcess
{
public:
    WifiClientProcess();
    ~WifiClientProcess();
    
    bool Setup( String Ssid, String PW, String FbID="" );
    int Process();
    
    unsigned long GetRequestValue( int Type ); //取得請求欄位的值//
    String GetRequestString( int Type );
private:
    unsigned long Pow( unsigned long X, unsigned long Y );  
    void FbRequest();

    unsigned long  m_RequestValue; //請求得到的值;//
    String m_RequestStr;   //請求得到的字串;//
    String m_FbField;  //要請求欄位的名稱;//
    String m_FbHost;
    char   m_FbPort;
    String m_FbID;
    
};

#endif




