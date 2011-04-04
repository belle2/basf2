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

namespace Belle2 {

  /** Store information, that is constant during a run, and is needed during the event loop.
   *
   *  This object is supposed to be updated during the beginRun function.
   *  Currently stored information: <br>
   *  Center of mass energy <br>
   *  \f$\gamma\f$ factor of the initial resonance in the lab frame. <br>
   *
   *  @author <a href="mailto:martin.heck@kit.edu?subject=RunMetaData">Martin Heck</a>
   */
  class RunMetaData : public TObject {
  public:

    /** Constructor.
     *
     *  @param cmsEnergy Energy in the center of mass system, by default set to the Y(4S) mass.
     *  @param labGamma  \f$\gamma\f$ factor in the lab frame, by default set to the value for the Y(4S) at beam energies of 4 vs. 7 GeV.
     */
    RunMetaData(const double& cmsEnergy = 10.5794 , const double& labGamma = 1.0398)
        : m_cmsEnergy(cmsEnergy), m_labGamma(labGamma) {}

    /** Destructor. */
    ~RunMetaData() {}

    /** Center of mass energy setter.
     *
     *  @param cmsEnergy Center of mass energy.
     */
    void setCmsEnergy(const double& cmsEnergy) {
      m_cmsEnergy = cmsEnergy;
    }

    /** \f$\gamma\f$ factor setter.
     *
     *  @param gammaLab \f$\gamma\f$ factor in the lab frame.
     */
    void setLabGamma(const double& labGamma) {
      m_labGamma = labGamma;
    }

    /** Center of mass energy getter.
     *
     *  @return Center of mass ernergy of the current run.
     */
    unsigned long getCmsEnergy() const {
      return m_cmsEnergy;
    }

    /** \f$\gamma\f$ factor getter.
     *
     *  @return \f$\gamma\f$ factor of the initial resonance in the lab frame.
     */
    unsigned long getLabGamma() const {
      return m_labGamma;
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
    double m_cmsEnergy;

    /** \f$\gamma\f$ factor in the lab frame.
     */
    double m_labGamma;

    /** ROOT Macro to make RunMetaData a ROOT class.
     */
    ClassDef(RunMetaData, 1);

  }; //class
} // namespace Belle2
#endif // EVENTMETADATA
