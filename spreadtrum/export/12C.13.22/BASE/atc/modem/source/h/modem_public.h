#ifndef MODEM_PUBLIC_H
#define MODEM_PUBLIC_H

typedef void(*ATResponseCB)(void*, unsigned char*, int);

/**
 * -1 means not modem file protocol, others are modem file handle
 */
int ATC_ProcessModemFileProtocol(void* atc_config_ptr, const char* data, int datalen, int linkid, ATResponseCB cb);

void ATC_EnableModemFileProtocol(unsigned char linkid);

int ATC_isFileProtocolMode(int linkid);
#endif //MODEM_PUBLIC_H