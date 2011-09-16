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

#include  <eklm/eklmhit/EKLMStripHit.h>
#include  "globals.hh"
#include  "CLHEP/Vector/ThreeVector.h"

#include <string>
namespace Belle2 {

  //! Class for 2d hits handling
  class EKLMHit2d : public EKLMHitBase  {

  public:

    //! Constructor
    EKLMHit2d();

    //! Constructor with two strips
    EKLMHit2d(Belle2::EKLMStripHit*, Belle2::EKLMStripHit*);

    //! Destructor
    ~EKLMHit2d() {};

    //! Print 2d hit information
    void Print();

    //! add StripHit to 2dhit. returns false if impossible
    bool addStripHit(Belle2::EKLMStripHit *);

    //! returns pointer to the strip hit in X direction
    inline Belle2::EKLMStripHit* getXStripHit() const
    {return m_XStrip;}

    //! returns pointer to the strip hit in Y direction
    inline Belle2::EKLMStripHit* getYStripHit() const
    {return m_YStrip;}

    //! set coordinates of the crossing point
    inline void  setCrossPoint(CLHEP::Hep3Vector & point)
    {m_crossPoint = point;}

    //! returns coordinates of the crossing point
    inline CLHEP::Hep3Vector  getCrossPoint() const
    {return  m_crossPoint;}

    //! calculates ChiSquare of the crossing point
    void setChiSq();

    //! returns  ChiSquare of the crossing point
    inline double getChiSq() const
    {return m_ChiSq;}


  private:


    //! reference to the X Strip hit
    Belle2::EKLMStripHit * m_XStrip; //-> {ROOT streamer directive}

    //! reference to the Y Strip hit
    Belle2::EKLMStripHit * m_YStrip; //-> {ROOT streamer directive}

    //! crossing point global coordinates
    CLHEP::Hep3Vector m_crossPoint;

    //! ChiSq of the hit
    double m_ChiSq;

    //! Needed to make Belle2::EKLMHit2d storable
    ClassDef(Belle2::EKLMHit2d, 1);
  };



} // end of namespace Belle2

#endif //EKLMHIT2D_H
