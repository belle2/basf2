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
#include <eklm/dataobjects/EKLMStepHit.h>

namespace Belle2 {

  /**
   * Class to handle simulation hits.
   */
  class EKLMSimHit : public EKLMHitBase, public EKLMHitCoord,
    public EKLMHitMomentum {

  public:

    /**
     * Default constructor needed to make the class storable.
     */
    EKLMSimHit();

    /**
     * Constructor with StepHit.
     * @param[in] stepHit EKLMStepHit.
     */
    EKLMSimHit(const EKLMStepHit* stepHit);

    /**
     * Destructor.
     */
    ~EKLMSimHit() {};

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
     * Print hit.
     */
    void Print();

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
    ClassDef(Belle2::EKLMSimHit, 1);

  };

} // end of namespace Belle2

#endif //EKLMSIMHIT_H
