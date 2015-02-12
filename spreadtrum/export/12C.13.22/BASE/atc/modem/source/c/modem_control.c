#include "modem_private.h"
#include "atc.h"
#define DEFAULT_DELAY_TIME  3000 //in millseconds

static ModemRequest g_req = {0};
static uint32 g_fileListener = 0;

static int modem_save_data(ModemRequest *request, char *data, int datalen);
static void _Modem_File_Task(uint32 argc, void* argv);
static void modem_register_device();


static void modem_register_device()
{
    uint16    dev_name[] = {'D', 0x00};
    SFS_DEVICE_FORMAT_E  format = SFS_FAT16_FORMAT;
    SFS_ERROR_E sfsError = SFS_ERROR_NONE;

    SCI_TRACE_LOW("[ModemFile] LSSFS_RegisterDevice  ");

    if (0 != SFS_GetDeviceStatus(dev_name))
    {
        sfsError = SFS_RegisterDevice(dev_name, &format);
        if(SFS_ERROR_NONE == sfsError)
        {
            SCI_TRACE_LOW("[ModemFile] SFS_RegisterDevice D: SUCCESS");
        }
        else
        {
            sfsError = SFS_Format(L"D", SFS_AUTO_FORMAT, NULL);
            if(SFS_ERROR_NONE == sfsError)
            {
                SCI_TRACE_LOW("[ModemFile] SFS_Format SUCCESS");
                SCI_TRACE_LOW("[ModemFile] SFS_RegisterDevice SUCCESS");
            }
            else
            {
                SCI_TRACE_LOW("[ModemFile] SFS_RegisterDevice error: %d", sfsError);
                SCI_TRACE_LOW("[ModemFile] SFS_Format error: %d", sfsError);
            }
        }
    }
}


void ATC_EnableModemFileProtocol(unsigned char linkid)
{
  if (g_req.state < MSTATE_WAIT)
  {
    g_req.linkid = linkid;
    g_req.state = MSTATE_WAIT;
    modem_register_device();
    ATC_StartFileListener();
  }
}

