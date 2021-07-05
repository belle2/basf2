/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TRGTOPSlotTiming.cc
// Section  : TRG TOP
// Owner    : Tong Pang   Vladimir Savinov
// Email    : top16@pitt.edu, vladimirsavinov@gmail.com
//---------------------------------------------------------------
// Description : A class to represent TRG TOP slot-level timing decision
//---------------------------------------------------------------
// $Log$
//---------------------------------------------------------------

#include "trg/top/dataobjects/TRGTOPSlotTiming.h"

using namespace std;
using namespace Belle2;

void TRGTOPSlotTiming::setSlotId(int slotId) { m_slotId      = slotId     ; }
void TRGTOPSlotTiming::setSlotTiming(int slotTiming) { m_slotTiming  = slotTiming ; }
void TRGTOPSlotTiming::setSlotSegment(int slotSegment) { m_slotSegment = slotSegment; }
void TRGTOPSlotTiming::setSlotNHits(int slotNHits) { m_slotNHits   = slotNHits  ; }
void TRGTOPSlotTiming::setSlotLogL(int slotLogL) { m_slotLogL    = slotLogL   ; }
void TRGTOPSlotTiming::setSlotNErrors(int slotNErrors) { m_slotNErrors = slotNErrors; }
//void TRGTOPSlotTiming::set(int ) { m_ = ; }

// no longer needed (in ROOT v6)
//ClassImp(TRGTOPSlotTiming)



