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
    int findBestVertex(void);

    //! corrects event timing after 2nd r-phi fit and returns event timing
    int CorrectEvtTiming(void);

    //! link axial segments to make track
    FTTrack* linkAxialSegments(const FTSegment* initial);

    //! set badwires
    void setBadWires(const bool EvtByEvt, const bool inBeginRun);

  private: // private data members
    static FTFinder* s_tFinder;
    double m_tOffSet;
    double m_xtCoEff;
    double m_tWindow;
    FTWire* m_wire;
    FTLayer* m_layer;
    FTSuperLayer* m_superLayer;
    int m_Nwire;
    int m_Nlayer;
    int m_NsuperLayer;
    FTList<FTTrack*>& m_tracks;
    FTList<FTSegment*>* m_linkedSegments;
    double m_tWindowLow;
    double m_tWindowHigh;
    double m_vx;
    double m_vy;
    double m_vz;
    FTList<FTWire*>* m_EvtByEvtBadWires;
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

