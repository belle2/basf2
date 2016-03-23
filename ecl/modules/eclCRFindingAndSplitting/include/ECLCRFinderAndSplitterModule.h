/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * The "old" Belle II eclRecShower within a combined                      *
 * Connected Region Finder and Splitter.                                  *
 *                                                                        *
 *  THIS IS AN INTERMEDIATE STEP TOWARDS THE FULL REFACTORING!            *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca) (TF)               *
 *                                                                        *
 * Original code: Poyuan Chen, Vishal                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLCRFINDERANDSPLITTERMODULE_H_
#define ECLCRFINDERANDSPLITTERMODULE_H_

#include <framework/core/Module.h>
#include <ecl/utility/ECLShowerId.h>

namespace Belle2 {
  namespace ECL {

    /** Class to perform the shower correction */
    class ECLCRFinderAndSplitterModule : public Module {

    public:
      /** Constructor. */
      ECLCRFinderAndSplitterModule();

      /** Destructor. */
      ~ECLCRFinderAndSplitterModule();

      /** Initialize. */
      virtual void initialize();

      /** Begin run. */
      virtual void beginRun();

      /** Event. */
      virtual void event();

      /** End run. */
      virtual void endRun();

      /** Terminate. */
      virtual void terminate();

    private:
      /** CPU time     */
      double m_timeCPU;

      /** Run number   */
      int    m_nRun;

      /** Event number */
      int    m_nEvent;

      /** Utility unpacker of the shower id that contains CR, seed and hypothesis */
      ECLShowerId m_SUtility;


    public:
      /** We need names for the data objects to differentiate between PureCsI and default*/

      /** Default name ECLHitAssignments */
      virtual const char* eclHitAssignmentArrayName() const
      { return "ECLHitAssignments" ; }

      /** Default name ECLShowers */
      virtual const char* eclShowerArrayName() const
      { return "ECLShowers" ; }

      /** Default name ECLCalDigits */
      virtual const char* eclCalDigitArrayName() const
      { return "ECLCalDigits" ; }

      /** Default name ECLConnectedRegions */
      virtual const char* eclConnectedRegionArrayName() const
      { return "ECLConnectedRegions" ; }
    }; // end of ECLCRFinderAndSplitterModule


    /** The very same module but for PureCsI */
    class ECLCRFinderAndSplitterPureCsIModule : public ECLCRFinderAndSplitterModule {
    public:
      /** PureCsI name ECLHitAssignmentsPureCsI */
      virtual const char* eclHitAssignmentArrayName() const override
      { return "ECLHitAssignmentsPureCsI" ; }

      /** PureCsI name ECLShowersPureCsI */
      virtual const char* eclShowerArrayName() const override
      { return "ECLShowersPureCsI" ; }

      /** PureCsI name ECLCalDigitsPureCsI */
      virtual const char* eclCalDigitArrayName() const override
      { return "ECLCalDigitsPureCsI" ; }

      /** PureCsI name ECLConnectedRegionsPureCsI */
      virtual const char* eclConnectedRegionArrayName() const override
      { return "ECLConnectedRegionsPureCsI" ; }
    }; // end of ECLCRFinderAndSplitterPureCsIModule

  } // end of ECL namespace
} // end of Belle2 namespace

#endif
