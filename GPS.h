#ifndef _GPS_H_
#define _GPS_H_

#include "SIM900.h"

class GPS {
public:
     // GPS methods
     int Enable();
     int Disable();
     int IsFixed();
     int GetLocation(char *location);
};

#endif
