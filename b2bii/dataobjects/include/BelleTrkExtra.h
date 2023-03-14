/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
#include <framework/datastore/RelationsObject.h>

namespace Belle2 {
  /** Class stores mdst_trk_fit information for each track. */

  class BelleTrkExtra : public RelationsObject {

  public:
    /** Constructor initializing everything to zero */
    BelleTrkExtra() : m_firstx(0.0), m_firsty(0.0), m_firstz(0.0), m_lastx(0.0), m_lasty(0.0), m_lastz(0.0), m_tof(0.0),
      m_path_length(0.0), m_tof_quality(0), m_tof_sigma(0.0), m_acc_ph(0), m_acc_quality(0), m_dedx(0.0), m_dedx_quality(0) {};

    /** Constructor initializing variables */
    BelleTrkExtra(double first_x, double first_y, double first_z,
                  double last_x, double last_y, double last_z,
                  double tof, double path_length, short tof_quality, double tof_sigma,
                  int acc_ph,  short acc_quality,
                  double dedx, short dedx_quality) : m_firstx(first_x), m_firsty(first_y), m_firstz(first_z),
      m_lastx(last_x), m_lasty(last_y), m_lastz(last_z),
      m_tof(tof), m_path_length(path_length), m_tof_quality(tof_quality), m_tof_sigma(tof_sigma),
      m_acc_ph(acc_ph),  m_acc_quality(acc_quality),
      m_dedx(dedx), m_dedx_quality(dedx_quality) {};

    /** Destructor */
    ~BelleTrkExtra() {};


    /** Set the tof-related information
     *
     *  @param tof measured tof of the track [ns]
     *  @param path_length length of the track from the doca to the beam axis to the tof [cm]
     *  @param tof_sigma expected time resolution [ns]
     *  @param tof_quality matching quality: 0 if ok, 1 if track is not matched
     */
    void setTofInfo(double tof, double path_length, double tof_sigma, short tof_quality);


    /** Set the ACC-related information
     *
     *  @param acc_ph number of ACC photo-electrons associated to the track
     *  @param acc_quality matching quality: 0 if ok, 1 if track is not matched
     */
    void setACCInfo(int acc_ph,  short acc_quality);


    /** Set the dEdx-related information
     *
     *  @param dedx specific ionization of the track (keV/cm)
     *  @param dedx_quality quality of the measurement. Sadly undocumented in the panther tables...
     */
    void setdEdxInfo(double dedx,  short dedx_quality);



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


    /** Get tof
     *
     *  @return measure time of flight [ns]
     */
    double getTof(void) const
    {
      return m_tof;
    }

    /** Get path length
     *
     *  @return length of the track from the doca to the beam axis to the tof [cm]
     */
    double getPathLength(void) const
    {
      return m_path_length;
    }

    /** Get tof sigma
     *
     *  @return expected tof resolution [ns]
     */
    double getTofSigma(void) const
    {
      return m_tof_sigma;
    }

    /** Get tof quality flag
     *
     *  @return  matching quality: 0 if ok, 1 if track is not matched
     */
    short getTofQuality(void) const
    {
      return m_tof_quality;
    }

    /** Get number of ACC photoelectrons
     *
     *  @return number of ACC photo-electrons associated to the track
     */
    int getACCPe(void) const
    {
      return m_acc_ph;
    }


    /** Get ACC quality flag
     *
     *  @return  matching quality: 0 if ok, 1 if track is not matched
     */
    short getACCQuality(void) const
    {
      return m_acc_quality;
    }

    /** Get specific ionization of the track
     *
     *  @return specific ionization [keV/cm+
     */
    double getdEdx(void) const
    {
      return m_dedx;
    }

    /** Get de/dx quality flag
     *
     *  @return quality of the measurement. Sadly undocumented in the panther tables...
     */
    short getdEdxQuality(void) const
    {
      return m_dedx_quality;
    }

  private:
    // Persistent data members


    double m_firstx; /**< Start point of the track near the 1st CDC hit point. x component. */
    double m_firsty; /**< Start point of the track near the 1st CDC hit point. y component. */
    double m_firstz; /**< Start point of the track near the 1st CDC hit point. z component. */
    double m_lastx; /**< End point of the track near the last CDC hit point. x component. */
    double m_lasty; /**< End point of the track near the last CDC hit point. y component. */
    double m_lastz; /**< End point of the track near the last CDC hit point. z component. */
    double m_tof; /**<  measured tof of the track [ns] */
    double m_path_length; /**<  length of the track from the doca to the beam axis to the tof [cm] */
    short m_tof_quality; /**<  matching quality: 0 if ok, 1 if track is not matched */
    double m_tof_sigma; /**<  expected time resolution [ns] */
    int m_acc_ph; /**<  number of ACC photo-electrons associated to the track */
    short m_acc_quality; /**<  matching quality: 0 if ok, 1 if track is not matched */
    double m_dedx; /**<  specific ionization of the track (keV/cm) */
    short  m_dedx_quality; /**<  quality of the measurement. Sadly undocumented in the panther tables... */

    ClassDef(BelleTrkExtra, 2) /**< class definition */

  };

} // end namespace Belle2

