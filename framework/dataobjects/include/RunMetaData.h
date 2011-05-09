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
     *  @param energyLER            Energy of the positron beam.
     *  @param energyHER            Energy of the electron beam.
     *  @param crossingAngle  Angle of crossing between the two beams
     *  @param angleLER       Angle between the LER beam and the solenoid axis, defining the axis of the Belle II detector.
     */
    RunMetaData(const float& energyLER = 4.0, const float& energyHER = 7.0,
                const float& crossingAngle = 83   * Unit::mrad,
                const float& angleLER      = 41.5 * Unit::mrad)
        : m_energyLER(energyLER), m_energyHER(energyHER), m_crossingAngle(crossingAngle), m_angleLER(angleLER) {}

    /** Destructor. */
    ~RunMetaData() {}

    /** LER energy getter. */
    float getEnergyLER() const {
      return m_energyLER;
    }

    /** HER energy getter. */
    float getEnergyHER() const {
      return m_energyHER;
    }

    /** Crossing angle getter. */
    float getCrossingAngle() const {
      return m_crossingAngle;
    }

    /** LER angle getter. */
    float getAngleLER() const {
      return m_angleLER;
    }

    /** Comparison Operator.
     *
     *  @param  runMetaData Object to compare this one with.
     *  @return Is true, if all DataMembers are the same.
     */
    bool operator== (const RunMetaData& runMetaData) const;


  private:

    /** Center of Mass Energy.
     */
    float m_energyLER;

    /** \f$\gamma\f$ factor in the lab frame.
     */
    float m_energyHER;

    /** Angle between beams. */
    float m_crossingAngle;

    /** Angle in the lab system
     */
    float m_angleLER;

    /** ROOT Macro to make RunMetaData a ROOT class.
     */
    ClassDef(RunMetaData, 1);

  }; //class
} // namespace Belle2
#endif // EVENTMETADATA
