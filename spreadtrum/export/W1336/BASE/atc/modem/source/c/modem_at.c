#include "modem_private.h"
#include "mux_api.h"

/**
 * -1 means not modem file protocol, others are modem file handle
 */
int ATC_ProcessModemFileProtocol(void* atc_config_ptr,
                                 const char* data,
                                 int datalen,
                                 int linkid,
                                 ATResponseCB cb)
{
  int result = -1;
  SCI_TRACE_LOW("[ModemFile] ATC_ProcessModemFileProtocol\n");
  result = modem_control_main(atc_config_ptr, data, datalen, linkid, cb);
  return result;
}

/**
 * -1 means not modem file protocol, others are modem file handle
 */
int ATC_ProcessModemFileProtocolEx(const char* data, int datalen, int linkid)
{
  int result = -1;
  SCI_TRACE_LOW("[ModemFile] ATC_ProcessModemFileProtocolEx\n");
  result = modem_control_main(NULL, data, datalen, linkid, NULL);
  return result;
}

int modem_at_response(void* atc_config_ptr, unsigned char flag, ATResponseCB cb)
{
  int result = -1;
  unsigned char buf[4] = {0};

  SCI_TRACE_LOW("[ModemFile] modem_at_response %c\n", flag);

  switch(flag)
  {
  case XACK:
  case XNAK:
  case YCRC:
    buf[0] = flag;
    if (atc_config_ptr && cb)
    {
      cb(atc_config_ptr, buf, 1);
    }
    else
    {
      MUX_Write(buf, 1, 0);
    }
    result = 0;
    break;
  default:
    //Not handled yet
    break;
  }

  return result;
}
