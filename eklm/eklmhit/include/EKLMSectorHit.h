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
#include  <eklm/eklmhit/EKLMHit2d.h>
#include  "globals.hh"

#include <string>
namespace Belle2 {


  //! Class for handling hits in the whole sector. Needed to form 2d hits in a comfortable way
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

    //! Print information about the sector hit
    void Print();

    //! Adds Belle2::EKLMStripHit to the Belle2::EKLMSectorHit
    bool addStripHit(Belle2::EKLMStripHit *);

    //! returns vector of pointers to the Belle2::EKLMStripHits included to the Belle2::EKLMSectorHit
    inline std::vector <Belle2::EKLMStripHit*> getStripHitVector() const
    {return m_stripHitVector;}

    //! returns vector of pointers to 2d hits included to the Belle2::EKLMSectorHit
    inline std::vector <Belle2::EKLMHit2d*> get2dHitVector() const
    {return m_hit2dVector;}


    //! creates 2d hits from the SectorHits of the current sector
    void create2dHits();

    //! stores hits from m_hit2dVector to the datastore
    void store2dHits();

  private:

    //! vector of pointers to the Belle2::EKLMStripHits included to the Belle2::EKLMSectorHit
    std::vector<Belle2::EKLMStripHit*> m_stripHitVector;

    //!  vector of pointers to 2d hits included to the Belle2::EKLMSectorHit
    std::vector<Belle2::EKLMHit2d*> m_hit2dVector;

    //! Needed to make objects storable
    ClassDef(Belle2::EKLMSectorHit, 1);


  };


} // end of namespace Belle2

#endif //EKLMSECTORHIT_H
