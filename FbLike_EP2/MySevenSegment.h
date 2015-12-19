#ifndef __MY_SEVEN_SEGMENT_H__
#define __MY_SEVEN_SEGMENT_H__

#include <Arduino.h>
#include <LedControl.h>

#define LED_NUM     1   //數字數字滾動效果;//
#define LED_SERVER  2   //SERVER效果;//
 
class MySevenSegment
{
public:    
    MySevenSegment( int PinData, int PinClk, int PinCs, byte digits, int NumDevices=1 );
    ~MySevenSegment();

    //顯示字串;//
    //支援的字元限以下幾種;//
    //'0','1','2','3','4','5','6','7','8','9','0',
    //'A','b','c','d','E','F','H','L','P',
    //'.','-','_',' ' 
    void PrintString( String Str );

    //顯示指定數字;//
    void PrintNumber( unsigned long Num );

    //指定字串由右往左滾動;//
    void RollSubtitle( String Str );

    //顯示指定效果;//
    void Effect( byte mode, unsigned long Value = 0);
    
private:
    //數字閃爍效果,Times:次數;//
    void EffectFlicker( int Times );

    //直接從現在數字滾動到指定數字;//
    void EffectPlusImmediately( unsigned long Num );

    //全部數字滾動遞增效果;//
    void EffectPlusAll( unsigned long Num );

    //由高至低位數一次滾動一位數;//
    void EffectPlusOne( unsigned long Num );

    void EffectRandom( unsigned long Num );

    //最外圍的跑燈效果;//
    void EffectAround( int Times );

    //設定Server時的效果, SEr __ -> SEr_ _ -> SEr__ ;//
    void EffectServer();

    //重新設定指定數字轉換至陣列;//
    void ResetNumToArray( unsigned long NewNum );

    //整理陣列數字,將字串移至指定位數,不足的前面補字串0,超過的位數則刪除;//
    void ArrangeArrary( char *NumArray, byte Digit );

private:   
    LedControl mLed;

    //機器顯示位數;//
    byte mDigits;

    //數字滾動間格時間;//
    short mRollTime;

    //紀錄現在的數字;//
    unsigned long mCurrentNum;

    //Server狀態的動畫狀態計數;//
    byte mServerWaitCount;
    int mServerWaitTime;

    //資料處裡用字串陣列;//
    char mNewNum[16];
    char mLedNum[16];
};

#endif

