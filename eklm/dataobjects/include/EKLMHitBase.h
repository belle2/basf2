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
    const int getEndcap();

    /**
     * Set endcap number.
     */
    void setEndcap(const int Endcap);

    /**
     * Get layer number.
     */
    const int getLayer();

    /**
     * Set layer number.
     */
    void setLayer(const int nLayer);

    /**
     * Get sector number.
     */
    const int getSector();

    /**
     * Set sector number.
     */
    void setSector(const int nSector);

    /**
     * Get plane number.
     */
    const int getPlane();

    /**
     * Set plane number.
     */
    void setPlane(const int Plane);

    /**
     * Get strip number.
     */
    const int getStrip();

    /**
     * Set strip number.
     */
    void setStrip(const int Strip);

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
