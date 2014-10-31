#ifndef CPL_ATCONTROL_H
#define CPL_ATCONTROL_H

int cpl_handleATRequest(char* atcommand, char* instr, char** outstr);
int cpl_handleATRequestWithNumericParam(char* atcommand, int param, char** outstr);
void cpl_freeOutStr(char** outstr);

#endif //CPL_ATCONTROL_H