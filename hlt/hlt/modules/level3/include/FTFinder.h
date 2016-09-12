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

  //! The Level-3 Fast Track Finder
  class FTFinder {

  public:
    //! returns fzisan
    static FTFinder& instance();

  public:
    //! Constructors and destructor
    FTFinder();

    //! Constructors and destructor
    FTFinder(const FTFinder&);

    //! initializer(creates geometry)
    void init();

    //! terminator
    void term();

    //! begin run function(reads constants)
    void beginRun();

    //! track finder core
    void event(const int findEventVertex);

  public:
    //! returns superlayer
    FTSuperLayer* superLayer(int id) const;

    //! returns track list
    FTList<FTTrack*>& getTracks(void) const;

    //! returns event primary vertex
    TVector3 getVertex(void) const;

    //! returns wire ID for given FTWire object
    int getWireId(FTWire*) const;

    //! convert t to x
    double t2x(const FTLayer& l, const double t) const;

    //! convert x to t
    double x2t(const double x) const;

  private:
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

  private:
    //! pointer of the FTFinder
    static FTFinder* s_tFinder;

    //! offset of the T0
    double m_tOffSet;

    //! coefficient of the x-t relation
    double m_xtCoEff;

    //! additional coefficient of the x-t relation
    double m_xtCoEff2;

    //! time window of a hit
    double m_tWindow;

    //! nominal T0 value
    double m_tZero;

    //! pointer to the array of wires
    FTWire* m_wire;

    //! pointer to the array of layers
    FTLayer* m_layer;

    //! pointer to the array of super layers
    FTSuperLayer* m_superLayer;

    //! number of wires
    int m_Nwire;

    //! number of layers
    int m_Nlayer;

    //! number of super layers
    int m_NsuperLayer;

    //! list of tracks for the event
    FTList<FTTrack*>& m_tracks;

    //! list of all the track segments
    FTList<FTSegment*>* m_linkedSegments;

    //! lower bound of the time window
    double m_tWindowLow;

    //! upper bound of the time window
    double m_tWindowHigh;

    //! x of the reconstucted event vertex
    double m_vx;

    //! y of the reconstucted event vertex
    double m_vy;

    //! z of the reconstucted event vertex
    double m_vz;

    //! list of bad wires
    FTList<FTWire*>* m_EvtByEvtBadWires;
  };

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
    return pow(x / m_xtCoEff, 1. / m_xtCoEff2);
  }

  inline
  double
  FTFinder::t2x(const FTLayer& l, const double t) const
  {
    double x = t > 0. ? m_xtCoEff * pow(t, m_xtCoEff2) : 0.;
    return x > 0.5 * l.csize() ? 0.5 * l.csize() : x;
  }

}

#endif /* FTFinder_FLAG_ */

