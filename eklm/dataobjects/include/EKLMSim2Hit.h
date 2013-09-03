/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMSIMHIT_H
#define EKLMSIMHIT_H

/* External headers. */
#include <TObject.h>

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMHitBase.h>
#include <eklm/dataobjects/EKLMHitCoord.h>
#include <eklm/dataobjects/EKLMHitMomentum.h>
#include <eklm/dataobjects/EKLMSimHit.h>

namespace Belle2 {

  /**
   * Class to handle simulation hits.
   */
  class EKLMSim2Hit : public EKLMHitBase, public EKLMHitCoord,
    public EKLMHitMomentum {

  public:

    /**
     * Default constructor needed to make the class storable.
     */
    EKLMSim2Hit();

    /**
     * Constructor with StepHit.
     * @param[in] stepHit EKLMSimHit.
     */
    EKLMSim2Hit(const EKLMSimHit* stepHit);

    /**
     * Destructor.
     */
    ~EKLMSim2Hit() {};

    /**
     * Get volume identifier.
     * @return Identifier.
     */
    int getVolumeID() const;

    /**
     * Set volume identifier.
     * @param[in] id Identifier.
     */
    void setVolumeID(int id);

    /**
     * Get plane number.
     */
    int getPlane() const;

    /**
     * Set plane number.
     * @param[in] Plane Plane number.
     */
    void setPlane(int Plane);

    /**
     * Get strip number.
     * @return Strip number.
     */
    int getStrip() const;

    /**
     * Set strip number.
     * @param[in] Strip Strip number.
     */
    void setStrip(int Strip);

  private:

    /** Volume identifier. */
    int m_volid;

    /** Number of plane. */
    int m_Plane;

    /** Number of strip. */
    int m_Strip;

    /** Needed to make root object storable. */
    ClassDef(Belle2::EKLMSim2Hit, 1);

  };

} // end of namespace Belle2

#endif //EKLMSIMHIT_H
