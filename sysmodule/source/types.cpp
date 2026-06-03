#include "types.h"

#ifdef NO_SWITCH_H

bool accountUidIsValid(const AccountUid *Uid) {
    return Uid->uid[0]!=0 || Uid->uid[1]!=0;
}

#endif