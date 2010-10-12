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

namespace Belle2 {

  // inherit HitBase class from TObject to make all hits storable
  class EKLMHitBase: public TObject  {

  public:

    //! Constructor with initial values
    EKLMHitBase() {};

    //! Destructor
    virtual ~EKLMHitBase() {};

    virtual void Print();

  private:
    ClassDef(EKLMHitBase, 1);   // needed to be storable
  };



} // end of namespace Belle2

#endif //EKLMHITBASE_H
