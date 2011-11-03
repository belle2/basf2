/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMHITBASE_H
#define EKLMHITBASE_H

#include <sys/types.h>

#include <TObject.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include  <string>


namespace Belle2 {

  //! Base Hit reconstruction class. inherit HitBase class from TObject
  //! to make all hits storable
  class EKLMHitBase: public TObject  {

  public:

    //! Constructor
    EKLMHitBase();

    //! Constructor
    EKLMHitBase(char nEndcap, char nLayer, char nSector, char nPlane,
                char nStrip);

    //! Destructor
    ~EKLMHitBase() {};

    //! Print hit information
    void Print();

    /**
     * Get endcap number.
     */
    int get_nEndcap();

    /**
     * Set endcap number.
     */
    void set_nEndcap(char nEndcap);

    /**
     * Get layer number.
     */
    int get_nLayer();

    /**
     * Set layer number.
     */
    void set_nLayer(char nLayer);

    /**
     * Get sector number.
     */
    int get_nSector();

    /**
     * Set sector number.
     */
    void set_nSector(char nSector);

    /**
     * Get plane number.
     */
    int get_nPlane();

    /**
     * Set plane number.
     */
    void set_nPlane(char nPlane);

    /**
     * Get strip number.
     */
    int get_nStrip();

    /**
     * Set strip number.
     */
    void set_nStrip(char nStrip);

  private:
    /**
     * Number of endcap.
     */
    char m_nEndcap;

    /**
     * Number of layer.
     */
    char m_nLayer;

    /**
     * Number of sector.
     */
    char m_nSector;

    /**
     * Number of plane.
     */
    char m_nPlane;

    /**
     * Number of strip.
     */
    char m_nStrip;

    /**
     * Needed to make objects storable.
     */
    ClassDef(Belle2::EKLMHitBase, 1);

  };




} // end of namespace Belle2

#endif //EKLMHITBASE_H
