// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

#define SPR_DMA(x) amiga->mem.peekChip16(amiga->agnus.sprpt[x])
#define INC_SPRDMAPTR(x) INC_DMAPTR(amiga->agnus.sprpt[x])

Denise::Denise()
{
    setDescription("Denise");
    
    registerSubcomponents(vector<HardwareComponent *> {
        
        &colorizer,
    });
    
    registerSnapshotItems(vector<SnapshotItem> {
        
        { &clock,         sizeof(clock),         0 },
        { &sprhstrt,      sizeof(sprhstrt),      WORD_ARRAY },
        { &sprShiftReg,   sizeof(sprShiftReg),   WORD_ARRAY },
        { &sprDmaState,   sizeof(sprDmaState),   DWORD_ARRAY },
        { &attach,        sizeof(attach),        0 },
        { &armed,         sizeof(armed),         0 },

        { &bplcon0,       sizeof(bplcon0),       0 },
        { &bplcon1,       sizeof(bplcon1),       0 },
        { &bplcon2,       sizeof(bplcon2),       0 },
        { &bpldat,        sizeof(bpldat),        WORD_ARRAY },
        { &sprdata,       sizeof(sprdata),       WORD_ARRAY },
        { &sprdatb,       sizeof(sprdatb),       WORD_ARRAY },

        { &joydat,        sizeof(joydat),        WORD_ARRAY },
        { &shiftReg,      sizeof(shiftReg),      DWORD_ARRAY },
        
        { &scrollLowEven, sizeof(scrollLowEven), 0 },
        { &scrollLowOdd,  sizeof(scrollLowOdd),  0 },
        { &scrollHiEven,  sizeof(scrollHiEven),  0 },
        { &scrollHiOdd,   sizeof(scrollHiOdd),   0 },

        { &pixel,         sizeof(pixel),         0 },

    });

    longFrame1.data = new int[HPIXELS * VPIXELS];
    longFrame2.data = new int[HPIXELS * VPIXELS];
    shortFrame1.data = new int[HPIXELS * VPIXELS];
    shortFrame2.data = new int[HPIXELS * VPIXELS];
}

Denise::~Denise()
{
    debug(2, "Destroying Denise[%p]\n", this);
}

void
Denise::_powerOn()
{
    clock = 0;
    workingLongFrame = &longFrame1;
    workingShortFrame = &shortFrame1;
    stableLongFrame = &longFrame2;
    stableShortFrame = &shortFrame2;
    frameBuffer = &longFrame1;
    pixel = 0;
    
    // Initialize frame buffers with a recognizable debug pattern
    for (unsigned line = 0; line < VPIXELS; line++) {
        for (unsigned i = 0; i < HPIXELS; i++) {
            longFrame1.data[line * HPIXELS + i] =
            longFrame2.data[line * HPIXELS + i] =
            shortFrame1.data[line * HPIXELS + i] =
            shortFrame2.data[line * HPIXELS + i] =
            (line % 2) ? 0x000000FF : 0x0000FFFF;
        }
    }
}

void
Denise::_powerOff()
{
    /* REMOVE AFTER DEBUGGING
    for (unsigned line = 0; line < VPIXELS; line++) {
        for (unsigned i = 0; i < HPIXELS; i++) {
            screenBuffer1[line * HPIXELS + i] =
            screenBuffer2[line * HPIXELS + i] = 0x000000FF;
        }
    }
    */
}

void
Denise::_reset()
{
    
}
void
Denise::_ping()
{
    
}

void
Denise::_inspect()
{
    // Prevent external access to variable 'info'
    pthread_mutex_lock(&lock);
    
    // Biplane information
    info.bplcon0 = bplcon0;
    info.bplcon1 = bplcon1;
    info.bplcon2 = bplcon2;
    
    for (unsigned i = 0; i < 32; i++)
    info.bpldat[i] = bpldat[i];
    
    for (unsigned i = 0; i < 32; i++)
    info.color[i] = colorizer.getRGBA(i);
    
    // Sprite information
    
    for (unsigned i = 0; i < 8; i++) {

        /* The sprite info is extracted from the pos and ctl values that are
         * recorded by the hsync handler at the beginning of rasterline 26.
         *
         * pos:  15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0  (Hx = HSTART)
         *       E7 E6 E5 E4 E3 E2 E1 E0 H8 H7 H6 H5 H4 H3 H2 H1  (Ex = VSTART)
         * ctl:  L7 L6 L5 L4 L3 L2 L1 L0 AT  -  -  -  - E8 L8 H0  (Lx = VSTOP)
         */
        uint16_t pos = info.sprite[i].pos;
        uint16_t ctl = info.sprite[i].ctl;

        info.sprite[i].hstrt = ((pos & 0x00FF) << 1) | (ctl & 0b001);
        info.sprite[i].vstrt = ((pos & 0xFF00) >> 8) | ((ctl & 0b100) << 6);
        info.sprite[i].vstop = ((ctl & 0xFF00) >> 8) | ((ctl & 0b010) << 7);
        info.sprite[i].attach = GET_BIT(ctl, 7);
    }

    pthread_mutex_unlock(&lock);
}

