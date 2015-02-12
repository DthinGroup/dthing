#include "modem_private.h"

#define MODEM_FS_ROOT "D:/" //D:/app/

int modem_fs_open_file(const char* fname)
{
  int i = 0;
  int j = 0;
  int result = -1;
  int len = strlen(fname);
  int rtlen = strlen(MODEM_FS_ROOT);
  uint16 wname[MAX_FILENAME_LEN] = {0};
  char* pch = MODEM_FS_ROOT;
  uint16 attr = 0;
  int sfshandle = 0;

  for (i = 0; i < rtlen; i++)
  {
    wname[i] = (uint16)*pch;
    pch++;
  }

  pch = fname;

  for (j = rtlen; j < (rtlen + len); j++)
  {
    wname[j] = (uint16)*pch;
    pch++;
  }

  SCI_TRACE_LOW("[ModemFile] modem_fs_open_file\n");
  if (0 > modem_fs_delete_file(fname))
  {
    goto end;
  }
  SCI_TRACE_LOW("[ModemFile] start to create file %s\n", fname);
  //create file and open it with append property//SFS_MODE_APPEND
  sfshandle = SFS_CreateFile(wname, SFS_MODE_SHARE_WRITE|SFS_MODE_OPEN_ALWAYS, 0, 0);

  if (sfshandle > 0)
  {
    result = sfshandle;
    SCI_TRACE_LOW("[ModemFile] Create File %s%s success\n", MODEM_FS_ROOT, fname);
  }
end:
  SCI_TRACE_LOW("[ModemFile] modem_fs_open_file with result %d\n", result);
  return result;
}

int modem_fs_write_file(int handle, char* data, int len)
{
  int result = -1;
  int written = 0;

  if (SFS_WriteFile((SFS_HANDLE)handle, data, len, &written, 0x0) ==  SFS_NO_ERROR)
  {
    result = written;
    SCI_TRACE_LOW("[ModemFile] Write %d/%d with handle[%d] success\n", written, len, handle);
  }
  return result;
}

int modem_fs_append_file(int handle, char* data, int len)
{
  return -1;
}

int modem_fs_close_file(int handle)
{
  int result = -1;

  if (SFS_CloseFile((SFS_HANDLE)handle) == SFS_NO_ERROR)
  {
    result = 0;
    SCI_TRACE_LOW("[ModemFile] Close handle[%d] success\n", handle);
  }
  return result;
}

int modem_fs_delete_file(const char* fname)
{
  int i = 0;
  int j = 0;
  int result = -1;
  int len = strlen(fname);
  int rtlen = strlen(MODEM_FS_ROOT);
  uint16 wname[MAX_FILENAME_LEN] = {0};
  char* pch = MODEM_FS_ROOT;
  uint16 attr = 0;

  for (i = 0; i < rtlen; i++)
  {
    wname[i] = (uint16)*pch;
    pch++;
  }

  pch = fname;

  for (j = rtlen; j < (rtlen + len); j++)
  {
    wname[j] = (uint16)*pch;
    pch++;
  }

  if (SFS_GetAttr(wname, &attr) != SFS_NO_ERROR)
  {
    result = 0;
    SCI_TRACE_LOW("[ModemFile] No exist file %s\n", fname);
    goto end;
  }

  if (SFS_DeleteFile(wname, 0x0) == SFS_NO_ERROR)
  {
    SCI_TRACE_LOW("[ModemFile] Delete file %s success\n", fname);
    result = 0;
  }

end:
  return result;
}
