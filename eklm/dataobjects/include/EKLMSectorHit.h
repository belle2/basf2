/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMSECTORHIT_H
#define EKLMSECTORHIT_H

#include  <eklm/dataobjects/EKLMHitBase.h>
#include  <eklm/dataobjects/EKLMStripHit.h>
#include  <eklm/dataobjects/EKLMHit2d.h>
#include  "globals.hh"

#include <string>
#include <vector>



namespace Belle2 {

  //! Class for handling hits in the whole sector. Needed to form 2d hits
  //! in a comfortable way
  class EKLMSectorHit : public EKLMHitBase  {

  public:

    //! Constructor
    EKLMSectorHit() {};

    //! Constructor
    EKLMSectorHit(int nEndcap, int nLayer, int nSector);

    //! Destructor
    ~EKLMSectorHit() {};

    //! Print information about the sector hit
    void Print();


    //! returns pointer to the vector of pointers to the EKLMStripHits included to
    //! the EKLMSectorHit
    std::vector <EKLMStripHit*> * getStripHitVector() ;

    //! returns pointer to the vector of pointers to 2d hits included to the

    //! Belle2::EKLMSectorHit
    std::vector <EKLMHit2d*>* get2dHitVector() ;


  private:

    //! vector of pointers to the Belle2::EKLMStripHits included to the
    //! Belle2::EKLMSectorHit
    std::vector<EKLMStripHit*> m_stripHitVector;

    //! vector of pointers to 2d hits included to the Belle2::EKLMSectorHit
    std::vector<EKLMHit2d*> m_hit2dVector;

    //! Needed to make objects storable
    ClassDef(Belle2::EKLMSectorHit, 1);


  };


} // end of namespace Belle2

#endif //EKLMSECTORHIT_H