void
Denise::_dump()
{

}

DeniseInfo
Denise::getInfo()
{
    DeniseInfo result;
    
    pthread_mutex_lock(&lock);
    result = info;
    pthread_mutex_unlock(&lock);
    
    return result;
}

SpriteInfo
Denise::getSprInfo(int nr)
{
    SpriteInfo result;
    
    pthread_mutex_lock(&lock);
    result = info.sprite[nr];
    pthread_mutex_unlock(&lock);
    
    return result;
}

void
Denise::didLoadFromBuffer(uint8_t **buffer)
{
    // The values in the color registers may have changed. To make the change
    // visible, we need to recompute the RGBA patterns based on the new values.
    colorizer.updateColorTable();
}

uint16_t
Denise::peekJOY0DATR()
{
    return _port1->joydat();
}

uint16_t
Denise::peekJOY1DATR()
{
    return _port2->joydat();
}

void
Denise::pokeJOYTEST(uint16_t value)
{
    value     &= 0b1111110011111100;
    
    joydat[0] &= 0b0000001100000011;
    joydat[0] |= value;

    joydat[1] &= 0b0000001100000011;
    joydat[1] |= value;
}

void
Denise::pokeBPLCON0(uint16_t value)
{
    debug(2, "pokeBPLCON0(%X)\n", value);
    
    bplcon0 = value;
    
    // Tell Agnus how many bitplanes we have
    _agnus->activeBitplanes = bplconBPU();  // (value >> 12) & 0b111;

    // Update the DMA time slot allocation table (hires / lores may change)
    _agnus->updateBitplaneDma();
    
    // Clear data registers of all inactive bitplanes
    for (int plane = 5; plane >= _agnus->activeBitplanes; plane--) {
        bpldat[plane] = 0;
    }
}

void
Denise::pokeBPLCON1(uint16_t value)
{
    debug(2, "pokeBPLCON1(%X)\n", value);
    
    bplcon1 = value & 0xFF;
    
    uint16_t ddfstrt = _agnus->ddfstrt;

    // Compute scroll values (adapted from WinFellow)
    scrollLowOdd  = (bplcon1        + (ddfstrt & 0b0100) ? 8 : 0) & 0x0F;
    scrollHiOdd   = ((scrollLowOdd  + (ddfstrt & 0b0010) ? 4 : 0) & 0x07) << 1;
    scrollLowEven = ((bplcon1 >> 4) + (ddfstrt & 0b0100) ? 8 : 0) & 0x0F;
    scrollHiEven  = ((scrollLowEven + (ddfstrt & 0b0010) ? 4 : 0) & 0x07) << 1;
}

void
Denise::pokeBPLCON2(uint16_t value)
{
    debug(2, "pokeBPLCON2(%X)\n", value);
    
    bplcon2 = value;
}

void
Denise::pokeBPLxDAT(int x, uint16_t value)
{
    assert(x < 6);
    debug(2, "pokeBPL%dDATA(%X)\n", x + 1, value);
    
    bpldat[x] = value;
}

void
Denise::pokeSPRxPOS(int x, uint16_t value)
{
    assert(x < 8);
    debug(2, "pokeSPR%dPOS(%X)\n", x, value);

    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0  (Ex = VSTART)
    // E7 E6 E5 E4 E3 E2 E1 E0 H8 H7 H6 H5 H4 H3 H2 H1  (Hx = HSTART)

    // Note: Denise only picks up the horizontal coordinate. Only Agnus knows
    // about the vertical coordinate.

    sprhstrt[x] = ((value & 0xFF) << 1) | (sprhstrt[x] & 0x01);
    
    // Update debugger info
    if (_agnus->vpos == 25) {
        info.sprite[x].pos = value;
    }
}

