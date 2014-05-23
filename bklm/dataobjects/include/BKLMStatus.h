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

//! Definition of bit positions/masks for construction of detector-module ID
//! Order must be (highest bit) END-SECTOR-LAYER-PLANE-STRIP (lowest bit) for reconstruction sorting
//! Definition of each bit in barrel KLM status word (in BKLMDigit etc)

//! BKLM moduleID's bit position for strip-1 [0..47]
#define BKLM_STRIP_BIT 0

//! BKLM moduleID's bit position for plane-1 [0..1]; 0 is inner-plane and phiReadout plane
#define BKLM_PLANE_BIT 6

//! BKLM moduleID's bit position for layer-1 [0..14]; 0 is innermost
#define BKLM_LAYER_BIT 7

//! BKLM moduleID's bit position for sector-1 [0..7]; 0 is on the +x axis and 2 is on the +y axis
#define BKLM_SECTOR_BIT 11

//! BKLM moduleID's bit position for detector end [0..1]; forward is 0
#define BKLM_END_BIT 14

//! BKLM moduleID's bit position for maxStrip-1 [0..47]
#define BKLM_MAXSTRIP_BIT 15

//! BKLM moduleID's bit position for inRPC flag
#define BKLM_INRPC_BIT 21

//! BKLM moduleID's bit position for MC-generated hit
#define BKLM_MC_BIT 22

//! BKLM moduleID's bit position for MC-generated hit that represents a particle-decay point
#define BKLM_DECAYED_BIT 23

//! BKLM moduleID's bit position for out-of-time-window flag (from BKLM hit reconstruction)
#define BKLM_OUTOFTIME_BIT 24

//! BKLM moduleID's bit position for MC-generated hit lost due to inefficiency (from BKLM hit reconstruction)
#define BKLM_INEFFICIENT_BIT 25

//! BKLM moduleID's bit position for above-threshold flag (scintillators only, from BKLM hit reconstruction)
#define BKLM_ABOVETHRESHOLD_BIT 26

//! BKLM moduleID's bit position for used-on-track flag (from tracking muid reconstruction)
#define BKLM_ONTRACK_BIT 27

//! BKLM moduleID's bit mask for strip-1 [0..47]
#define BKLM_STRIP_MASK (63 << BKLM_STRIP_BIT)

//! BKLM moduleID's bit mask for plane-1 [0..1]; 0 is inner-plane and phiReadout plane
#define BKLM_PLANE_MASK (1 << BKLM_PLANE_BIT)

//! BKLM moduleID's bit mask for layer-1 [0..15]; 0 is innermost and 14 is outermost
#define BKLM_LAYER_MASK (15 << BKLM_LAYER_BIT)

//! BKLM moduleID's bit mask for sector-1 [0..7]; 0 is on the +x axis and 2 is on the +y axis
#define BKLM_SECTOR_MASK (7 << BKLM_SECTOR_BIT)

//! BKLM moduleID's bit mask for detector end [0..1]; forward is 0
#define BKLM_END_MASK (1<<BKLM_END_BIT)

//! BKLM moduleID's bit mask for maxStrip-1 [0..47]
#define BKLM_MAXSTRIP_MASK (63 << BKLM_MAXSTRIP_BIT)

//! BKLM moduleID's bit mask for inRPC flag
#define BKLM_INRPC_MASK (1 << BKLM_INRPC_BIT)

//! BKLM moduleID's bit mask for MC-generated hit
#define BKLM_MC_MASK (1 << BKLM_MC_BIT)

//! BKLM moduleID's bit mask for MC-generated hit that represents a particle-decay point
#define BKLM_DECAYED_MASK (1 << BKLM_DECAYED_BIT)

//! BKLM moduleID's bit mask for out-of-time-window flag (from BKLM hit reconstruction)
#define BKLM_OUTOFTIME_MASK (1 << BKLM_OUTOFTIME_BIT)

//! BKLM moduleID's bit mask for MC-generated hit lost due to inefficiency (from BKLM hit reconstruction)
#define BKLM_INEFFICIENT_MASK (1 << BKLM_INEFFICIENT_BIT)

//! BKLM moduleID's bit mask for used-on-track flag (from tracking muid reconstruction)
#define BKLM_ONTRACK_MASK (1 << BKLM_ONTRACK_BIT)

//! BKLM moduleID's bit mask for above-threshold flag (scintillators only, from BKLM hit reconstruction)
#define BKLM_ABOVETHRESHOLD_MASK (1 << BKLM_ABOVETHRESHOLD_BIT)

//! BKLM moduleID's bit mask for module identifier
#define BKLM_MODULEID_MASK (BKLM_END_MASK | BKLM_SECTOR_MASK | BKLM_LAYER_MASK)

//! BKLM moduleID's bit mask for module-and-strip identifier
#define BKLM_MODULESTRIPID_MASK (BKLM_END_MASK | BKLM_SECTOR_MASK | BKLM_LAYER_MASK | BKLM_PLANE_MASK | BKLM_STRIP_MASK)

//! BKLM moduleID's bit mask for status bits
#define BKLM_STATUS_MASK (BKLM_INRPC_MASK | BKLM_MC_MASK | BKLM_DECAYED_MASK | BKLM_OUTOFTIME_MASK | BKLM_INEFFICIENT_MASK | BKLM_ONTRACK_MASK | BKLM_ABOVETHRESHOLD_MASK)

//! BKLMHit2d Zstrips bit position for strip-1 [0..47]
#define BKLM_ZSTRIP_BIT 0

//! BKLMHit2d Zstrips bit position for maxStrip-1 [0..47]
#define BKLM_ZMAXSTRIP_BIT 6

//! BKLMHit2d Zstrips bit mask for strip-1 [0..47]
#define BKLM_ZSTRIP_MASK (63 << BKLM_ZSTRIP_BIT)

//! BKLMHit2d Zstrips bit mask for maxStrip-1 [0..47]
#define BKLM_ZMAXSTRIP_MASK (63 << BKLM_ZMAXSTRIP_BIT)

#endif //BKLMSTATUS_H
