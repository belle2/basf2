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
                      unsigned nX,
                      unsigned nY,
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

    /// do track finding and fitting.
    int doit(std::vector<TRGCDCTrack*>& trackList2D,
             std::vector<TRGCDCTrack*>& trackList2DFitted);

  public:

    /// sets and returns switch to do perfect finding.
    bool perfect(bool);

  private:

    /// do track finding and fitting (Trasan version).
    int doit1(std::vector<TRGCDCTrack*>& trackList2D,
              std::vector<TRGCDCTrack*>& trackList2DFitted);

    /// do track finding and fitting (Kaiyu version).
    int doit2(std::vector<TRGCDCTrack*>& trackList2D,
              std::vector<TRGCDCTrack*>& trackList2DFitted);

    /// do track finding and fitting (development version)
    int doit3(std::vector<TRGCDCTrack*>& trackList2D,
              std::vector<TRGCDCTrack*>& trackList2DFitted);

    /// selects the best(fastest) hits in each super layer.
    std::vector<TRGCDCLink*> selectBestHits(
      const std::vector<TRGCDCLink*>& links) const;

    /// creates mappings by a file. (Kaiyu's original)
    void mappingByFile(const std::string& mappingFilePlus,
                       const std::string& mappingFileMinus);

    /// creates mappings by a file.
    void mappingByFile2(const std::string& mappingFilePlus,
                        const std::string& mappingFileMinus);

    /// do track finding. (trasan version)
    int doFinding(std::vector<unsigned> peaks[],
                  std::vector<TRGCDCTrack*>& trackList2D) const;

    /// do track finding. (kaiyu version)
    int doFinding2(std::vector<std::vector<unsigned>> peaks[],
                   std::vector<TRGCDCTrack*>& trackList2D);

    /// do track fitting. (old trasan version)
    int doFitting(std::vector<unsigned> peaks[],
                  std::vector<TRGCDCTrack*>& trackList2DFitted) const;

    /// do track fitting. (kaiyu original)
    int doFitting2(std::vector<TRGCDCTrack*>& trackList2D,
                   std::vector<TRGCDCTrack*>& trackList2DFitted);

    /// do track fitting.
    int doFitting3(std::vector<TRGCDCTrack*>& trackList2D,
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