void
Denise::pokeSPRxCTL(int x, uint16_t value)
{
    assert(x < 8);
    debug(2, "pokeSPR%dCTL(%X)\n", x, value);
    
    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
    // L7 L6 L5 L4 L3 L2 L1 L0 AT  -  -  -  - E8 L8 H0  (Lx = VSTOP)

    // Note: Denise only picks up the horizontal coordinate. Only Agnus knows
    // about the vertical coordinate.
    
    sprhstrt[x] = (sprhstrt[x] & 0x1FE) | (value & 0x01);
    WRITE_BIT(attach, x, GET_BIT(value, 7));
    
    // Update debugger info
    if (_agnus->vpos == 25) {
        info.sprite[x].ctl = value;
        info.sprite[x].ptr = _agnus->sprpt[x];
    }
}

void
Denise::pokeSPRxDATA(int x, uint16_t value)
{
    assert(x < 8);
    debug(2, "pokeSPR%dDATA(%X)\n", x, value);
    
    sprdata[x] = value;
    armSprite(x);
}

void
Denise::pokeSPRxDATB(int x, uint16_t value)
{
    assert(x < 8);
    debug(2, "pokeSPR%dDATB(%X)\n", x, value);
    
    sprdatb[x] = value;
}

void
Denise::armSprite(int x)
{
    SET_BIT(armed, x);

    // sprShiftReg[x] = HI_W_LO_W(sprdatb[x], sprdata[x]);
}

/*
void
Denise::serveSprDma1Event(int x)
{
    assert(x < 8);
    
    // int16_t vpos = amiga->agnus.vpos;
    // vstrtCmp[x] = vpos == vstrt[x] && vpos >= 24;
    // vstopCmp[x] = vpos == vstop[x];

    switch(sprDmaState[x]) {

        case SPR_FETCH_CONFIG:

            debug(2, "serveSprDma1Event(%d, %X) SPR_FETCH_CONFIG\n", x, SPR_DMA(x));
            pokeSPRxPOS(x, SPR_DMA(x));
            INC_SPRDMAPTR(x);
            break;
            
        case SPR_WAIT_VSTART:
            
            debug(2, "serveSprDma1Event(%d, %X) SPR_WAIT_VSTART\n", x, SPR_DMA(x));
            if (inFirstSprLine(x)) {
                pokeSPRxDATB(x, SPR_DMA(x));
                INC_SPRDMAPTR(x);
            }
            break;

        case SPR_FETCH_DATA:
            
            debug(2, "serveSprDma1Event(%d, %X) SPR_FETCH_DATA\n", x, SPR_DMA(x));
            if (inLastSprLine(x)) {
                pokeSPRxPOS(x, SPR_DMA(x));
            } else {
                pokeSPRxDATB(x, SPR_DMA(x));
            }
            INC_SPRDMAPTR(x);
            break;
            
        default:
            assert(false);
    }
}

void
Denise::serveSprDma2Event(int x)
{
    assert(x < 8);

    switch(sprDmaState[x]) {
            
        case SPR_FETCH_CONFIG:
            
            debug(2, "serveSprDma2Event(%d, %X) SPR_FETCH_CONFIG\n", x, SPR_DMA(x));
            pokeSPRxCTL(x, SPR_DMA(x));
            INC_SPRDMAPTR(x);
            sprDmaState[x] = inFirstSprLine(x) ? SPR_FETCH_DATA : SPR_WAIT_VSTART;
            break;
            
        case SPR_WAIT_VSTART:
            
            debug(2, "serveSprDma2Event(%d, %X) SPR_WAIT_VSTART\n", x, SPR_DMA(x));
            if (inFirstSprLine(x)) {
                pokeSPRxDATA(x, SPR_DMA(x));
                INC_SPRDMAPTR(x);
                sprDmaState[x] = inLastSprLine(x) ? SPR_FETCH_CONFIG : SPR_FETCH_DATA;
            }
            break;
            
        case SPR_FETCH_DATA:
            
            debug(2, "serveSprDma2Event(%d, %X) SPR_FETCH_DATA\n", x, SPR_DMA(x));
            if (inLastSprLine(x)) {
                pokeSPRxCTL(x, SPR_DMA(x));
            } else {
                pokeSPRxDATB(x, SPR_DMA(x));
            }
            INC_SPRDMAPTR(x);
            sprDmaState[x] = inFirstSprLine(x) ? SPR_FETCH_DATA : SPR_WAIT_VSTART;
            break;
            
        default:
            assert(false);
    }
}
*/


/*
void
Denise::serviceEvent(EventID id, int64_t data)
{
  
}
*/

