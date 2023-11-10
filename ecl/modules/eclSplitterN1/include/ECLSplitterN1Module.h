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
#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <framework/gearbox/Unit.h>

/* Root headers */
#include <TH2F.h>

/* C++ headers. */
#include <vector>

namespace Belle2 {
  class ECLCalDigit;
  class ECLConnectedRegion;
  class ECLShower;
  class ECLLocalMaximum;
  class EventLevelClusteringInfo;
  class ECLnOptimal;

  namespace ECL {
    class ECLNeighbours;
    class ECLGeometryPar;
  }

  /** Class to perform the shower correction */
  class ECLSplitterN1Module : public Module {

  public:
    /** Constructor. */
    ECLSplitterN1Module();

    /** Destructor. */
    ~ECLSplitterN1Module();

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
    // Module parameters:

    // Splitter
    double m_threshold; /**< Local maximum threshold after splitting */
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

    /** nOptimal payload */
    DBObjPtr<ECLnOptimal> m_eclNOptimal;
    TH2F m_nOptimal2D; /**< 2D hist of nOptimal for Ebin vs groupID */
    std::vector<int> m_groupNumber; /**< group number for each crystal */
    const int m_nLeakReg = 3; /**< 3 ECL regions: 0 = forward, 1 = barrel, 2 = backward */
    int m_nEnergyBins = 0; /**< number of energies bins in nOptimal payload */
    std::vector< std::vector<float> > m_eBoundaries; /**< energy boundaries each region */

    // Position
    std::string m_positionMethod;  /**< Position calculation: lilo or linear */
    double m_liloParameterA; /**< lin-log parameter A */
    double m_liloParameterB; /**< lin-log parameter B */
    double m_liloParameterC; /**< lin-log parameter C */
    std::vector<double> m_liloParameters; /**< lin-log parameters A, B, and C */

    // Background
    int m_fullBkgdCount; /**< Number of expected background digits at full background, FIXME: move to database. */

    /** vector (ECLElementNumbers::c_NCrystals + 1 entries) with cell id to store array positions */
    std::vector< int > m_StoreArrPosition;

    /** vector (ECLElementNumbers::c_NCrystals + 1 entries) with cell id to store array positions for LM*/
    std::vector< int > m_StoreArrPositionLM;

    /** list with all cellid of this connected region */
    std::vector< int > m_cellIdInCR;

    /** Neighbour maps */
    ECL::ECLNeighbours* m_NeighbourMap9{nullptr}; /**< 3x3 = 9 neighbours */
    ECL::ECLNeighbours* m_NeighbourMap21{nullptr}; /**< 5x5 neighbours excluding corners = 21 */

    /** Store array: ECLCalDigit. */
    StoreArray<ECLCalDigit> m_eclCalDigits;

    /** Store array: ECLConnectedRegion. */
    StoreArray<ECLConnectedRegion> m_eclConnectedRegions;

    /** Store array: ECLShower. */
    StoreArray<ECLShower> m_eclShowers;

    /** Store array: ECLLocalMaximum. */
    StoreArray<ECLLocalMaximum> m_eclLocalMaximums;

    /** Store object pointer: EventLevelClusteringInfo. */
    StoreObjPtr<EventLevelClusteringInfo> m_eventLevelClusteringInfo;

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

    /** Name to be used for default option: EventLevelClusteringInfo.*/
    virtual const char* eventLevelClusteringInfoName() const
    { return "EventLevelClusteringInfo" ; }

    /** Geometry */
    ECL::ECLGeometryPar* m_geom{nullptr};

    /** Split connected region into showers. */
    void splitConnectedRegion(ECLConnectedRegion& aCR);

    /** Get number of neighbours based on first energy estimation and background level per event. */
    int getNeighbourMap(const double energy, const double background);

    /** Get optimal number of digits (out of 21) based on first energy estimation and background level per event. */
    std::vector<int> getOptimalNumberOfDigits(const int cellid, const double energy);

    /** Get energy sum for weighted entries. */
    double getEnergySum(std::vector < std::pair<double, double> >& weighteddigits, const unsigned int n);

    /** Estimate energy using 3x3 around central crystal. */
    double estimateEnergy(const int centerid);

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

    /** Name to be used for PureCsI option: EventLevelClusteringInfoPureCsI.*/
    virtual const char* eventLevelClusteringInfoName() const override
    { return "EventLevelClusteringInfoPureCsI" ; }

  }; // end of ECLSplitterN1PureCsIModule

} // end of Belle2 namespace
