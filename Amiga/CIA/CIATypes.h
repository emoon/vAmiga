// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _CIA_T_INC
#define _CIA_T_INC

/* Emulated CIA model
 *
 *   CIA_8520_DIP  mimics option "[ ] 391078-01" in UAE (default)
 *   CIA_8520_PLCC mimics option "[X] 391078-01" in UAE (A600)
 */
typedef enum : long
{
    CIA_8520_DIP,
    CIA_8520_PLCC
}
CIAType;

inline bool isCIAType(long value)
{
    return value >= CIA_8520_DIP && value <= CIA_8520_PLCC;
}

inline const char *ciaTypeName(CIAType type)
{
    assert(isCIAType(type));

    switch (type) {
        case CIA_8520_DIP:   return "CIA_8520_DIP";
        case CIA_8520_PLCC:  return "CIA_8520_PLCC";
        default:             return "???";
    }
}

typedef union
{
    struct
    {
        u8 hi;
        u8 mid;
        u8 lo;
    };
    u32 value;
}
Counter24;

typedef struct
{
    Counter24 value;
    Counter24 latch;
    Counter24 alarm;
}
CounterInfo;

typedef struct
{
    CIAType type;
    bool    todBug;
}
CIAConfig;

typedef struct
{
    struct {
        u8 port;
        u8 reg;
        u8 dir;
    } portA;

    struct {
        u8 port;
        u8 reg;
        u8 dir;
    } portB;

    struct {
        u16 count;
        u16 latch;
        bool running;
        bool toggle;
        bool pbout;
        bool oneShot;
    } timerA;

    struct {
        u16 count;
        u16 latch;
        bool running;
        bool toggle;
        bool pbout;
        bool oneShot;
    } timerB;

    u8 sdr;
    u8 icr;
    u8 imr;
    bool intLine;
    CounterInfo cnt;
    bool cntIntEnable;
    Cycle idleCycles;
    double idlePercentage;
}
CIAInfo;

#endif