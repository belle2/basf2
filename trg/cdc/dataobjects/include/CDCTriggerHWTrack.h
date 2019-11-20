#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <framework/dataobjects/Helix.h>
#include <framework/geometry/BFieldManager.h>
#include <framework/gearbox/Const.h>
#include <TVector3.h>
#include <cmath>

#pragma once

namespace Belle2 {

  /** Track created by the CDC trigger from CDC Unpacker. */
  class CDCTriggerHWTrack : public CDCTriggerTrack {
  public:
    CDCTriggerHWTrack(): CDCTriggerTrack()
    {
      m_foundoldtrack.assign(6, false);
      m_driftthreshold.assign(9, false);
      m_valstereobit = false;
      m_quadrant = -1;
      m_time = 0;
    }
    CDCTriggerHWTrack(double phi0, double omega, double chi2, std::vector<bool> foundoldtrack, std::vector<bool> driftthreshold,
                      bool valstereobit, short time = 0, short quadrant = -1):
      CDCTriggerTrack(phi0, omega, chi2, time, quadrant)
    {
      m_foundoldtrack = foundoldtrack;
      m_driftthreshold = driftthreshold;
      m_valstereobit = valstereobit;
      m_quadrant = quadrant;
      m_time = time;
    }
    CDCTriggerHWTrack(double phi0, double omega, double chi2, short time = 0, short quadrant = -1):
      CDCTriggerTrack(phi0, omega, chi2)
    {
      m_foundoldtrack.assign(6, false);
      m_driftthreshold.assign(9, false);
      m_valstereobit = false;
      m_quadrant = quadrant;
      m_time = time;
    }
    // accessors
    /** returns the quadrant the track was found in */
    short getQuadrant()
    {
      return m_quadrant;
    }
    /** get the track found time */
    float getTime() const { return m_time; }
    /** returns true, if old 2dtrack was found */
    std::vector<bool> getFoundOldTrack() const {return m_foundoldtrack;}

    /** returns true, if at least 3 stereo ts were found */
    bool getValidStereoBit() const {return m_valstereobit;}

    /** returns true, if the drift time was fitted into the time window */
    std::vector<bool> getDriftThreshold() const {return m_driftthreshold;}
  protected:
    /** value to store the quadrant the track was found */
    short m_quadrant;
    /** number of trigger clocks of (the track output - L1 trigger)*/
    short m_time;
    /** array to store wether an old 2dtrack was found */
    std::vector<bool> m_foundoldtrack;
    /** store if at least 3 valid stereo ts were found in the NNInput */
    bool m_valstereobit;
    /** store if drift time was within the timing window */
    std::vector<bool> m_driftthreshold;
    //! Needed to make the ROOT object storable
    ClassDef(CDCTriggerHWTrack, 1);
  };
}
