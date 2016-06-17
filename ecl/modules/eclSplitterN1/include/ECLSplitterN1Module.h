/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Main reconstruction splitter code for the N1 hypothesis.               *
 * Based on a connected region (CR) we look for local maxima and          *
 * create one shower for each local maximum (LM). In case of multiple     *
 * LM in one CR the energy is shared between the showers based on         *
 * their exponentially weighted distance in an iterative procedure. If    *
 * a CR has no LM the highest energetic digit in the CR is taken as LM.   *
 * The position is reconstructed using logarithmic weights for not too    *
 * small shower and linear weights otherwise ('lilo').                    *
 *                                                                        *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca) (TF)               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLSPLITTERN1MODULE_H_
#define ECLSPLITTERN1MODULE_H_

// ECL
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLConnectedRegion.h>
#include <ecl/dataobjects/ECLLocalMaximum.h>
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLEventInformation.h>
#include <ecl/utility/ECLShowerId.h>

// FRAMEWORK
#include <framework/core/Module.h>
#include <framework/gearbox/Unit.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// GEOMETRY
#include <ecl/geometry/ECLNeighbours.h>
#include <ecl/geometry/ECLGeometryPar.h>

// ROOT
#include "TH1F.h"
#include "TFile.h"
#include "TTree.h"
#include "TGraph2D.h"

#include <vector>    // std::vector

namespace Belle2 {
  namespace ECL {

    /** Class to perform the shower correction */
    class ECLSplitterN1Module : public Module {

    public:
      /** Constructor. */
      ECLSplitterN1Module();

      /** Destructor. */
      ~ECLSplitterN1Module();

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
      // Module parameters:
      // Local Maximum
      double m_lmEnergyCut; /**< Local maximum energy cut. */
      double m_lmEnergyNeighbourCut; /**< Neighbours of maximum energy cut. */
      std::string m_lmMethod; /**< LM Method. */
      // Local Maximum, Method specific parameters: BaBar method
      double m_lmCutOffset; /**< cut offset */
      double m_lmCutSlope; /**< cut slope. */
      double m_lmRatioCorrection; /**< correction for nominator and denominator of the ratio. */
      // from BaBar:
      //                                            ^
      //                                            |  ....             /
      //                                         1.0|   ....           /|
      //                                            |                 / | fCutSlope
      //                                            |  splitoffs     /  |
      //          MaxE of neighbours - fERatioCorr  |    and        /___|
      // ERatio =  -------------------------------  |  hadrons     /..
      //                    MaxE-fERatioCorr        |             / ..
      //                                            |  ....      /  ..
      //                                            |   ....    /  ...  <-- merged pions
      //                                            |          /  ....      and photons
      //                                            |  ...    /   ....
      //                                         0.0|        /    ....
      //                                            |------------------->
      //                                             0     /     6 7 8
      //                                            <------->             number of neighbours
      //                                            fCutOffset             with energy > fNeighbourECut

      // Splitter
      double m_expConstant; /**< Constant a from exp(-a*dist/RM), 1.5 to 2.5 */
      int m_maxIterations; /**< Maximum number of iterations */
      double m_shiftTolerance; /**< Tolerance level for centroid shifts. */
      double m_minimumSharedEnergy; /**< Minimum shared energy. */
      int m_maxSplits; /**< Maximum number of splits */
      const double c_molierRadius = 3.581 *
                                    Belle2::Unit::cm; /**< Constant RM (Molier Radius) from exp(-a*dist/RM), http://pdg.lbl.gov/2009/AtomicNuclearProperties/HTML_PAGES/141.html */

      double m_cutDigitEnergyForEnergy; /**< Minimum digit energy to be included in the shower energy calculation*/
      double m_cutDigitTimeResidualForEnergy; /**< Maximum time residual to be included in the shower energy calculation*/
      int m_useOptimalNumberOfDigitsForEnergy; /**< Optimize the number of neighbours for energy calculations */
      TGraph2D* m_tg2OptimalNumberOfDigitsForEnergy; /**< 2D graph used for interpolation between background and energy. */

      // Position
      std::string m_positionMethod;  /**< Position calculation: lilo or linear */
      double m_liloParameterA; /**< lin-log parameter A */
      double m_liloParameterB; /**< lin-log parameter B */
      double m_liloParameterC; /**< lin-log parameter C */
      std::vector<double> m_liloParameters; /**< lin-log parameters A, B, and C */

      // Background
      const int c_fullBkgdCount = 280; /**< Number of expected background digits at full background, FIXME: ove to database. */

      /** vector (8736+1 entries) with cell id to store array positions */
      std::vector< int > m_StoreArrPosition;

      /** list with all cellid of this connected region */
      std::vector< int > m_cellIdInCR;

      /** Neighbour maps */
      ECLNeighbours* m_NeighbourMap9; /**< 3x3 = 9 neighbours */
      ECLNeighbours* m_NeighbourMap21; /**< 5x5 neighbours excluding corners = 21 */

      /** Store array: ECLCalDigit. */
      StoreArray<ECLCalDigit> m_eclCalDigits;

      /** Store array: ECLConnectedRegion. */
      StoreArray<ECLConnectedRegion> m_eclConnectedRegions;

      /** Store array: ECLShower. */
      StoreArray<ECLShower> m_eclShowers;

      /** Store array: ECLLocalMaximum. */
      StoreArray<ECLLocalMaximum> m_eclLocalMaximums;

      /** Store object pointer: ECLEventInformation. */
      StoreObjPtr<ECLEventInformation> m_eclEventInformation;

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

      /** Name to be used for default option: ECLEventInformation.*/
      virtual const char* eclEventInformationName() const
      { return "ECLEventInformation" ; }

      /** Geometry */
      ECLGeometryPar* m_geom;

      /** Make local maximas for a given connected region. */
//      void makeLocalMaximums(ECLConnectedRegion& aCR, std::map < int, ECLCalDigit>& cellIdToDigitMap);
      void makeLocalMaximums(ECLConnectedRegion& aCR);

      /** Split connected region into showers. */
//      void splitConnectedRegion(ECLConnectedRegion& aCR, std::map < int, ECLCalDigit>& cellIdToDigitMap);
      void splitConnectedRegion(ECLConnectedRegion& aCR);

      /** Get number of neighbours based on first energy estimation and background level per event. */
      int getNeighbourMap(const double energy, const double background);

      /** Get optimal number of digits (out of 21) based on first energy estimation and background level per event. */
      unsigned int getOptimalNumberOfDigits(const double energy, const double background);

      /** Get energy sum for weighted entries. */
      double getEnergySum(std::vector < std::pair<double, double> >& weighteddigits, const unsigned int n);

      /** Estimate energy using 3x3 around central crystal. */
//      int estimateEnergy(const int centerid, std::map < int, ECLCalDigit>& cellIdToDigitPointerMap);
      int estimateEnergy(const int centerid);

    }; // end of ECLSplitterN1Module


    /** The very same module but for PureCsI */
    class ECLSplitterN1PureCsIModule : public ECLSplitterN1Module {
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

      /** Name to be used for PureCsI option: ECLEventInformationPureCsI.*/
      virtual const char* eclEventInformationName() const override
      { return "ECLEventInformationPureCsI" ; }

    }; // end of ECLSplitterN1PureCsIModule

  } // end of ECL namespace
} // end of Belle2 namespace

#endif
