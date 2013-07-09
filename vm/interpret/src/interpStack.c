#include "interpStack.h"


vbool dvmIsBreakFrame(const u4* fp)
{
    return SAVEAREA_FROM_FP(fp)->method == NULL;
}