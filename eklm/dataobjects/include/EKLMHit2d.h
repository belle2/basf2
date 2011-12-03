/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMHIT2D_H
#define EKLMHIT2D_H

#include <framework/datastore/DataStore.h>
#include <TObject.h>

#include  <eklm/dataobjects/EKLMStripHit.h>
#include  "globals.hh"
#include  "TVector3.h"

#include <string>
namespace Belle2 {

  //! Class for 2d hits handling
  class EKLMHit2d : public EKLMHitBase  {

  public:

    //! Constructor
    EKLMHit2d();

    //! Constructor with two strips
    EKLMHit2d(EKLMStripHit*, EKLMStripHit*);

    //! Destructor
    ~EKLMHit2d() {};

    //! Print 2d hit information
    void Print();

    //! add StripHit to 2dhit. returns false if impossible
    bool addStripHit(const EKLMStripHit *);

    //! returns pointer to the strip hit in X direction
    const EKLMStripHit* getXStripHit() const;

    //! returns pointer to the strip hit in Y direction
    const EKLMStripHit* getYStripHit() const;

    //! set coordinates of the crossing point
    void setCrossPoint(TVector3 & point);

    //! returns coordinates of the crossing point
    TVector3 getCrossPoint()  const;

    //! calculates ChiSquare of the crossing point
    void setChiSq();

    //! returns  ChiSquare of the crossing point
    double getChiSq() const;


  private:

    //! reference to the X Strip hit
    EKLMStripHit const * m_XStrip; //-> {ROOT streamer directive}

    //! reference to the Y Strip hit
    EKLMStripHit const * m_YStrip; //-> {ROOT streamer directive}

    //! crossing point global coordinates
    TVector3  m_crossPoint;

    //! ChiSq of the hit
    double m_ChiSq;

    //! Needed to make Belle2::EKLMHit2d storable
    ClassDef(Belle2::EKLMHit2d, 1);
    /**
     * Number of plane.
     */
    int m_Plane;


  };

} // end of namespace Belle2

#endif //EKLMHIT2D_H
