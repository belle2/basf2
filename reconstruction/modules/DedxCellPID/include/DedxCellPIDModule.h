/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef DEDXCELLPIDMODULE_H
#define DEDXCELLPIDMODULE_H

#include <reconstruction/dataobjects/DedxConstants.h>

#include <framework/core/Module.h>
#include <framework/gearbox/Const.h>
#include <framework/datastore/StoreArray.h>

#include <string>
#include <vector>
#include <TVector3.h>

using namespace std;

class TH2F;

namespace Belle2 {

  class DedxCell;

  /** Performs a simple path length correction to the dE/dx measuremnt
   * based on individual hits in the CDC and determines the mean and
   * truncated mean dE/dx value for each track.
   *
   * Outputs a DataStore object (DedxCell), which contains pertinent
   * information for each CDC hit of an associated track.
   *
   * Adapted from DedxPIDModule
   */
  class DedxCellPIDModule : public Module {

  public:

    /** Default constructor */
    DedxCellPIDModule();

    /** Destructor */
    virtual ~DedxCellPIDModule();

    /** Initialize the module */
    virtual void initialize();

    /** This method is called for each event. All processing of the event
     * takes place in this method. */
    virtual void event();

    /** End of the event processing. */
    virtual void terminate();

  private:

    /** counter for events */
    int m_eventID;
    /** counter for tracks in this event */
    int m_trackID;

    /** Save arithmetic and truncated mean for the 'dedx' values.
     *
     * @param mean              calculated arithmetic mean
     * @param truncatedMean     calculated truncated mean
     * @param truncatedMeanErr  error for truncatedMean
     * @param dedx              input values
     */
    void calculateMeans(double* mean, double* truncatedMean, double* truncatedMeanErr, const std::vector<double>& dedx) const;

    /** for all particles, save log-likelihood values into 'logl'.
     *
     * @param logl  array of log-likelihood to be modified
     * @param p     track momentum
     * @param dedx  dE/dx value
     * @param pdf   pointer to array of 2d PDFs to use (not modified)
     * */
    void saveLogLikelihood(float(&logl)[Const::ChargedStable::c_SetSize], double p, double dedx, TH2F* const* pdf) const;

    /** dedx:momentum PDFs. */
    TH2F* m_pdfs[Dedx::c_num_detectors][Const::ChargedStable::c_SetSize]; //m_pdfs[detector_type][particle_type]

    // parameters: full likelihood vs. truncated mean
    bool m_useIndividualHits; /**< Include PDF value for each hit in likelihood. If false, the truncated mean of dedx values for the detectors will be used. */
    double m_removeLowest; /**< Portion of lowest dE/dx values that should be discarded for truncated mean */
    double m_removeHighest; /**< Portion of highest dE/dx values that should be discarded for truncated mean */

    bool m_enableDebugOutput; /**< Whether to save information on tracks and associated hits and dE/dx values in DedxTrack objects */
    string m_pdfFile; /**< file containing the PDFs required for constructing a likelihood. */
    bool m_ignoreMissingParticles; /**< Ignore particles for which no PDFs are found. */

  };
} // Belle2 namespace
#endif