void
Denise::fillShiftRegisters()
{
    // warn("fillShiftRegisters: IMPLEMENTATION MISSING (vpos = %d)\n", amiga->agnus.vpos);
    // warn("blpdat: %X %X %X %X\n", bpldat[0], bpldat[1], bpldat[2], bpldat[3]);

    shiftReg[0] = bpldat[0];
    shiftReg[1] = bpldat[1];
    shiftReg[2] = bpldat[2];
    shiftReg[3] = bpldat[3];
    shiftReg[4] = bpldat[4];
    shiftReg[5] = bpldat[5];

    // draw16();
}

int *
Denise::pixelAddr(int pixel)
{
    assert(pixel < HPIXELS);
    assert(_agnus->vpos >= 26); // 0 .. 25 is VBLANK area

    int offset = pixel + (_agnus->vpos - 26) * HPIXELS;
    // debug("pixel offset for pixel %d is %d\n", pixel, offset);
    assert(offset < VPIXELS * HPIXELS);
    
    return frameBuffer->data + offset;
}

void
Denise::draw16()
{
    int *ptr = pixelAddr(pixel);
    
    for (int i = 0; i < 16; i++) {
        
        // Read a bit slice
        uint8_t index =
        ((shiftReg[0] & 0x8000) >> 15) |
        ((shiftReg[1] & 0x8000) >> 14) |
        ((shiftReg[2] & 0x8000) >> 13) |
        ((shiftReg[3] & 0x8000) >> 12) |
        ((shiftReg[4] & 0x8000) >> 11) |
        ((shiftReg[5] & 0x8000) >> 10);
        
        for (unsigned j = 0; j < 6; j++) {
            shiftReg[j] <<= 1;
        }
        
        // Draw a single hires pixel
        uint32_t rgba = colorizer.getRGBA(index);
        *ptr++ = rgba;
    }
    pixel += 16;
}

void
Denise::draw32()
{
    if (ham()) { draw32HAM(); return; }
    
    int *ptr = pixelAddr(pixel);
    
    for (int i = 0; i < 16; i++) {
        
        // Read a bit slice
        uint8_t index =
        ((shiftReg[0] & 0x8000) >> 15) |
        ((shiftReg[1] & 0x8000) >> 14) |
        ((shiftReg[2] & 0x8000) >> 13) |
        ((shiftReg[3] & 0x8000) >> 12) |
        ((shiftReg[4] & 0x8000) >> 11) |
        ((shiftReg[5] & 0x8000) >> 10);
        
        for (unsigned j = 0; j < 6; j++) {
            shiftReg[j] <<= 1;
        }
        
        // Draw two lores pixels
        uint32_t rgba = colorizer.getRGBA(index);
        *ptr++ = rgba;
        *ptr++ = rgba;
    }
    pixel += 32;
}

void
Denise::draw32HAM()
{
    // uint16_t colReg;

    int *ptr = pixelAddr(pixel);

    for (int i = 0; i < 16; i++) {

        // Read a bit slice
        uint8_t index =
        ((shiftReg[0] & 0x8000) >> 15) |
        ((shiftReg[1] & 0x8000) >> 14) |
        ((shiftReg[2] & 0x8000) >> 13) |
        ((shiftReg[3] & 0x8000) >> 12) |
        ((shiftReg[4] & 0x8000) >> 11) |
        ((shiftReg[5] & 0x8000) >> 10);

        for (unsigned j = 0; j < 6; j++) {
            shiftReg[j] <<= 1;
        }

        // Draw two lores pixels
        uint32_t rgba = colorizer.computeHAM(index);
        *ptr++ = rgba;
        *ptr++ = rgba;
/*
        switch (index >> 4) {

            case 0b00: // Get color from register

                assert(index < 16);
                colReg = colorizer.peekColorReg(index);
                r = (colReg >> 8) & 0b1111;
                g = (colReg >> 4) & 0b1111;
                b = (colReg >> 0) & 0b1111;
                break;

            case 0b01: // Modify blue

                b = (index & 0b1111);
                // r = 0; g = 0; b = 15;
                break;

            case 0b10: // Modify red

                r = (index & 0b1111);
                // r = 15; g = 0; b = 0;
                break;

            case 0b11: // Modify green

                g = (index & 0b1111);
                // r = 0; g = 0; b = 0;
                break;

            default:

                assert(false);
        }

        assert(r < 16);
        assert(g < 16);
        assert(b < 16);
        uint32_t rgba = HI_HI_LO_LO(0xFF, b << 4, g << 4, r << 4);
        *ptr++ = rgba;
        *ptr++ = rgba;
 */
    }
    pixel += 32;
}