int modem_control_main(void* atc_config_ptr, const char* data, int datalen, int linkid, ATResponseCB cb)
{
  static int eotCount = 0;
  int result = -1;

  if ((atc_config_ptr == 0x0) || (data == 0x0) || (datalen <= 0))
  {
    goto end;
  }

  //save data
  if ((g_req.state > MSTATE_WAIT) && (linkid == g_req.linkid))
  {
    result = modem_control_available(datalen);

    if (result < 0)
    {
      g_req.state = MSTATE_END;
      SCI_TRACE_LOW("[ModemFile] Not enough memory\n");
    }
    
    result = modem_save_data(&g_req, data, datalen);
  }

  do
  {
    switch(g_req.state)
    {
    case MSTATE_INIT:
      SCI_TRACE_LOW("[ModemFile] modem file protocol is not opened\n");
      break;

    case MSTATE_WAIT:
      //modem_control_waiting(atc_config_ptr, cb);

      if (((*data == 'A') || (*data == 'a'))
        && ((*(data + 1) == 'T') || (*(data + 1) == 't')))
      {
        SCI_TRACE_LOW("[ModemFile] AT cmd\n");
        break;
      }

      result = modem_control_available(datalen);

      if (result < 0)
      {
        g_req.state = MSTATE_END;
        SCI_TRACE_LOW("[ModemFile] Not enough memory\n");
        break;
      }

      result = modem_check_header(&g_req, data, datalen);

      if (result >= 0)
      {
        int offset = result;
        g_req.linkid = linkid;
        g_req.state = MSTATE_READY;
        result = modem_save_data(&g_req, (data + offset), (datalen - offset));
        continue;
      }
      break;

    case MSTATE_READY:
      SCI_TRACE_LOW("[ModemFile] MSTATE_READY\n");
      result = modem_parse_header(&g_req);

      if (result == MRESULT_SUCCESS)
      {
        //create file
        //g_req.handle = modem_fs_open_file(g_req.filename);
        g_req.state = MSTATE_PREVERIFY;
        continue;
        //g_req.state = MSTATE_RECEIVING_DATA;
        //modem_at_response(atc_config_ptr, XACK, cb);
        //modem_at_response(atc_config_ptr, (g_req.mode == MODE_CRC)? YCRC : XNAK, cb);
      }
      else if (result == MRESULT_ERROR)
      {
        g_req.state = MSTATE_ERROR;
      }
      else if (result == MRESULT_RETRY)
      {
        modem_at_response(atc_config_ptr, XNAK, cb);
        g_req.state = MSTATE_WAIT;
      }
      break;

    case MSTATE_PREVERIFY:
      SCI_TRACE_LOW("[ModemFile] MSTATE_PREVERIFY\n");

      result = modem_check_cancel(data, datalen);

      if (result > 0)
      {
        g_req.state = MSTATE_CANCEL;
        modem_at_response(atc_config_ptr, XACK, cb);
        break;
      }

      result = modem_preverify_filename(&g_req);

      if (result == MRESULT_SUCCESS)
      {
        g_req.state = MSTATE_RECEIVING_DATA;
        modem_at_response(atc_config_ptr, XACK, cb);
        modem_at_response(atc_config_ptr, (g_req.mode == MODE_CRC)? YCRC : XNAK, cb);
        continue;
      }
      else if (result == MRESULT_ERROR)
      {
        g_req.state = MSTATE_ERROR;
      }
      else if (result == MRESULT_RETRY)
      {
        modem_at_response(atc_config_ptr, XNAK, cb);
        g_req.state = MSTATE_WAIT;
      }
      break;

    case MSTATE_RECEIVING_DATA:
      SCI_TRACE_LOW("[ModemFile] MSTATE_RECEIVING_DATA\n");
      result = modem_check_cancel(data, datalen);

      if (result > 0)
      {
        g_req.state = MSTATE_CANCEL;
        modem_at_response(atc_config_ptr, XACK, cb);
        break;
      }

      result = modem_check_header(&g_req, data, datalen);

      result = modem_parse_data(&g_req);

      //TODO:
      if (result == MRESULT_BLOCK_SUCCESS)
      {
        modem_at_response(atc_config_ptr, XACK, cb);
      }
      else if (result == MRESULT_SUCCESS)
      {
        g_req.state = MSTATE_FINISH;
        modem_at_response(atc_config_ptr, XACK, cb);
      }
      else if ((result == MRESULT_ERROR) || (result == MRESULT_ERROR_WRITE_FILE))
      {
        g_req.state = MSTATE_ERROR;
      }
      else if (result == MRESULT_RETRY)
      {
        modem_at_response(atc_config_ptr, XNAK, cb);
        g_req.state = MSTATE_WAIT;
      }
      break;

    case MSTATE_CANCEL:
      SCI_TRACE_LOW("[ModemFile] MSTATE_CANCEL\n");
      //TODO: release resource
      g_req.state = MSTATE_END;
      result = 0;
      break;

    case MSTATE_FINISH:
      SCI_TRACE_LOW("[ModemFile] MSTATE_FINISH\n");
      //TODO: release resource
      result = modem_check_end(&g_req.buf[g_req.bufpos], g_req.buflen - g_req.bufpos);

      if (result > 0)
      {
        eotCount++;
        if (eotCount > 2)
        {
          eotCount = 0;
          g_req.state = MSTATE_END;
        }
        modem_at_response(atc_config_ptr, (eotCount == 1)? XNAK : XACK, cb);

        if (eotCount == 2)
        {
          modem_at_response(atc_config_ptr, YCRC, cb);
        }
        break;
      }

      if (eotCount >= 2)
      {
          eotCount = 0;
          g_req.state = MSTATE_END;
          modem_at_response(atc_config_ptr, XACK, cb);
          continue;
      }

      g_req.state = MSTATE_END;
      break;

    case MSTATE_ERROR:
      SCI_TRACE_LOW("[ModemFile] MSTATE_ERROR\n");
      //TODO: retry or goto end
      g_req.state = MSTATE_END;
      result = 0;
      break;

    case MSTATE_END:
    default:
      SCI_TRACE_LOW("[ModemFile] MSTATE_END or else\n");
      g_req.state = MSTATE_INIT;

      if (g_req.handle > 0)
      {
        modem_fs_close_file(g_req.handle);
      }

      ATC_StopFileListener();

      memset(&g_req, 0x0, sizeof(ModemRequest));
      result = 0;
      break;
    }
    break;
  }while(1);

end:
  SCI_TRACE_LOW("[ModemFile] modem_control_main with result %d\n", result);
  return result;
}

