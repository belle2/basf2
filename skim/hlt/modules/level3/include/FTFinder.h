/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hidekazu Kakuno                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FTFinder_H
#define FTFinder_H

#include <TVector3.h>
#include "skim/hlt/modules/level3/FTList.h"
#include "skim/hlt/modules/level3/FTWire.h"
#include "skim/hlt/modules/level3/FTTrack.h"
#include "skim/hlt/modules/level3/FTLayer.h"
#include "skim/hlt/modules/level3/FTSuperLayer.h"

namespace Belle2 {

  class FTSegment;

  // The Level-3 Fast Track Finder
  class FTFinder {

  public:
    //! returns fzisan
    static FTFinder& instance();

  public:
    //! Constructors and destructor
    FTFinder();

    //! initializer(creates geometry)
    void init();

    //! terminator
    void term();

    //! begin run function(reads constants)
    void beginRun();

    //! track finder core
    void event(const int findEventVertex);

  public: // Selectors
    //! returns superlayer
    FTSuperLayer* superLayer(int id) const;

    //! returns track list
    FTList<FTTrack*>& getTracks(void) const;

    //! returns event primary vertex
    TVector3 getVertex(void) const;

    //! returns wire ID for given FTWire object
    int getWireId(FTWire*) const;

    //! convert t to x
    double t2x(const double t) const;

    //! convert x to t
    double x2t(const double x) const;

  private: // private member functions
    //! clear object
    void clear(void);

    //! clear geometry (delete wire/layer/superlayer objects)
    void clearGeometry(void);

    //! unpack RAWCDC and create wire-hit
    //int updateCdc(void);

    //! create wire hit from CDCHit
    void updateCdc3(void);

    //! create track list
    void mkTrackList(void);

    //! create 3D track list
    void mkTrack3D(void);

    //! finds event primary vertex
    int VertexFit(int z_flag);

    //! finds event primary vertex from 2D tracks
    int VertexFit2D();

    //! find vertex closest to IP
    //int findBestVertex(void);

    //! corrects event timing after 2nd r-phi fit and returns event timing
    int CorrectEvtTiming(void);

    //! link axial segments to make track
    FTTrack* linkAxialSegments(const FTSegment* initial);

    //! set badwires
    void setBadWires(const bool EvtByEvt, const bool inBeginRun);

  private: // private data members
    static FTFinder* s_tFinder; // pointer of the FTFinder
    double m_tOffSet; // offset of the T0
    double m_xtCoEff; // coefficient of the x-t relation
    double m_tWindow; // time window of a hit
    FTWire* m_wire; // pointer to the array of wires
    FTLayer* m_layer; // pointer to the array of layers
    FTSuperLayer* m_superLayer; // pointer to the array of super layers
    int m_Nwire; // number of wires
    int m_Nlayer; // number of layers
    int m_NsuperLayer; // number of super layers
    FTList<FTTrack*>& m_tracks; // list of tracks for the event
    FTList<FTSegment*>* m_linkedSegments; // list of all the track segments
    double m_tWindowLow; // lower bound of the time window
    double m_tWindowHigh; // upper bound of the time window
    double m_vx; // x of the reconstucted event vertex
    double m_vy; // y of the reconstucted event vertex
    double m_vz; // z of the reconstucted event vertex
    FTList<FTWire*>* m_EvtByEvtBadWires; // list of bad wires
  };

  //----------------------------------------------
#ifdef FTFinder_NO_INLINE
#define inline
#else
#undef inline
#define FTFinder_INLINE_DEFINE_HERE
#endif

#ifdef FTFinder_INLINE_DEFINE_HERE

  inline
  FTSuperLayer*
  FTFinder::superLayer(int id) const
  {
    return m_superLayer + id;
  }

  inline
  FTList<FTTrack*>&
  FTFinder::getTracks(void) const
  {
    return m_tracks;
  }

  inline
  int
  FTFinder::getWireId(FTWire* w) const
  {
    return (int)(w - m_wire);
  }

  inline
  double
  FTFinder::x2t(const double x) const
  {
    return x / m_xtCoEff;
  }

  inline
  double
  FTFinder::t2x(const double t) const
  {
    return t * m_xtCoEff;
  }

#endif

#undef inline

}

#endif /* FTFinder_FLAG_ */

