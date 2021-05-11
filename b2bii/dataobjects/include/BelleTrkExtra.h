/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Chia-Ling Hsu                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <framework/datastore/RelationsObject.h>

namespace Belle2 {
  /** Class stores mdst_trk_fit information for each track. */

  class BelleTrkExtra : public RelationsObject {

  public:
    /** Constructor initializing everything to zero */
    BelleTrkExtra() : m_firstx(0.0), m_firsty(0.0), m_firstz(0.0), m_lastx(0.0), m_lasty(0.0), m_lastz(0.0) {};

    /** Constructor initializing variables */
    BelleTrkExtra(double first_x, double first_y, double first_z,
                  double last_x, double last_y, double last_z) :
      m_firstx(first_x), m_firsty(first_y), m_firstz(first_z),
      m_lastx(last_x), m_lasty(last_y), m_lastz(last_z) {};

    /** Destructor */
    ~BelleTrkExtra() {};

    /** Set first_x
     *  Start point of the track near the 1st CDC hit
     *  point. x-comp. (cm).
     *
     *  @param first_x for a track
     */
    void setTrackFirstX(double first_x);

    /** Set first_y
     *  Start point of the track near the 1st CDC hit
     *  point. y-comp. (cm).
     *
     *  @param first_y for a track
     */
    void setTrackFirstY(double first_y);

    /** Set first_z
     *  Start point of the track near the 1st CDC hit
     *  point. z-comp. (cm).
     *
     *  @param first_z for a track
     */
    void setTrackFirstZ(double first_z);

    /** Set last_x
     *  End point of the track near the last CDC hit
     *  point. x-comp. (cm).
     *
     *  @param last_x for a track
     */
    void setTrackLastX(double last_x);

    /** Set last_y
     *  End point of the track near the last CDC hit
     *  point. y-comp. (cm).
     *
     *  @param last_y for a track
     */
    void setTrackLastY(double last_y);

    /** Set last_z
     *  End point of the track near the last CDC hit
     *  point. z-comp. (cm).
     *
     *  @param last_z for a track
     */
    void setTrackLastZ(double last_z);

    /** Get first_x
     *
     *  @return first_x for a track
     */
    double getTrackFirstX(void) const
    {
      return m_firstx;
    }

    /** Get first_y
     *
     *  @return first_y for a track
     */
    double getTrackFirstY(void) const
    {
      return m_firsty;
    }

    /** Get first_z
     *
     *  @return first_z for a track
     */
    double getTrackFirstZ(void) const
    {
      return m_firstz;
    }

    /** Get last_x
     *
     *  @return last_x for a track
     */
    double getTrackLastX(void) const
    {
      return m_lastx;
    }

    /** Get last_y
     *
     *  @return last_y for a track
     */
    double getTrackLastY(void) const
    {
      return m_lasty;
    }

    /** Get last_z
     *
     *  @return last_z for a track
     */
    double getTrackLastZ(void) const
    {
      return m_lastz;
    }

  private:
    // Persistent data members
    double m_firstx; /**< Start point of the track near the 1st CDC hit point. x component. */
    double m_firsty; /**< Start point of the track near the 1st CDC hit point. y component. */
    double m_firstz; /**< Start point of the track near the 1st CDC hit point. z component. */
    double m_lastx; /**< End point of the track near the last CDC hit point. x component. */
    double m_lasty; /**< End point of the track near the last CDC hit point. y component. */
    double m_lastz; /**< End point of the track near the last CDC hit point. z component. */

    ClassDef(BelleTrkExtra, 1) /**< class definition */

  };

} // end namespace Belle2

