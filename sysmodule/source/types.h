#pragma once

#ifdef NO_SWITCH_H

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 unsigned long long

typedef struct {
    u64 uid[2];
} AccountUid;

bool accountUidIsValid(const AccountUid *Uid);

#define Result u16
#else
#include <switch.h>
#endif