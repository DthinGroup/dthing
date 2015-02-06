#include "modem_private.h"

static int xymodem_parser(ModemRequest* request);
static int zmodem_parser(ModemRequest* request);

//return offset of SOH or STX, -1 means no found
int modem_check_header(ModemRequest* request, const char* data, int datalen)
{
  int i = 0;
  int result = -1;
  char* pch = data;
  for(i = 0; i < datalen; i++)
  {
    if (*pch == XSTX)
    {
      result = i;
      request->protocol = MTYPE_YMODEM_G;
      break;
    }
    else if (*pch == XSOH)
    {
      result = i;
      break;
    }
    pch++;
  }
  return result;
}

int modem_check_cancel(const char* data, int datalen)
{
  int i = 0;
  int result = -1;
  char* pch = data;
  int count = 0;

  for(i = 0; i < datalen; i++)
  {
    if (*pch == XCAN)
    {
      count++;
      if (count >= 5)
      {
        result = i;
        break;
      }
    }
    pch++;
  }
  return result;
}

int modem_check_end(const char* data, int datalen)
{
  int i = 0;
  int result = -1;
  char* pch = data;
  for(i = 0; i < datalen; i++)
  {
    if (*pch == XEOT)
    {
      result = i + 1;
      break;
    }
    pch++;
  }
  return result;
}

int modem_parse_data(ModemRequest* request)
{
  int result = -1;

  if (request)
  {
    SCI_TRACE_LOW("[ModemFile] modem_parser_process state[%d]\n", request->state);

    switch(request->protocol)
    {
    case MTYPE_XMODEM:
      result = xymodem_parser(request);
      break;
    case MTYPE_YMODEM:
      result = xymodem_parser(request);
      break;
    case MTYPE_YMODEM_G:
      result = xymodem_parser(request);
      break;
    case MTYPE_ZMODEM:
      result = zmodem_parser(request);
      break;
    default:
      result = xymodem_parser(request);
      break;
    }
  }

end:
  SCI_TRACE_LOW("[ModemFile] modem_parser_process with result %d\n", result);
  return result;
}

static int modem_get_package_size(ModemRequest* request)
{
  int i = 0;
  int len = request->buflen - request->bufpos;
  char *pch = request->buf + request->bufpos;
  int size = 0;

  for (i = 0; i < len; i++)
  {
    if (*pch == XSTX)
    {
      size = MAX_PACKAGE_SIZE_1024;
      break;
    }
    else if (*pch == XSOH)
    {
      size = MAX_PACKAGE_SIZE_128;
      break;
    }
    pch++;
  }
  return size;
}

int xymodem_parser(ModemRequest* request)
{
  int result = -1;
  char *pch = 0x0;
  int canCount = 0;
  int i = 0;
  int filelength = 0;
  int requiredLength = 0;
  int packageSize = 0;
  char* writeData = 0x0;
  int writeDataLen = 0;
  int len = 0;
  int isEndWithCTRLZ = 0;

  packageSize = modem_get_package_size(request);
  requiredLength = 3 + packageSize + ((request->mode == MODE_CRC)? 2 : 1);

  if ((request->buflen - request->bufpos) >= requiredLength)
  {
    //TODO: SOH|01|FE|<filedata>|CRC or AND
    //      SOT|01|FE|<filedata>|CRC or AND
    SCI_TRACE_LOW("[ModemFile] bufpos[0x%x] buflen[0x%x]\n", request->bufpos, request->buflen);
    do
    {
      pch = request->buf + request->bufpos;

      while (request->bufpos < request->buflen)
      {
        if ((*pch == XSTX) || (*pch == XSOH))
        {
          break;
        }
        pch++;
        request->bufpos++;
      }

      if ((*pch != XSTX) && (*pch != XSOH))
      {
        result = MRESULT_ERROR;
        SCI_TRACE_LOW("[ModemFile] Error: No STX/SOH\n");
        SCI_TRACE_LOW("[ModemFile] bufpos[0x%x] buflen[0x%x]\n", request->bufpos, request->buflen);
        break;
      }

      pch++;
      request->bufpos++;

      if ((*pch + *(pch + 1) != 0xff) || ((request->blockid + 1) != *pch))
      {
        result = MRESULT_ERROR;
        SCI_TRACE_LOW("[ModemFile] Error: pch[0x%x] pch1[0x%x] blockId[0x%x]\n", *pch, *(pch+1), request->blockid);
        break;
      }

      request->blockid = *pch;
      pch += 2;
      request->bufpos += 2;

      writeData = pch;
      isEndWithCTRLZ = 0;

      //save data into file
      for (i = 0; i < packageSize; i++)
      {
        MDEBUG("[ModemFile] [%d] 0x%x\n", i, *pch);
        if (*pch == XCTRLZ)
        {
          isEndWithCTRLZ++;
        }
        else
        {
          if (isEndWithCTRLZ > 0)
          {
            //Which means previous continous CTRLZ not means end of file
            writeDataLen += isEndWithCTRLZ;
            isEndWithCTRLZ = 0;
          }
          writeDataLen++;
        }
        *pch++;
        request->bufpos++;
      }

      //preverify
      if(request->mode == MODE_CRC)
      {
        request->crc[0] = *pch;
        *pch++;
        request->bufpos++;
        request->crc[1] = *pch;
        *pch++;
        request->bufpos++;
        SCI_TRACE_LOW("[ModemFile] crc0:%d crc1:%d\n", request->crc[0], request->crc[1]);
      }
      else
      {
        request->crc[0] = *pch;
        *pch++;
        request->bufpos++;
        SCI_TRACE_LOW("[ModemFile] crc0:%d\n", request->crc[0]);
      }

      //TODO: write data to buffer
      if (request->handle <= 0)
      {
        request->handle = modem_fs_open_file(request->filename);
      }

      len = modem_fs_write_file(request->handle, writeData, writeDataLen);

      if (len > 0)
      {
        request->writelen += len;
      }
      else
      {
        result = MRESULT_ERROR_WRITE_FILE;
        break;
      }

      if (request->expectlen == request->writelen)
      {
        result = MRESULT_SUCCESS;
      }
      else
      {
        result = MRESULT_BLOCK_SUCCESS;
      }
    } while(0);
  }
  else
  {
    result = MRESULT_PENDING;
  }

end:
  return result;
}

