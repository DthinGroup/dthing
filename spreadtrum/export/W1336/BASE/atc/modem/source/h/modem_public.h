#ifndef MODEM_PUBLIC_H
#define MODEM_PUBLIC_H

typedef void(*ATResponseCB)(void*, unsigned char*, int);

/**
 * -1 means not modem file protocol, others are modem file handle
 */
int ATC_ProcessModemFileProtocol(void* atc_config_ptr, const char* data, int datalen, int linkid, ATResponseCB cb);
int ATC_ProcessModemFileProtocolEx(const char* data, int datalen, int linkid);

void ATC_EnableModemFileProtocol(unsigned char linkid);
void ATC_EnableDirectFileSend(unsigned char linkid, char* filename);

int ATC_isFileProtocolMode(int linkid);
int ATC_isDirectSendFileMode(int linkid);
#endif //MODEM_PUBLIC_H