//True: parse SOT
//False: do nothing
int ATC_isFileProtocolMode(int linkid)
{
  int result = 0;

  if (g_req.linkid != linkid)
  {
    goto end;
  }

  if (g_req.state >= MSTATE_WAIT)
  {
    result = 1;
  }

end:
  return result;
}

//True: parse data with modem file protocol
//False: normal handle
int ATC_isFileReceivingMode(int linkid)
{
  int result = 0;

  if (g_req.linkid != linkid)
  {
    goto end;
  }

  if (g_req.state > MSTATE_WAIT)
  {
    result = 1;
  }

end:
  return result;
}

static int modem_save_data(ModemRequest *request, char *data, int datalen)
{
  int i = 0;
  char *buf = g_req.buf + g_req.buflen;
  memcpy(buf, data, datalen);
  g_req.buflen += datalen;
  //SCI_TRACE_LOW("[ModemFile] modem_save_data d0[0x%x] length[%d]\n", data[0], datalen);
  //for (i = 0; i < datalen; i++)
  {
    //SCI_TRACE_LOW("[ModemFile] modem_save_data [%d] 0x%x\n", i, data[i]);
  }
  return 0;
}

static int modem_control_available(int datalen)
{
  int result = 0;

  if ((g_req.buflen + datalen) >= MAX_BUFFER_LEN)
  {
    g_req.buflen = g_req.buflen - g_req.bufpos;
    memcpy(g_req.buf, g_req.buf + g_req.bufpos, g_req.buflen);
    g_req.bufpos = 0;
    memset((g_req.buf + g_req.buflen), 0x0, (MAX_BUFFER_LEN - g_req.buflen));
  }

  if ((g_req.buflen + datalen) >= MAX_BUFFER_LEN)
  {
    g_req.state = MSTATE_ERROR;
    result = -1;
  }
  return result;
}

#define MAX_CRC_WAITING_COUNT 10
static void _Modem_File_Task(uint32 argc, void* argv)
{
  int linkid = 0x0;
  void **params = (void **)argv;
  unsigned char buf[2] = {0};
  ModemRequest *req = 0x0;
  static uint32 nextMode = MODE_CRC;
  static uint32 crcCount = MAX_CRC_WAITING_COUNT;

  if(argc == 2)
  {
    req = params[0];
  }

  while(1)
  {
    if (req->state != MSTATE_WAIT)
    {
      SCI_TRACE_LOW("[ModemFile] state is not MSTATE_WAIT\n");
      break;
    }

    if (nextMode == MODE_DEFAULT)
    {
      SCI_TRACE_LOW("[ModemFile] modem_control_waiting response NAK\n");
      buf[0] = XNAK;
      ATC_SendReslutRspEx(req->linkid, buf, 1);
      req->mode = MODE_DEFAULT;
      nextMode = MODE_CRC;
      crcCount = MAX_CRC_WAITING_COUNT;
    }
    else if (nextMode == MODE_CRC)
    {
      SCI_TRACE_LOW("[ModemFile] modem_control_waiting response CRC\n");
      buf[0] = YCRC;
      ATC_SendReslutRspEx(req->linkid, buf, 1);
      req->mode = MODE_CRC;
      crcCount--;
      nextMode = (crcCount == 0)? MODE_DEFAULT : MODE_CRC;
    }

    SCI_Sleep(((req->mode == MODE_CRC)? DEFAULT_DELAY_TIME : 10000));
  }

end:
  ATC_StopFileListener();
}

void ATC_StartFileListener()
{
  static void *params[2] = {0};

  if (g_fileListener != 0)
  {
    goto end;
  }

  params[0] = &g_req;

  SCI_TRACE_LOW("[ModemFile] params[0]:%p params[1]:%p\n", params[0], params[1]);
  g_fileListener = SCI_CreateThread("Modem_File_Task",
                                    "Modem_File_Queue",
                                    _Modem_File_Task,
                                    2,
                                    (void *)params,
                                    0x8000,
                                    10,
                                    PRI_GPS_TASK,
                                    SCI_PREEMPT,
                                    SCI_AUTO_START);
end:
  SCI_TRACE_LOW("modem start to listen file with thread id %d\n", g_fileListener);
}

void ATC_StopFileListener()
{
  if (g_fileListener == 0)
  {
    goto end;
  }

  SCI_TerminateThread(g_fileListener);
  SCI_DeleteThread(g_fileListener);
  g_fileListener = 0;
end:
  SCI_TRACE_LOW("modem stop to listen file\n");
}