int zmodem_parser(ModemRequest* request)
{
  int result = -1;
  //TODO:
  return result;
}

int modem_preverify_filename(ModemRequest* request)
{
  int result = MRESULT_PENDING;
  char *pch = 0x0;

  pch = request->buf + request->bufpos;

  while((*pch == 0x0) && (request->bufpos < request->buflen))
  {
    pch++;
    request->bufpos++;
  }

  if(request->mode == MODE_CRC)
  {
    if ((request->buflen - request->bufpos) < 2)
    {
      goto end;
    }

    request->crc[0] = *pch;
    *pch++;
    request->bufpos++;
    request->crc[1] = *pch;
    *pch++;
    request->bufpos++;
  }
  else
  {
    if ((request->buflen - request->bufpos) < 1)
    {
      goto end;
    }
    request->crc[0] = *pch;
    *pch++;
    request->bufpos++;
  }
  result = MRESULT_SUCCESS;
  MDEBUG("[ModemFile] filename[%s] filelen[%d] crc0[0x%x] crc1[%d]\n",
    request->filename, request->expectlen, request->crc[0], request->crc[1]);

end:
  return result;
}

int modem_parse_header(ModemRequest* request)
{
  int result = MRESULT_PENDING;
  char *pch = 0x0;
  int canCount = 0;
  int i = 0;
  int zeroCount = 0;
  int flen = 0;
  int weight = 1;
  //int requiredLength = 0;

  //requiredLength = 3 + MAX_PACKAGE_SIZE_128 + ((request->mode == MODE_CRC)? 2 : 1);

  //if ((request->buflen - request->bufpos) >= requiredLength)
  {

    pch = request->buf + request->bufpos;

    for (i = 0; i < (request->buflen - request->bufpos); i++)
    {
      MDEBUG("[ModemFile] [%d] 0x%x\n", i, *pch);
      if (*pch == 0)
      {
        zeroCount++;
      }
      if (zeroCount == 3)
      {
        break;
      }
      pch++;
    }

    if (zeroCount == 3)
    {
      //TODO: SOH|00|FF|<filename>|<filelength>|CRC or AND
      do
      {
        pch = request->buf + request->bufpos;

        if (*pch != XSOH)
        {
          break;
        }

        pch++;
        request->bufpos++;

        if ((*pch != 0x0) || (*(pch + 1) != 0xff))
        {
          result = MRESULT_ERROR;
          break;
        }

        pch += 2;
        request->bufpos += 2;
        i = 0;
        while((*pch != 0x0) && (request->bufpos < request->buflen))
        {
          request->filename[i] = *pch;
          i++;
          pch++;
          request->bufpos++;
        }

        if (*pch != 0x0)
        {
          result = MRESULT_ERROR;
          break;
        }

        pch++;
        request->bufpos++;

        i = 0;
        while((*pch != 0x0) && (request->bufpos < request->buflen))
        {
          request->filelen[i] = *pch - '0';
          i++;
          pch++;
          request->bufpos++;
        }

        //get expect file length from request->filelen array
        flen = i;
        

        while(i > 0)
        {
          request->expectlen += request->filelen[i - 1] * weight;
          weight *= 10;
          i--;
        }

        if (*pch != 0x0)
        {
          result = MRESULT_ERROR;
          break;
        }

        pch++;
        request->bufpos++;

        MLOG("[ModemFile] filename[%s] expectlen[%d]\n",
          request->filename, request->expectlen);

        //TODO: delete old file and create new file
        result = MRESULT_SUCCESS;
      }while(0);
    }
  }

end:
  return result;
}

unsigned int CRC16(unsigned char *ptr, unsigned char len)
{
   unsigned int crc = 0xffff;
   unsigned char i = 0;
   while(len--)
   {
     crc ^= *ptr++;
     for(i = 0; i < 8; i++)
     {
       if(crc & 0x1)
       {
        crc >>= 1;
        crc ^= 0xa001;
       }
       else
       {
        crc >>= 1;
       }
     }
   }
   return crc;
}

unsigned char CheckCRC(unsigned char *ptr,unsigned char len)
{
  unsigned int crc;
  crc=(unsigned int)CRC16(ptr, len - 2);
  if(ptr[len - 1] == (crc >> 8) && ptr[len - 2] == (crc & 0x00ff))
  {
    return 0xff;
  }
  else
  {
     return 0x0;
  }
}
