/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMSECTORHIT_H
#define EKLMSECTORHIT_H

#include <framework/datastore/DataStore.h>
#include <TObject.h>

#include  <eklm/eklmhit/EKLMStripHit.h>
#include  "globals.hh"

#include <string>
namespace Belle2 {

  class EKLMSectorHit : public EKLMHitBase  {

  public:

    //! Constructor
    EKLMSectorHit() {};

    //! Constructor with name
    EKLMSectorHit(const char *);

    //! Constructor with name
    EKLMSectorHit(std::string &);


    //! Destructor
    ~EKLMSectorHit() {};

    void Print();

    //    void clear();

    bool addStripHit(EKLMStripHit *);

    inline std::vector <EKLMStripHit*> getStripHitVector() const
    {return m_stripHitVector;}



  private:


    std::vector<EKLMStripHit*> m_stripHitVector;
    //    std::vector<EKLM2dHit*> m_hit2dVector;

    ClassDef(EKLMSectorHit, 1);


  };



} // end of namespace Belle2

#endif //EKLMSECTORHIT_H
