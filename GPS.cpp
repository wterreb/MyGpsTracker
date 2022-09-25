#include "GPS.h"


int GPS::Enable()
{
    char ret_val = -1;
    if (AT_RESP_OK == gsm.SendATCmdWaitResp(F("AT+GPS=1"), 1000, 50, str_ok, 2)) {
       ret_val = 1;
    } else ret_val = 0;
    return (ret_val);
}

int GPS::Disable()
{
    char ret_val = -1;
    if (AT_RESP_OK ==  gsm.SendATCmdWaitResp(F("AT+GPS=0"), 1000, 50, str_ok, 2)) {
       ret_val = 1;
    } else ret_val = 0;
    return (ret_val);
}

int GPS::IsFixed()
{
    char ret_val = -1;
    if (AT_RESP_OK ==  gsm.SendATCmdWaitResp(F("AT+LOCATION=2"), 1000, 50, str_ok, 2)) {
       ret_val = 1;
    } else {
        if (gsm.IsStringReceived("+LOCATION: GPS NOT FIX NOW")) {
          ret_val = 0;
        }
    }
    return (ret_val);
}

int GPS::GetLocation(char *location)
{
    char ret_val = -1;
    char *p_char;
    char *p_char1;
    
    if (CLS_FREE != gsm.GetCommLineStatus()) return (ret_val);
     ret_val = 0; // not found yet
     location[0] = 0; // GPS Location not found yet  => empty string

    if (AT_RESP_OK ==  gsm.SendATCmdWaitResp(F("AT+LOCATION=2"), 1000, 50, str_ok, 2)) {
       ret_val = 1;
      
      p_char = strstr((char *)(gsm.comm_buf+2),"\r\n");
       if (p_char != NULL) {
             int position = ((char *)p_char - (char *)gsm.comm_buf);
             // Copy but ignore "\r\n"
             int j = 0; 
             for (int i = 0; i < position; i++) {
               if (gsm.comm_buf[i]!='\r' &&  gsm.comm_buf[i]!='\n') {
                  location[j++] = gsm.comm_buf[i];
               }
             }
             location[j++] = '\0'; 
            ret_val = 1;
       }
    } else {

    }
    return (ret_val);
}
