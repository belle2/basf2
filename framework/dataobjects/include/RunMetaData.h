/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef RUNMETADATA_H
#define RUNMETADATA_H

#include <TObject.h>
#include <framework/utilities/Boosts.h>
#include <framework/gearbox/Unit.h>


namespace Belle2 {

  /** Store information, that is constant during a run, and is needed during the event loop.
   *
   *  This object is supposed to be updated during the beginRun function.
   *  Currently stored information: <br>
   *  LER beam energy, HER beam energy, crossing angle of the beams, and angle between LER beam and solenoid axis. <br>
   *  The latter two are probably constants during the lifetime of the experiment, but maybe changed for design studies. <br>
   *  The Center of Mass energy and the \f$\gamma\f$ factor of the initial resonance in the lab frame can be calculated from the
   *  these variables.
   *
   *  @author <a href="mailto:martin.heck@kit.edu?subject=RunMetaData">Martin Heck</a>
   */
  class RunMetaData : public TObject {
  public:


    /** Constructor.
     *
     *  @param LER            Energy of the positron beam.
     *  @param HER            Energy of the electron beam.
     *  @param crossingAngle  Angle of crossing between the two beams
     *  @param angleLER       Angle between the LER beam and the solenoid axis, defining the axis of the Belle II detector.
     */
    RunMetaData(const float& energyLER = 4.0, const float& energyHER = 7.0,
                const float& crossAngle = 83 * Unit::mrad,
                const float& angleLER = 41.5 * Unit::mrad)
        : m_energyLER(energyLER), m_energyHER(energyHER), m_crossAngle(crossAngle), m_angleLER(angleLER) {}

    /** Destructor. */
    ~RunMetaData() {}

    /** Center of mass energy getter.
     *
     *  Will be implemented soon.
     *  @return Center of mass ernergy of the current run.
     */
    float getCmsEnergy() {
      return -999.;
    }

    /** \f$\gamma\f$ factor getter.
     *
     *  Will be implemented soon.
     *  @return \f$\gamma\f$ factor of the initial resonance in the lab frame.
     */
    float getLabGamma() {
      return -999;
    }

    /** LER energy getter. */
    float getEnergyLER() {
      return m_energyLER;
    }

    /** HER energy getter. */
    float getEnergyHER() {
      return m_energyHER;
    }

    /** Crossing angle getter. */
    float getCrossingAngle() {
      return m_crossAngle;
    }

    /** LER angle getter. */
    float getAngleLER() {
      return m_angleLER;
    }

    /** Comparison Operator.
     *
     *  @param eventMetaData The event meta data to compare with.
     *  @return True if event, run, and experiment numbers are the same
     */
    bool operator== (const RunMetaData& eventMetaData) const;

    /** Comparison Operator.
     *
     *  @param eventMetaData The event meta data to compare with.
     *  @return True if event, run, or experiment numbers are different
     */
    bool operator!= (const RunMetaData& eventMetaData) const;

  private:

    /** Center of Mass Energy.
     */
    double m_energyLER;

    /** \f$\gamma\f$ factor in the lab frame.
     */
    double m_energyHER;

    /** Angle in the lab system
     */
    double m_angleLER;

    /** Angle between beams. */
    double m_crossAngle;

    /** ROOT Macro to make RunMetaData a ROOT class.
     */
    ClassDef(RunMetaData, 1);

  }; //class
} // namespace Belle2
#endif // EVENTMETADATA
