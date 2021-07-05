/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : HoughFinder.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to find tracks usning Hough algorithm
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCHoughFinder_FLAG_
#define TRGCDCHoughFinder_FLAG_

#include <string>
#include "trg/cdc/HoughPlaneMulti2.h"
#include "trg/cdc/HoughTransformationCircle.h"
#include "trg/cdc/PeakFinder.h"
///
#include <map>
///
#ifdef TRGCDC_SHORT_NAMES
#define TCHFinder TRGCDCHoughFinder
#endif

namespace Belle2 {

  class TRGCDC;
  class TRGCDCPeakFinder;
  class TRGCDCTrack;
  class TRGCDCLink;
  class TRGCDCJSignal;
  class TRGCDCJLUT;
  class TRGCDCJSignalData;

/// A class to find tracks using Hough algorithm
  class TRGCDCHoughFinder {

  public:

    /// Contructor.
    TRGCDCHoughFinder(const std::string& name,
                      const TRGCDC&,
                      unsigned peakMin,
                      const std::string& mappingFilePlus,
                      const std::string& mappingFileMinus,
                      unsigned doit);

    /// Destructor
    virtual ~TRGCDCHoughFinder();

  public:

    /// returns name.
    std::string name(void) const;

    /// returns version.
    std::string version(void) const;

    /// termination.
    void terminate();

    /// do track finding and fitting (wrapper that can choose between different versions).
    int FindAndFit(std::vector<TRGCDCTrack*>& trackList2D,
                   std::vector<TRGCDCTrack*>& trackList2DFitted);

  public:

    /// sets and returns switch to do perfect finding.
    bool perfect(bool);

  private:

    /// do track finding and fitting (Trasan version).
    int doFindingAndFittingTrasan(std::vector<TRGCDCTrack*>& trackList2D,
                                  std::vector<TRGCDCTrack*>& trackList2DFitted);

    /// do track finding and fitting (Kaiyu version).
    int doFindingAndFitting(std::vector<TRGCDCTrack*>& trackList2D,
                            std::vector<TRGCDCTrack*>& trackList2DFitted);

    /// selects the best(fastest) hits in each super layer.
    std::vector<TRGCDCLink*> selectBestHits(
      const std::vector<TRGCDCLink*>& links) const;

    /// creates mappings by a file.
    void mappingByFile(const std::string& mappingFilePlus,
                       const std::string& mappingFileMinus);

    /// do track finding. (trasan version)
    int doFindingTrasan(std::vector<unsigned> peaks[],
                        std::vector<TRGCDCTrack*>& trackList2D) const;

    /// do track finding. (kaiyu version)
    int doFinding(std::vector<std::vector<unsigned>> peaks[],
                  std::vector<TRGCDCTrack*>& trackList2D);

    /// do track fitting. (old trasan version)
    int doFittingTrasan(std::vector<unsigned> peaks[],
                        std::vector<TRGCDCTrack*>& trackList2DFitted) const;

    /// do track fitting. (kaiyu original)
    int doFitting(std::vector<TRGCDCTrack*>& trackList2D,
                  std::vector<TRGCDCTrack*>& trackList2DFitted);

    /// Make a track from serial ID in Hough plane. (no fit, segment
    /// hit attached)
    TRGCDCTrack* makeTrack(const unsigned serialID, const unsigned pm) const;

    ///  Utility functions.
    static double calPhi(TRGCDCSegmentHit const* segmentHit,
                         double eventTime);

    /// Calculate Cos Sin ?
    static void calCosPhi(std::map<std::string, Belle2::TRGCDCJSignal>
                          & mSignalStorage,
                          std::map<std::string, Belle2::TRGCDCJLUT* >
                          & mLutStorage);

    /// Calculate Cos Sin ?
    static void calSinPhi(std::map<std::string, Belle2::TRGCDCJSignal>
                          & mSignalStorage,
                          std::map<std::string, Belle2::TRGCDCJLUT* >
                          & mLutStorage);

    /// Calculate r * phi ?
    static void rPhi(std::map<std::string, Belle2::TRGCDCJSignal>
                     & mSignalStorage,
                     std::map<std::string, Belle2::TRGCDCJLUT* >
                     & mLutStorage);
  private:

    /// Name.
    const std::string _name;

    /// CDCTRG.
    const TRGCDC& _cdc;

    /// Hough planes, for + and - charges.
    TRGCDCHoughPlaneMulti2* _plane[2];

    /// Circle Hough transformtion.
    TRGCDCHoughTransformationCircle _circleH;

    /// Doit version.
    const unsigned _doit;

    /// Peak finder.
    TRGCDCPeakFinder _peakFinder;

    /// Min. peak height for the peak finder.
    const unsigned _peakMin;

    /// Map to hold double values for Fitter2D.
    std::map<std::string, double> m_mDouble;

    /// Map to hold vector values for Fitter2D.
    std::map<std::string, std::vector<double> > m_mVector;

    /// Map to hold const values for Fitter2D.
    std::map<std::string, double> m_mConstants;

    /// Map to ?
    std::map<std::string, double> m_mConstD;

    /// Map to ?
    std::map<std::string, std::vector<double> > m_mConstV;

    /// Map to hold input options.
    std::map<std::string, bool> m_mBool;

    /// Map to hold JSignals.
    std::map<std::string, TRGCDCJSignal> m_mSignalStorage;

    /// Map to hold JLuts.
    std::map<std::string, TRGCDCJLUT*> m_mLutStorage;

    /// For VHDL code.
    TRGCDCJSignalData* _commonData;
  };

//-----------------------------------------------------------------------------

  inline
  std::string
  TRGCDCHoughFinder::name(void) const
  {
    return _name;
  }


} // namespace Belle2

#endif
