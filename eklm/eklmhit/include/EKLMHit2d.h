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

  class EKLMHit2d : public EKLMHitBase  {

  public:

    //! Constructor
    EKLMHit2d();

    //! Constructor with name
    EKLMHit2d(const char *);

    //! Constructor with name
    EKLMHit2d(const std::string &);

    //! Constructor with two strips
    EKLMHit2d(EKLMStripHit*, EKLMStripHit*);

    //! Destructor
    ~EKLMHit2d() {};

    void Print();

    bool addStripHit(EKLMStripHit *);

    inline EKLMStripHit* getXStripHit() const
    {return m_XStrip;}

    inline EKLMStripHit* getYStripHit() const
    {return m_YStrip;}

    inline void  setCrossPoint(CLHEP::Hep3Vector & point)
    {m_crossPoint = point;}

    void setChiSq();


    inline double getChiSq() const
    {return m_ChiSq;}


  private:

    EKLMStripHit * m_XStrip;
    EKLMStripHit * m_YStrip;

    CLHEP::Hep3Vector m_crossPoint;

    ClassDef(EKLMHit2d, 1);

    double m_ChiSq;

  };



} // end of namespace Belle2

#endif //EKLMHIT2D_H
