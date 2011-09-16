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
    EKLMHitBase() {};

    //! Constructor
    EKLMHitBase(char nEndcap, char nLayer, char nSector, char nPlane,
                char nStrip);

    //! Destructor
    virtual ~EKLMHitBase() {};

    //! Print hit information
    virtual void Print();

    /**
     * Get endcap number.
     */
    inline char get_nEndcap() {return m_nEndcap;}

    /**
     * Set endcap number.
     */
    inline void set_nEndcap(char nEndcap) {m_nEndcap = nEndcap;}

    /**
     * Get layer number.
     */
    inline char get_nLayer() {return m_nLayer;}

    /**
     * Set layer number.
     */
    inline void set_nLayer(char nLayer) {m_nLayer = nLayer;}

    /**
     * Get sector number.
     */
    inline char get_nSector() {return m_nSector;}

    /**
     * Set sector number.
     */
    inline void set_nSector(char nSector) {m_nSector = nSector;}

    /**
     * Get plane number.
     */
    inline char get_nPlane() {return m_nPlane;}

    /**
     * Set plane number.
     */
    inline void set_nPlane(char nPlane) {m_nPlane = nPlane;}

    /**
     * Get strip number.
     */
    inline char get_nStrip() {return m_nStrip;}

    /**
     * Set strip number.
     */
    inline void set_nStrip(char nStrip) {m_nStrip = nStrip;}

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
