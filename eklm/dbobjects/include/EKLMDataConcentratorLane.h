/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMDATACONCENTRATORLANE_H
#define EKLMDATACONCENTRATORLANE_H

/* External headers. */
#include <TObject.h>

namespace Belle2 {

  /**
   * EKLM data concentrator lane identifier.
   */
  class EKLMDataConcentratorLane : public TObject {

  public:

    /**
     * Constructor.
     */
    EKLMDataConcentratorLane();

    /**
     * Destructor.
     */
    ~EKLMDataConcentratorLane();

    /**
     * Get copper number.
     */
    int getCopper() const;

    /**
     * Set copper number.
     * @param[in] copper Copper number.
     */
    void setCopper(int copper);

    /**
     * Get data concentrator number.
     */
    int getDataConcentrator() const;

    /**
     * Set data concentrator number.
     * @param[in] dataConcentrator Data concentrator number.
     */
    void setDataConcentrator(int dataConcentrator);

    /**
     * Get lane number.
     */
    int getLane() const;

    /**
     * Set lane number.
     * @param[in] lane Lane number.
     */
    void setLane(int lane);

    /**
     * Operator <.
     */
    bool operator<(const EKLMDataConcentratorLane& lane) const;

  private:

    /** Copper number - EKLM_ID (7000). */
    int m_Copper;

    /** Data concentrator number. */
    int m_DataConcentrator;

    /** Lane number. */
    int m_Lane;

    /** Needed to make objects storable, */
    ClassDef(EKLMDataConcentratorLane, 1);

  };

}

#endif

