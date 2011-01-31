/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMHITBASE_H
#define EKLMHITBASE_H

#include <TObject.h>
//#include <simulation/simkernel/B4VHit.h>

//#include <framework/datastore/StoreDefs.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include  <string>


namespace Belle2 {

  //! Base Hit reconstruction class. inherit HitBase class from TObject to make all hits storable
  class EKLMHitBase: public TObject  {

  public:

    //! Constructor
    EKLMHitBase() {};

    //! Constructor with name
    EKLMHitBase(const char *);

    //! Constructor with name
    EKLMHitBase(std::string &);

    //! Destructor
    virtual ~EKLMHitBase() {};

    //! Print hit information
    virtual void Print();

    //! sets name for the hit. Usually constructed from the strip name
    inline void setName(std::string name)
    {m_Name = name;}

    //! returns hit name
    inline std::string getName() const
    {return m_Name;}


  private:
    //! needed to make objects storable
    ClassDef(Belle2::EKLMHitBase, 1);

    //! name of the hit
    std::string m_Name;
  };




} // end of namespace Belle2

#endif //EKLMHITBASE_H
