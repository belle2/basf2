/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMSTATUS_H
#define BKLMSTATUS_H

//! Definition of each bit in barrel KLM status word (in BKLMDigit etc)
//! Definition of bit positions/masks for construction of detector-module ID

//! BKLM hit is unused
#define STATUS_UNUSED 0x00000000

//! BKLM hit originated from MC simulation (rather than real data)
#define STATUS_MC 0x40000000

//! BKLM MC-originated hit should be ignored (detector efficiency cut)
#define STATUS_MCKILL 0x20000000

//! BKLM hit represents the decay point of a simulated particle
#define STATUS_DECAYED 0x20000000

//! BKLM hit is located in an RPC (rather than a scintillator)
#define STATUS_INRPC 0x00000001

// BKLM hit is outside the in-time coincidence window
#define STATUS_OUTOFTIME 0x00000002

// BKLM hit has been assigned to a track
#define STATUS_ONTRACK 0x00000004

//! BKLM moduleID's bit position for detector end [0..1]; forward is 0
#define MODULE_END_BIT 14

//! BKLM moduleID's bit position for sector-1 [0..7]; 0 is on the +x axis and 2 is on the +y axis
#define MODULE_SECTOR_BIT 11

//! BKLM moduleID's bit position for layer-1 [0..14]; 0 is innermost
#define MODULE_LAYER_BIT 7

//! BKLM moduleID's bit position for plane-1 [0..1]; 0 is inner-plane and phiReadout plane
#define MODULE_PLANE_BIT 6

//! BKLM moduleID's bit position for strip-1 [0..95]; always 0
#define MODULE_STRIP_BIT 0

//! BKLM moduleID's mask for detector end [0..1]; forward is 0
#define MODULE_END_MASK (1<<MODULE_END_BIT)

//! BKLM moduleID's bit position for sector-1 [0..7]; 0 is on the +x axis and 2 is on the +y axis
#define MODULE_SECTOR_MASK (7 << MODULE_SECTOR_BIT)

//! BKLM moduleID's bit position for layer-1 [0..15]; 0 is innermost and 14 is outermost
#define MODULE_LAYER_MASK (15 << MODULE_LAYER_BIT)

//! BKLM moduleID's bit position for plane-1 [0..1]; 0 is inner-plane and phiReadout plane
#define MODULE_PLANE_MASK (1 << MODULE_PLANE_BIT)

//! BKLM moduleID's bit position for strip-1 [0..63]; always 0
#define MODULE_STRIP_MASK (63 << MODULE_STRIP_BIT)

#endif //BKLMSTATUS_H
