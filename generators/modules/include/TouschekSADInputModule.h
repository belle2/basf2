/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOUSCHEKSADINPUTMODULE_H_
#define TOUSCHEKSADINPUTMODULE_H_

#include <framework/core/Module.h>
#include <generators/touschek/TouschekReaderSAD.h>

#include <string>

class TGeoMatrix;

namespace Belle2 {

  class MCParticleGraph;

  /**
   * The TouschekSAD Input module.
   *
   * Reads in the Touschek data from a SAD file and stores it into
   * the MCParticle collection.
   * This module requires that the interaction region XML file has already been loaded by the Gearbox module.
  */
  class TouschekSADInputModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the description and the parameters of the module.
     */
    TouschekSADInputModule();

    /** Destructor. */
    virtual ~TouschekSADInputModule() {};

    /**
     * Checks the validity of the module parameters.
     * Checks if the filepath of the given filename exists.
     */
    virtual void initialize();

    /** Reads the data and stores it into the MCParticle collection. */
    virtual void event();


  protected:

    TouschekReaderSAD m_readerLER;    /**< The Touschek reader object for the LER data. */
    TGeoHMatrix* m_lerPipePartMatrix; /**< LER transformation matrix from Touschek space into geant4 space. */

    double m_readoutTime;      /**< The readout time of the detector [ns]. */
    int m_readMode;            /**< The read mode: 0 = one real particle per event, 1 = all SAD particles per event. */
    std::string m_filenameLER; /**< The filename of the LER TURTLE Touschek file. */
    double m_rangeLER;         /**< All particles within the range around the IP are loaded. */
    double m_beamEnergyLER;    /**< The beam energy of the LER [GeV]. */
    double m_currentLER;       /**< The current of the LER [A]. */
    double m_lifetimeLER;      /**< The Touschek lifetime of the LER [ns]. */
    double m_pxResLER;         /**< The resolution for the x momentum component of the Touschek LER real particle. */
    double m_pyResLER;         /**< The resolution for the y momentum component of the Touschek LER real particle. */
    double m_rotateParticles;  /**< Rotate the SAD particles around the nominal beam axis (just for unphysical tests !!!). */


  private:

    /** Reads one SAD particle per event into the MCParticle graph.
     * @param reader Reference to the Touschek SAD reader from which the data is read.
     * @param mpg Reference to the MCParticle Graph which is filled for each event.
     */
    void readSADParticle(TouschekReaderSAD& reader, MCParticleGraph& mpg);

    /** Reads one real particle per event into the MCParticle graph.
     * @param reader Reference to the Touschek SAD reader from which the data is read.
     * @param mpg Reference to the MCParticle Graph which is filled for each event.
     */
    void readRealParticle(TouschekReaderSAD& reader, MCParticleGraph& mpg);
  };

} // end namespace Belle2

#endif /* TOUSCHEKSADINPUTMODULE_H_ */