void
Denise::drawLeftBorder()
{
    if (_agnus->hpos < 0x35) return; // HBLANK area
    
    // Assign the horizontal pixel counter
    // pixel = (amiga->agnus.hpos - (0x35 - 0xF)) * 4;
    pixel = (_agnus->hpos - 0x35) * 4;

    // Fill the beginning of the line with the current background color
    int bgcol = colorizer.getRGBA(0);
    int *ptr  = pixelAddr(0);
    int *end  = pixelAddr(pixel);
    
    while (ptr < end) *ptr++ = bgcol;
}

void
Denise::drawRightBorder()
{
    // Fill the rest of the line with the current background color
    int bgcol = colorizer.getRGBA(0);
    int *ptr  = pixelAddr(pixel);
    int *end  = pixelAddr(HPIXELS - 2);
    
    while (ptr <= end) *ptr++ = bgcol;
    
    /* In the last pixel of each line, we store if this line was drawn in lores
     * or hires mode. This information is used by the uscaling shader to
     * determine if in-texture scaling should be applied.
     */
    *ptr = lores() ? 0x00FFFF00 : 0;
    
    // Reset the horizontal pixel counter
    pixel = 0;
}

void
Denise::drawSprites()
{
    int nr = 0;
    
    // EXPERIMENTAL
    while (armed != 0) {
        
        if (armed & 0x1) {
            
            int16_t pixel = 2 * sprhstrt[nr] - (0x35 * 4);
            if (pixel >= HPIXELS - 33) { pixel = HPIXELS - 33; }
            int *ptr = pixelAddr(pixel);
            
            int rgba[4];
            rgba[1] = colorizer.getRGBA(17);
            rgba[2] = colorizer.getRGBA(18);
            rgba[3] = colorizer.getRGBA(19);
            
            for (int i = 15; i >= 0; i--) {
                
                int colNr = !!GET_BIT(sprdata[nr], i) << 1;
                colNr |=    !!GET_BIT(sprdatb[nr], i);
                
                if (colNr) {
                    *ptr++ = rgba[colNr];
                    *ptr++ = rgba[colNr];
                } else {
                    ptr += 2;
                }
            }
        }
        armed >>= 1;
        nr++;
    }
}

void
Denise::endOfLine()
{
    // debug("endOfLine pixel = %d HPIXELS = %d\n", pixel, HPIXELS);
    
    if (_agnus->vpos >= 26) {
        
        // Fill the rest of the current line with the background color.
        drawRightBorder();
        
        // Draw sprites if one or more of them is armed.
        if (armed) drawSprites();
    }
}

void
Denise::prepareForNextFrame(bool longFrame, bool interlace)
{
    assert(workingLongFrame == &longFrame1 || workingLongFrame == &longFrame2);
    assert(workingShortFrame == &shortFrame1 || workingShortFrame == &shortFrame2);
    assert(stableLongFrame == &longFrame1 || stableLongFrame == &longFrame2);
    assert(stableShortFrame == &shortFrame1 || stableShortFrame == &shortFrame2);
    assert(workingLongFrame != stableLongFrame);
    assert(workingShortFrame != stableShortFrame);
    assert(frameBuffer == workingLongFrame || frameBuffer == workingShortFrame);

    // pthread_mutex_lock(&lock);

    if (frameBuffer == &longFrame1 || frameBuffer == &longFrame2) {

        workingLongFrame = stableLongFrame;
        stableLongFrame = frameBuffer;
        frameBuffer = interlace ? workingShortFrame : workingLongFrame;

    } else {

        assert(frameBuffer == &shortFrame1 || frameBuffer == &shortFrame2);
        workingShortFrame = stableShortFrame;
        stableShortFrame = frameBuffer;
        frameBuffer = workingLongFrame;

    }

    // debug("long = %d interlace = %d\n", frameBuffer->longFrame, frameBuffer->interlace);
    frameBuffer->longFrame = longFrame;
    frameBuffer->interlace = interlace;

    // pthread_mutex_unlock(&lock);
}


void
Denise::debugSetActivePlanes(int count)
{
    assert(count >= 0 && count <= 6);
    
    _amiga->suspend();
    
    uint16_t value = bplcon0 & 0b1000111111111111;
    pokeBPLCON0(value | (count << 12));
    
    _amiga->resume();
}

void
Denise::debugSetBPLCON0Bit(unsigned bit, bool value)
{
    assert(bit <= 15);
    
    _amiga->suspend();
    
    uint16_t mask = 1 << bit;
    pokeBPLCON0(value ? (bplcon0 | mask) : (bplcon0 & ~mask));
    
    _amiga->resume();
}
