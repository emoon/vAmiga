// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _JOYSTICK_INC
#define _JOYSTICK_INC

#include "AmigaComponent.h"

class Joystick : public AmigaComponent {

    // The control port this joystick is connected to
    const PortNr nr;

    // Button state
    bool button = false;
    
    // Horizontal joystick position (-1 = left, 1 = right, 0 = released)
    int axisX = 0;
    
    // Vertical joystick position (-1 = up, 1 = down, 0 = released)
    int axisY = 0;
    
    // Indicates whether multi-shot mode is enabled
    bool autofire = false;
    
    // Number of bullets per gun volley
    int autofireBullets = -3;
    
    // Autofire frequency in Hz
    float autofireFrequency = 2.5;
    
    // Bullet counter used in multi-fire mode
    i64 bulletCounter = 0;
    
    // Next frame to auto-press or auto-release the fire button
    Frame nextAutofireFrame = 0;
    
    
    //
    // Constructing and destructing
    //
    
public:
    
    Joystick(PortNr n, Amiga& ref);
    

    //
    // Iterating over snapshot items
    //

    template <class T>
    void applyToPersistentItems(T& worker)
    {
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
    }

    
    //
    // Methods from HardwareComponent
    //
    
private:

    void _reset() override;
    void _dump() override;
    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }
    size_t didLoadFromBuffer(u8 *buffer) override;
    
    
    //
    // Accessing properties
    //
    
public:
    
    // Auto-fire
    bool getAutofire() { return autofire; }
    void setAutofire(bool value);
    
    // Number of bullets per gun volley (a negative value means infinite)
    int getAutofireBullets() { return autofireBullets; }
    void setAutofireBullets(int value);
    
    // Autofire frequency
    float getAutofireFrequency() { return autofireFrequency; }
    void setAutofireFrequency(float value) { autofireFrequency = value; }

private:
    
    // Updates variable nextAutofireFrame
    void scheduleNextShot();


    //
    // Managing registers
    //

public:

    // Modifies the PRA bits of CIA A according to the current button state
    void changePra(u8 &pra);


    //
    // Using the device
    //
    
public:

    // Callback handler for function ControlPort::joydat()
    u16 joydat();

    // Callback handler for function ControlPort::ciapa()
    u8 ciapa();
    
    // Triggers a gamepad event
    void trigger(GamePadAction event);

    /* Execution function for this control port
     * This method needs to be invoked at the end of each frame to make the
     * auto-fire mechanism work.
     */
    void execute();
};

#endif