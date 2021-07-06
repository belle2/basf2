/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

//STL
#include <vector>

// FRAMEWORK
#include <framework/core/Module.h>
#include <framework/gearbox/Unit.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

class TGraph2D;
class TFile;
class TH1F;

namespace Belle2 {
  class ECLCalDigit;
  class ECLConnectedRegion;
  class ECLShower;
  class ECLLocalMaximum;
  class EventLevelClusteringInfo;

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

    std::string m_fileBackgroundNormName; /**< Background normalization filename. */
    std::string m_fileNOptimalFWDName; /**< FWD number of optimal neighbours filename. */
    std::string m_fileNOptimalBarrelName; /**< Barrel number of optimal neighbours filename. */
    std::string m_fileNOptimalBWDName; /**< BWD number of optimal neighbours filename. */
    TFile* m_fileBackgroundNorm{nullptr}; /**< Background normalization file. */
    TFile* m_fileNOptimalFWD{nullptr}; /**< FWD number of optimal neighbours. */
    TFile* m_fileNOptimalBarrel{nullptr}; /**< Barrel number of optimal neighbours. */
    TFile* m_fileNOptimalBWD{nullptr}; /**< BWD number of optimal neighbours. */
    TH1F* m_th1fBackgroundNorm{nullptr}; /**< Background normalization histogram. */
    TGraph2D* m_tg2dNOptimalFWD[13][9] {}; /**< Array of 2D graphs used for interpolation between background and energy. */
    TGraph2D* m_tg2dNOptimalBWD[10][9] {}; /**< Array of 2D graphs used for interpolation between background and energy. */
    TGraph2D* m_tg2dNOptimalBarrel{nullptr}; /**< Array of 2D graphs used for interpolation between background and energy. */

    // Position
    std::string m_positionMethod;  /**< Position calculation: lilo or linear */
    double m_liloParameterA; /**< lin-log parameter A */
    double m_liloParameterB; /**< lin-log parameter B */
    double m_liloParameterC; /**< lin-log parameter C */
    std::vector<double> m_liloParameters; /**< lin-log parameters A, B, and C */

    // Background
    int m_fullBkgdCount; /**< Number of expected background digits at full background, FIXME: move to database. */

    const unsigned short c_nSectorCellIdBWD[10] = {9, 9, 6, 6, 6, 6, 6, 4, 4, 4}; /**< crystals per sector for theta rings */
    const unsigned short c_nSectorCellIdFWD[13] = {3, 3, 4, 4, 4, 6, 6, 6, 6, 6, 6, 9, 9}; /**< crystals per sector for theta rings */

    // Crystals per Ring
    const unsigned short c_crystalsPerRing[69] = {48, 48, 64, 64, 64, 96, 96, 96, 96, 96, 96, 144, 144, //FWD (13)
                                                  144, 144, 144, 144, 144, 144, 144,  // BARREL 20
                                                  144, 144, 144, 144, 144, 144, 144, 144, 144, 144, //30
                                                  144, 144, 144, 144, 144, 144, 144, 144, 144, 144, //40
                                                  144, 144, 144, 144, 144, 144, 144, 144, 144, 144, //50
                                                  144, 144, 144, 144, 144, 144, 144, 144, 144,//59
                                                  144, 144, 96, 96, 96, 96, 96, 64, 64, 64
                                                 }; /**< Number of crystals per theta ring. */

    /** vector (8736+1 entries) with cell id to store array positions */
    std::vector< int > m_StoreArrPosition;

    /** vector (8736+1 entries) with cell id to store array positions for LM*/
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
    unsigned int getOptimalNumberOfDigits(const int cellid, const double energy, const double background);

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
