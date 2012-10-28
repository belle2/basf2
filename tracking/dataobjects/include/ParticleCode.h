/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/logging/Logger.h>

namespace Belle2 {
  /** Blah blah blah
   */
  class ParticleCode {
  public:
    /** Number of particles in the enum.
     */
    enum {NParticle = 5};

    /** Constants for the enum.
     */
    static const unsigned int s_chargedStables[NParticle];

    /** Enumeration for stable charged particles.
     *
     *  This is the list of particles we assume e.g. to have typically fits for tracking.
     */
    enum EChargedStable {
      c_Electron = 11,   /** Electron pdg Code. */
      c_Muon     = 13,   /** Muon pdg Code.     */
      c_Pion     = 211,  /** Pion pdg Code.     */
      c_Kaon     = 321,  /** Kaon pdg Code.     */
      c_Proton   = 2212  /** Proton pdg Code.   */
    };

    /** Constructor, ...
     */
    ParticleCode(unsigned int particeCode = c_Pion);

    /** Nothing; nothing dynamically allocated here.
     */
    ~ParticleCode() {}

    /** Getter for ordering in lists, where you e.g. have first a pion info, then a kaon info etc.
     */
    unsigned int getIContinuous() const {
      return m_iCode;
    }

    /** Increment Operator to step through the known charged stables.
     */
    unsigned int operator ++() {
      if (!(++m_iCode < NParticle)) {
        B2ERROR("Requested increment above highest known PDG Code.");
      }
      return s_chargedStables[m_iCode];
    }

    /** Decrement Operator to step through the known charged stables.
     */
    unsigned int operator --() {
      if (!(--m_iCode > -1)) {
        B2ERROR("Requested decrement below 0.");
      }
      return s_chargedStables[m_iCode];
    }

    //--- Implicit conversions --------------------------------------------------------------------------------------------------
    /** Provide implicit conversion to unsigned int.
     *
     *  Various of this implicit conversions might be necessary as there is a maximum of one implicit conversions.
     */
    operator unsigned int () const {
      return s_chargedStables[m_iCode];
    }

    /** Provide implicit conversion to int.
     */
    operator int () const {
      return static_cast<int>(s_chargedStables[m_iCode]);
    }

    //---------------------------------------------------------------------------------------------------------------------------
  private:
    /** Storing the actual PDG code.
     */
    int m_iCode;
  };
}
