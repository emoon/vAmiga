// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _FS_USERDIR_BLOCK_H
#define _FS_USERDIR_BLOCK_H

#include "FSBlock.h"

struct UserDirBlock : HashableBlock {
            
    // Name
    FSName name = FSName("");
    
    // Creation date
    FSTimeStamp created = FSTimeStamp();

    // Hash table storing references to other blocks
    FSHashTable hashTable = FSHashTable();

    // Reference to the parent block
    Block *parent = nullptr;
    
    //
    // Methods
    //
    
    UserDirBlock(const char *str) : name(FSName(str)) { };
    
    // Methods from Block class
    FSBlockType type() override { return FS_USERDIR_BLOCK; }
    virtual void dump() override;
    void write(u8 *dst) override;

    // Methods from HashableBlock class
    virtual u32 hashValue() override { return name.hashValue(); }
    bool matches(FSName &otherName) override { return name == otherName; }
};

#endif