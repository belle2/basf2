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
#include  <string>

namespace Belle2 {

  /**
   * Base Hit reconstruction class. inherit HitBase class from TObject
   *  to make all hits storable
   */
  class EKLMHitBase: public TObject  {

  public:

    /**
     *  Constructor
     */
    EKLMHitBase();

    /**
     *  Constructor
     */

    EKLMHitBase(int Endcap, int Layer, int Sector, int Plane,
                int Strip);

    /**
     * Destructor
     */
    ~EKLMHitBase() {};

    /**
     * Get endcap number.
     */
    int getEndcap() const;

    /**
     * Set endcap number.
     */
    void setEndcap(int Endcap);

    /**
     * Get layer number.
     */
    int getLayer() const;

    /**
     * Set layer number.
     */
    void setLayer(int nLayer);

    /**
     * Get sector number.
     */
    int getSector() const;

    /**
     * Set sector number.
     */
    void setSector(int nSector);

    /**
     * Get plane number.
     */
    int getPlane() const;

    /**
     * Set plane number.
     */
    void setPlane(int Plane);

    /**
     * Get strip number.
     */
    int getStrip() const;

    /**
     * Set strip number.
     */
    void setStrip(int Strip);

  private:

    /**
     * Number of endcap.
     */
    int m_Endcap;

    /**
     * Number of layer.
     */
    int m_Layer;

    /**
     * Number of sector.
     */
    int m_Sector;

    /**
     * Number of plane.
     */
    int m_Plane;

    /**
     * Number of strip.
     */
    int m_Strip;

    /**
     * Needed to make objects storable.
     */
    ClassDef(Belle2::EKLMHitBase, 1);

  };




} // end of namespace Belle2

#endif //EKLMHITBASE_H
