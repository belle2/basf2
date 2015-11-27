/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {


  /**
   * Class to store T0 information
   */
  class TOPTimeZero : public RelationsObject {
  public:

    /**
     * Default constructor
     */
    TOPTimeZero()
    {}

    /**
     * Set FTSW
     * @param ftsw FTSW count
     */
    void setFTSW(unsigned ftsw) { m_ftsw = ftsw;}

    /**
     * Set time zero
     * @param t0 time zero in [ns]
     */
    void setTime(double t0) { m_t0 = t0;}

    /**
     * Set time zero uncertainty
     * @param err uncertainty in [ns]
     */
    void setError(double err) { m_err = err;}

    /**
     * Returns FTSW count
     * @return FTSW
     */
    unsigned getFTSW() const {return m_ftsw;}

    /**
     * Returns time zero
     * @return time zero [ns]
     */
    double getTime() const {return m_t0;}

    /**
     * Returns time zero uncertainty
     * @return uncertainty [ns]
     */
    double getError() const {return m_err;}

  private:

    unsigned m_ftsw = 0; /**< FTSW */
    float m_t0 = 0;    /**< time zero in [ns] */
    float m_err = 0;   /**< error on time zero [ns] */

    ClassDef(TOPTimeZero, 1); /**< ClassDef */

  };

}
