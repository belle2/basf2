/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* Basf2 headers. */
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {
  class ECLShower;
  class ECLLocalMaximum;
  class ECLConnectedRegion;
  class ECLCalDigit;

  /** Class to perform the shower correction */
  class ECLSplitterN2Module : public Module {

  public:
    /** Constructor. */
    ECLSplitterN2Module();

    /** Destructor. */
    ~ECLSplitterN2Module();

    /** Initialize. */
    virtual void initialize() override;

    /** Begin run. */
    virtual void beginRun() override;

    /** Event. */
    virtual void event() override;

    /** End run. */
    virtual void endRun() override;

    /** Terminate. */
    virtual void terminate() override;

  private:

    // Position
    std::string m_positionMethod;  /**< Position calculation: lilo or linear */
    double m_liloParameterA; /**< lin-log parameter A */
    double m_liloParameterB; /**< lin-log parameter B */
    double m_liloParameterC; /**< lin-log parameter C */
    std::vector< double > m_liloParameters; /**< lin-log parameters A, B, and C */

    // Background
    //const int c_fullBkgdCount = 280; /**< Number of expected background digits at full background, FIXME: ove to database. */

    /** Store array: ECLCalDigit. */
    StoreArray<ECLCalDigit> m_eclCalDigits;

    /** Store array: ECLConnectedRegion. */
    StoreArray<ECLConnectedRegion> m_eclConnectedRegions;

    /** Store array: ECLLocalMaximum. */
    StoreArray<ECLLocalMaximum> m_eclLocalMaximums;

    /** Store array: ECLShower. */
    StoreArray<ECLShower> m_eclShowers;

    /** Default name ECLCalDigits */
    virtual const char* eclCalDigitArrayName() const
    { return "ECLCalDigits" ; }

    /** Default name ECLConnectedRegions */
    virtual const char* eclConnectedRegionArrayName() const
    { return "ECLConnectedRegions" ; }

    /** Default name ECLLocalMaximums */
    virtual const char* eclLocalMaximumArrayName() const
    { return "ECLLocalMaximums" ; }

    /** Default name ECLShowers */
    virtual const char* eclShowerArrayName() const
    { return "ECLShowers" ; }

  }; // end of ECLSplitterN2Module


  /** The very same module but for PureCsI */
  class ECLSplitterN2PureCsIModule : public ECLSplitterN2Module {
  public:
    /** PureCsI name ECLCalDigitsPureCsI */
    virtual const char* eclCalDigitArrayName() const override
    { return "ECLCalDigitsPureCsI" ; }

    /** PureCsI name ECLConnectedRegionsPureCsI */
    virtual const char* eclConnectedRegionArrayName() const override
    { return "ECLConnectedRegionsPureCsI" ; }

    /** PureCsI name ECLLocalMaximumsPureCsI */
    virtual const char* eclLocalMaximumArrayName() const override
    { return "ECLLocalMaximumsPureCsI" ; }

    /** PureCsI name ECLShowersPureCsI */
    virtual const char* eclShowerArrayName() const override
    { return "ECLShowersPureCsI" ; }

  }; // end of ECLSplitterN2PureCsIModule

} // end of Belle2 namespace
