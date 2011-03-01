/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef VOLUMEUSERINFOBASE_H_
#define VOLUMEUSERINFOBASE_H_

// ROOT
#include <TObject.h>

namespace Belle2 {

  /**
    * VolumeUserInfo base class.
    *
    * The base class for all classes, which are attached to TGeoVolumes and carry user information.
    */
  class VolumeUserInfoBase : public TObject {

  public:

    /** Default constructor for ROOT. */
    VolumeUserInfoBase() {
      /*! Does nothing */
    }

    /** Full constructor.
     */
    //VolumeUserInfoBase()
    //{
    /* Does nothing. */
    //}

    /** Destructor */
    ~VolumeUserInfoBase() {
      /* Does nothing. */
    }

    /** Assignment operator.*/
    VolumeUserInfoBase& operator=(const VolumeUserInfoBase& other);

  private:

    ClassDef(VolumeUserInfoBase, 1)

  };

} // end namespace Belle2

#endif /* VOLUMEUSERINFOBASE_H_ */
