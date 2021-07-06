/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//-----------------------------------------------------------------------------
// Description : A class to find 3D tracks using Hough algorithm
//-----------------------------------------------------------------------------

#ifndef TRGCDCHough3DFinder_FLAG_
#define TRGCDCHough3DFinder_FLAG_

#include <string>
#include "trg/cdc/Hough3DUtility.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCH3DFinder TRGCDCHough3DFinder
#endif

namespace Belle2 {

  class TRGCDC;
  class TRGCDCTrack;
  class TRGCDCSegmentHit;
  class TRGCDCLink;

  /// class of TRGCDCHough3DFinder
  class TRGCDCHough3DFinder {
  public:
    /// Contructor.
    TRGCDCHough3DFinder(const TRGCDC&, bool makeRootFile, int finderMode);
    /// Destructor
    ~TRGCDCHough3DFinder();
    /// Copy constructor, deleted
    TRGCDCHough3DFinder(TRGCDCHough3DFinder&) = delete;
    /// Assignment operator, deleted
    TRGCDCHough3DFinder& operator=(TRGCDCHough3DFinder&) = delete;
    /// Member functions.
    /// Finds tracks using tracklist2D and outputs to trackList3D. Has parameter to choose between perfect and original finder.
    void doit(std::vector<TRGCDCTrack*> const& trackList2D, std::vector<TRGCDCTrack*>& trackList3D);
    /// Finds tracks using tracklist. Has parameter to choose between perfect and original finder.
    void doit(std::vector<TRGCDCTrack*>& trackList);
    /// Perfect 3D finder for a track.
    void perfectFinder(std::vector<TRGCDCTrack*>& trackList, unsigned j, std::vector<const TRGCDCSegmentHit*>& mcTSList);
    /// Perfect 3D finder for a tracklist.
    void doitPerfectly(std::vector<TRGCDCTrack*>& trackList);
    /// Finds tracks using tracklist.
    void doitFind(std::vector<TRGCDCTrack*>& trackList);
    /// Finds tracks using tracklist.
    void doitNewFind(std::vector<TRGCDCTrack*>& trackList);
    /// Finds number of hit superlayers for each mc particle.
    void findNumberOfHitSuperlayersForMcParticles(std::vector<std::vector<const TRGCDCSegmentHit*> >& p_stTSs,
                                                  std::map<unsigned, unsigned>& numberTSsForParticle);
    /// Termination method.
    void terminate(void);

    /// Members.
    /// Holds the trgcdc singleton.
    const TRGCDC& _cdc;
    /// Choose whether to save root file.
    bool m_makeRootFile;
    /// 0: perfect finder, 1: Hough3DFinder, 2: GeoFinder, 3: VHDL GeoFinder
    /// Choose what finder to use.
    int m_finderMode;
    /// Hough Variables.
    /// Pointer to hough 3D finder.
    Hough3DFinder* m_Hough3DFinder;
    /// For root file variables.
    std::string m_rootHough3DFilename;
    /// Tfile for Hough3D root file.
    TFile* m_fileFinder3D;
    /// TTree for tracks of Hough3D.
    TTree* m_treeTrackFinder3D;
    /// TTree for constants of Hough3D.
    TTree* m_treeConstantsFinder3D;
    /// Map to hold track double values for Fitter3D.
    std::map<std::string, double> m_mDouble;
    /// Map to hold track vector values for Fitter3D.
    std::map<std::string, std::vector<double> > m_mVector;
    /// Map to hold run values for Fitter3D.
    std::map<std::string, double> m_mConstD;
    /// Map to hold run vectcors for Fitter3D.
    std::map<std::string, std::vector<double> > m_mConstV;
    /// Map to hold event values for Fitter3D.
    std::map<std::string, double> m_mEventD;
    /// Map to hold event vectcors for Fitter3D.
    std::map<std::string, std::vector<double> > m_mEventV;
    /// Map to hold input options.
    std::map<std::string, bool> m_mBool;
    /// TVectorD map for saving run values to root file.
    std::map<std::string, TVectorD*> m_mRunTVectorD;
    /// TVectorD map for saving event values to root file.
    std::map<std::string, TVectorD*> m_mEventTVectorD;
    /// TClonesArray map for saving track values to root file.
    std::map<std::string, TClonesArray*> m_mTClonesArray;

  };

} // namespace Belle2

#endif
