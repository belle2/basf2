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

//! Definition of bit positions/masks for elements of detector-module ID.
//! Order must be (high bit) END-SECTOR-LAYER-PLANE-STRIP (low bit) for sorting in bklmReconstructor module.
//! Also, definition of each status bit/mask.

//! bit position for strip-1 [0..47]
#define BKLM_STRIP_BIT 0

//! bit position for plane-1 [0..1]; 0 is inner-plane and phiReadout plane
#define BKLM_PLANE_BIT 6

//! bit position for layer-1 [0..14]; 0 is innermost
#define BKLM_LAYER_BIT 7

//! bit position for sector-1 [0..7]; 0 is on the +x axis and 2 is on the +y axis
#define BKLM_SECTOR_BIT 11

//! bit position for detector end [0..1]; forward is 0
#define BKLM_END_BIT 14

//! bit position for maxStrip-1 [0..47]
#define BKLM_MAXSTRIP_BIT 15

//! bit position for inRPC flag
#define BKLM_INRPC_BIT 21

//! bit position for MC-generated hit
#define BKLM_MC_BIT 22

//! bit position for MC-generated hit that represents a particle-decay point
#define BKLM_DECAYED_BIT 23

//! bit position for out-of-time-window flag (from BKLM hit reconstruction)
#define BKLM_OUTOFTIME_BIT 24

//! bit position for MC-generated hit lost due to inefficiency (from BKLM hit reconstruction)
#define BKLM_INEFFICIENT_BIT 25

//! bit position for above-threshold flag (scintillators only, from BKLM hit reconstruction)
#define BKLM_ABOVETHRESHOLD_BIT 26

//! bit position for used-on-track flag (from tracking muid reconstruction)
#define BKLM_ONTRACK_BIT 27

//! bit position for ghost-hit flag (from tracking muid reconstruction)
#define BKLM_GHOST_BIT 28

//! bit mask for strip-1 [0..47]
#define BKLM_STRIP_MASK (63 << BKLM_STRIP_BIT)

//! bit mask for plane-1 [0..1]; 0 is inner-plane and phiReadout plane
#define BKLM_PLANE_MASK (1 << BKLM_PLANE_BIT)

//! bit mask for layer-1 [0..15]; 0 is innermost and 14 is outermost
#define BKLM_LAYER_MASK (15 << BKLM_LAYER_BIT)

//! bit mask for sector-1 [0..7]; 0 is on the +x axis and 2 is on the +y axis
#define BKLM_SECTOR_MASK (7 << BKLM_SECTOR_BIT)

//! bit mask for detector end [0..1]; forward is 0
#define BKLM_END_MASK (1<<BKLM_END_BIT)

//! bit mask for maxStrip-1 [0..47]
#define BKLM_MAXSTRIP_MASK (63 << BKLM_MAXSTRIP_BIT)

//! bit mask for inRPC flag
#define BKLM_INRPC_MASK (1 << BKLM_INRPC_BIT)

//! bit mask for MC-generated hit
#define BKLM_MC_MASK (1 << BKLM_MC_BIT)

//! bit mask for MC-generated hit that represents a particle-decay point
#define BKLM_DECAYED_MASK (1 << BKLM_DECAYED_BIT)

//! bit mask for out-of-time-window flag (from BKLM hit reconstruction)
#define BKLM_OUTOFTIME_MASK (1 << BKLM_OUTOFTIME_BIT)

//! bit mask for MC-generated hit lost due to inefficiency (from BKLM hit reconstruction)
#define BKLM_INEFFICIENT_MASK (1 << BKLM_INEFFICIENT_BIT)

//! bit mask for above-threshold flag (scintillators only, from BKLM hit reconstruction)
#define BKLM_ABOVETHRESHOLD_MASK (1 << BKLM_ABOVETHRESHOLD_BIT)

//! bit mask for used-on-track flag (from tracking muid reconstruction)
#define BKLM_ONTRACK_MASK (1 << BKLM_ONTRACK_BIT)

//! bit mask for ghost-hit flag (from tracking muid reconstruction)
#define BKLM_GHOST_MASK (1 << BKLM_GHOST_BIT)

//! bit mask for module identifier
#define BKLM_MODULEID_MASK (BKLM_END_MASK | BKLM_SECTOR_MASK | BKLM_LAYER_MASK)

//! bit mask for module-and-strip identifier
#define BKLM_MODULESTRIPID_MASK (BKLM_END_MASK | BKLM_SECTOR_MASK | BKLM_LAYER_MASK | BKLM_PLANE_MASK | BKLM_STRIP_MASK)

//! bit mask for status bits
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
