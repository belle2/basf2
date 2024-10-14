/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef CDCTRIGGER3DFITTERModule_H
#define CDCTRIGGER3DFITTERModule_H

#include "framework/core/Module.h"
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <framework/dataobjects/BinnedEventT0.h>

namespace Belle2 {
  /** Module for the 3D Fitter of the CDC trigger. */
  class CDCTrigger3DFitterModule : public Module {

  public:

    /** Constructor, for setting module description and parameters. */
    CDCTrigger3DFitterModule();

    // /** Destructor */
    // virtual ~CDCTrigger3DFitterModule();

    /** Initialize the module and register DataStore arrays. */
    virtual void initialize() override;

    /** Run the 3D fitter for an event. */
    virtual void event() override;

  private:
    // major functions

    /** Select 10 wires which crosses with the given 2D track. */
    std::vector<std::vector<CDCHit*>> preselector(double phi0, double omega);

    /** Calculate s(arc length of the 2D track) at the hit position. */
    std::vector<std::vector<double>> sConverter(const std::vector<std::vector<CDCHit*>>& preselectedHits, double omega);

    /** Calculate z at the hit position. */
    std::vector<std::vector<double>> zConverter(const std::vector<std::vector<CDCHit*>>& preselectedHits, double phi0, double omega);

    /** Class for the Hough voter. */
    class HoughVoter {
    private:
      const int nCellZ0 = 16;
      const int nCellCot = 16;
      const double minZ0 = -50;
      const double maxZ0 = 78;
      const double minCot = -0.8;
      const double maxCot = 1.5;
      const double cellWidthZ0 = (maxZ0 - minZ0) / nCellZ0;
      const double cellWidthCot = (maxCot - minCot) / nCellCot;

      // n-th layer's info is at n-th bit from LSB
      std::vector<std::vector<int>> votingCell;

      inline int digitizeZ0(double z0) {return floor((maxZ0 - z0) / cellWidthZ0);}
      inline int digitizeCot(double cot) {return floor((cot - minCot) / cellWidthCot);}
      // get value at the center of parameter cell
      inline double getZ0CellValue(double iZ0) {return maxZ0 - (iZ0 + 0.5) * cellWidthZ0;}
      inline double getCotCellValue(double iCot) {return minCot + (iCot + 0.5) * cellWidthCot;}

    public:
      HoughVoter();
      void vote(const std::vector<std::vector<double>>& sCand, const std::vector<std::vector<double>>& zCand);
      std::tuple<double, double, int> findPeak();
    };

    /** Select the nearest hits from the voter result. */
    std::pair<std::vector<double>, std::vector<double>> selector(const std::vector<std::vector<double>>& sCand,
                                                     const std::vector<std::vector<double>>& zCand, double z0, double cot);

    /** Performance linear fitting with the selected s and z. */
    std::pair<double, double> fitter(const std::vector<double>& s, const std::vector<double>& z);


    // minor functions

    /** Convert continuous layer ID (0--55) -> stereo layer ID (0--19). */
    int getIStereoLayer(int iContinuousLayer);

    /** Convert the given angle(rad) to the range [-pi, pi]. */
    double normalizeAngle(double angle);

    /** Calculate phi coordinate of the crossing point of the given radius and the given 2D track. */
    double calPhiCross(double r, double phi0, double omega);

    /** Get the beginning wire ID of the preselection range(10 wires) for each layer. */
    std::vector<int> getIWireBegin(double phi0, double omega);

    /** 1 cell is selected in each layer to reduce LUT comsumption.
    This functionality is implemented in the HitmapMaker module in firmware */
    std::vector<int> select5Cells(const CDCTriggerSegmentHit* TS);

    /** Get drift length */
    double getDriftLength(const CDCHit hit);


    // 3D Fitter parameter arguments
    /** Name of the StoreArray containing the input CDChits. */
    std::string m_CDCHitCollectionName;
    /** Name of the StoreArray containing the input track segment hits. */
    std::string m_TSHitCollectionName;
    /** name of the event time StoreObjPtr */
    std::string m_EventTimeName;
    /** Name of the StoreArray containing the input tracks from the 2D fitter. */
    std::string m_inputCollectionName;
    /** Name of the StoreArray containing the resulting 3D tracks. */
    std::string m_outputCollectionName;
    /** Minimal number of votes in Hough voting. */
    int m_minVoteCount;


    // dataobjects
    /** list of track segment hits */
    StoreArray<CDCTriggerSegmentHit> m_TSHits;
    /** StoreObjPtr containing the event time */
    StoreObjPtr<BinnedEventT0> m_eventTime;
    /** list of 2D input tracks */
    StoreArray<CDCTriggerTrack> m_tracks2D;
    /** list of 3D output tracks*/
    StoreArray<CDCTriggerTrack> m_tracks3D;


    // 3D Fitter constants
    const int m_maxDriftTime = 512; // unit: TRGCLK ~ 2ns
    static const int m_nStereoLayer = 20;
    const int m_nCellInTS = 11;
    const int m_nLayerInSuperLayer = 5; // 5 layers out of 6 are used in trigger
    const double m_maxZSelection = 25; // cm

    /** CDC geometry constants **/
    std::vector<std::vector<double>> m_xtCurve;
    std::vector<int> m_nWire;
    std::vector<double> m_rWire;
    std::vector<std::vector<double>> m_phiBW;
    std::vector<double> m_zBW;
    std::vector<double> m_stereoAngle;
    std::map<std::pair<int, int>, int> m_cellIDInTS = {
      {std::make_pair(-2, -1), 0},
      {std::make_pair(-2, 0), 1},
      {std::make_pair(-2, 1), 2},
      {std::make_pair(-1, -1), 3},
      {std::make_pair(-1, 0), 4},
      {std::make_pair(0, 0), 5},
      {std::make_pair(1, -1), 6},
      {std::make_pair(1, 0), 7},
      {std::make_pair(2, -1), 8},
      {std::make_pair(2, 0), 9},
      {std::make_pair(2, 1), 10}
    };
  };
} // namespace Belle2

#endif // CDCTrigger3DFitterModule_H
