/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hidekazu Kakuno                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FTSuperLayer_H
#define FTSuperLayer_H

#include "skim/hlt/modules/level3/FTList.h"

namespace Belle2 {

  class FTWire;
  class FTSegment;
  class FTFinder;
  class FTLayer;

  //! CDC superlayer class for the Level-3 Fast Track Finder
  class FTSuperLayer {
  public:
    //! constructor
    FTSuperLayer(const int NLayer, const int superLayerID, const int isAxial,
                 const bool haveSmallCell, FTLayer* first);

    //! destructor
    ~FTSuperLayer();

  public:
    //! clear object
    void clear(void);

    //! create segment lists
    void mkSegmentList(void);

    //! append wireHit to the list of hits
    void appendHit(FTWire*);

    //! append segments which are not used for tracks to the list for salvage
    void reAppendSalvage(void);

    //! returns if this superlayer have small cell layer
    bool haveSmallCell(void) const;

  public:
    //! returns wirehit list
    FTList<FTWire*>& wireHits(void) const;

    //! returns segement list
    FTList<FTSegment*>& segments(void) const;

    //! returns number of layers
    int nLayer(void) const;

    //! returns layer max ID
    int layerMaxId(void) const;

    //! returns super-layer ID
    int superLayerId(void) const;

    //! returns true/false for Axial/Stereo super-layer
    int IsAxial(void) const;

    //! returns complicated segments
    FTList<FTSegment*>& complicatedSegments(void);

    //! returns first layer in the super layer
    FTLayer* firstLayer() const;

  private:
    //! clustering of wire-hits
    void clustering(void);

    //! conect short clusters to make segments
    void connectShortSegments(FTList<FTSegment*>& innerShort,
                              FTList<FTSegment*>& outerShort,
                              bool complicatedSeg = false);

    //! connect single wireHit to make segments
    void connectSingleHit(FTList<FTSegment*>&, bool complicatedSeg = false);

    //! returns 2*localId diffrerence between two hits
    int i2phiDiff(const FTWire& h1, const FTWire& h2) const;

  private:
    //! state mask for six neighbors
    static const unsigned int m_neighborsMask[6];

  private:
    //! ID of the superlayer
    const int m_superLayerId;

    //! number of layers
    const int m_Nlayer;

    //! =0: stereo =1: axial
    const int m_isAxial;

    //! list of segments having complicated shape
    FTList<FTSegment*>* const m_complicatedSegments;

    //! list of wire hits
    FTList<FTWire*>& m_wireHits;

    //! list of single hits
    FTList<FTWire*>& m_singleHits;

    //! list of segments
    FTList<FTSegment*>& m_segments;

    //! true for small cell
    const bool m_haveSmallCell;

    //! pointer to the first layer
    FTLayer* const m_firstLayer;
  };

  inline
  FTSuperLayer::FTSuperLayer(const int NLayer, const int superLayerID,
                             const int isAxial, const bool haveSmallCell,
                             FTLayer* first)
    : m_superLayerId(superLayerID),
      m_Nlayer(NLayer),
      m_isAxial(isAxial),
      m_complicatedSegments((superLayerID) ? NULL : new FTList<FTSegment * >(20)),
      m_wireHits(*(new FTList<FTWire * >(500))),
      m_singleHits(*(new FTList<FTWire * >(100))),
      m_segments(*(new FTList<FTSegment * >(20))),
      m_haveSmallCell(haveSmallCell),
      m_firstLayer(first)
  {
  }

  inline
  FTSuperLayer::~FTSuperLayer()
  {
    clear();
    delete &m_wireHits;
    delete &m_singleHits;
    delete &m_segments;
    if (m_complicatedSegments) delete m_complicatedSegments;
  }

  inline
  FTList<FTWire*>&
  FTSuperLayer::wireHits(void) const
  {
    return m_wireHits;
  }

  inline
  int
  FTSuperLayer::nLayer(void) const
  {
    return m_Nlayer;
  }

  inline
  int
  FTSuperLayer::layerMaxId(void) const
  {
    return (m_Nlayer - 1);
  }

  inline
  FTList<FTSegment*>&
  FTSuperLayer::segments(void) const
  {
    return m_segments;
  }

  inline
  int
  FTSuperLayer::superLayerId(void) const
  {
    return m_superLayerId;
  }

  inline
  int
  FTSuperLayer::IsAxial(void) const
  {
    return m_isAxial;
  }

  inline
  FTList<FTSegment*>&
  FTSuperLayer::complicatedSegments(void)
  {
    return *m_complicatedSegments;
  }

  inline
  void
  FTSuperLayer::appendHit(FTWire* h)
  {
    m_wireHits.append(h);
  }

  inline
  bool
  FTSuperLayer::haveSmallCell(void) const
  {
    return m_haveSmallCell;
  }

  inline
  FTLayer*
  FTSuperLayer::firstLayer() const
  {
    return m_firstLayer;
  }

}

#endif /* FTSuperLayer_FLAG_ */

