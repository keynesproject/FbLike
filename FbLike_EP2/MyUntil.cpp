#include "MyUntil.h"

unsigned long Pow( unsigned long X, unsigned long Y )
{
    unsigned long Temp = 1;
    if( Y == 0 )
        return 1;
        
    for( int i=0; i<Y; i++ )
    {
        Temp *= X;
    }

    return Temp;
}
