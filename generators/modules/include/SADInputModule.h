/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2012  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Hiroyuki Nakayama                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SADINPUTMODULE_H_
#define SADINPUTMODULE_H_

#include <framework/core/Module.h>
#include <generators/SAD/ReaderSAD.h>

#include <string>

class TGeoMatrix;

namespace Belle2 {

  class MCParticleGraph;

  /**
   * The SAD Input module.
   *
   * Reads in the data from a SAD file and stores it into the MCParticle collection.
   * This module is used to read in the SAD output for Touschek, Radiative Bhabha and Coulomb scattering background.
  */
  class SADInputModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the description and the parameters of the module.
     */
    SADInputModule();

    /** Destructor. */
    virtual ~SADInputModule() {};

    /**
     * Checks the validity of the module parameters.
     * Checks if the filepath of the given filename exists.
     */
    virtual void initialize();

    /** Reads the data and stores it into the MCParticle collection. */
    virtual void event();


  protected:

    ReaderSAD m_reader;     /**< The SAD reader object for the SAD data. */
    TGeoHMatrix* m_PipePartMatrix; /**< Transformation matrix from SAD space into geant4 space. */

    int m_accRing;          /**< The accelerator ring: 0 = LER, 1 = HER. */
    double m_readoutTime;   /**< The readout time of the detector [ns]. */
    int m_readMode;         /**< The read mode: 0 = one SAD particle per event, 1 = one real particle per event, 2 = all SAD particles per event. */
    std::string m_filename; /**< The filename of the SAD file. */
    double m_range;         /**< All particles within the range around the IP are loaded. */
    double m_pxRes;         /**< The resolution for the x momentum component of the SAD real particle. */
    double m_pyRes;         /**< The resolution for the y momentum component of the SAD real particle. */
    double m_rotateParticles;  /**< Rotate the SAD particles around the nominal beam axis (just for unphysical tests !!!). */


  private:

    /** Reads one SAD particle per event into the MCParticle graph.
     * @param reader Reference to the SAD reader from which the data is read.
     * @param mpg Reference to the MCParticle Graph which is filled for each event.
     */
    void readSADParticle(ReaderSAD& reader, MCParticleGraph& mpg);

    /** Reads one real particle per event into the MCParticle graph.
     * @param reader Reference to the SAD reader from which the data is read.
     * @param mpg Reference to the MCParticle Graph which is filled for each event.
     */
    void readRealParticle(ReaderSAD& reader, MCParticleGraph& mpg);
  };

} // end namespace Belle2

#endif /* SADINPUTMODULE_H_ */
