/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMSECTORHIT_H
#define EKLMSECTORHIT_H

/* C++ headers. */
#include <vector>

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMHitBase.h>
#include <eklm/dataobjects/EKLMDigit.h>

namespace Belle2 {

  /**
   * Class for handling hits in the whole sector. Needed to form 2d hits
   * in a comfortable way.
   */
  class EKLMSectorHit : public EKLMHitBase  {

  public:

    /**
     * Constructor.
     */
    EKLMSectorHit() {};

    /**
     * Constructor.
     */
    EKLMSectorHit(int nEndcap, int nLayer, int nSector);

    /**
     * Destructor.
     */
    ~EKLMSectorHit() {};

    /**
     * Add hit.
     * @param[in] hit Hit.
     * @return 0    Successful.
     * @return != 0 Hit cannot be added. It is from another sector.
     */
    int addHit(EKLMDigit* hit);

    /**
     * Get number of hits.
     * @param[in] plane  Number of plane.
     * @return Number of hits in this plane.
     */
    int getHitNumber(int plane);

    /**
     * Get hit.
     * @param[in] plane  Number of plane.
     * @param[in] hit    Number of hit.
     * @return Hit.
     */
    EKLMDigit* getHit(int plane, int hit);

  private:

    /** Hits. Index is equal to (number of plane - 1). */
    std::vector<EKLMDigit*> m_hits[2];

    /** Needed to make objects storable. */
    ClassDef(Belle2::EKLMSectorHit, 1);

  };

}

#endif

