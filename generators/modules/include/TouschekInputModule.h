/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOUSCHEKINPUTMODULE_H_
#define TOUSCHEKINPUTMODULE_H_

#include <framework/core/Module.h>
#include <generators/touschek/TouschekReader.h>
#include <generators/dataobjects/MCParticleGraph.h>

#include <TGeoMatrix.h>
#include <string>

namespace Belle2 {

  namespace Generators {

    /**
     * The Touschek Input module.
     *
     * Reads in the Touschek data from a Touschek file and stores it into
     * the MCParticle collection.
    */
    class TouschekInputModule : public Module {

    public:

      /**
       * Constructor.
       * Sets the description and the parameters of the module.
       */
      TouschekInputModule();

      /** Destructor. */
      virtual ~TouschekInputModule();

      /**
       * Checks the validity of the module parameters.
       * Checks if the filepath of the given filename exists.
       */
      virtual void initialize();

      /** Reads the data and stores it into the MCParticle collection. */
      virtual void event();


    protected:

      int m_evtNumber; /**< The current event number. */

      TouschekReader* m_readerHER;      /**< The Touschek reader object for the HER data. */
      TouschekReader* m_readerLER;      /**< The Touschek reader object for the LER data. */
      TGeoHMatrix* m_herPipePartMatrix; /**< HER transformation matrix from TURTLE space into geant4 space. */
      TGeoHMatrix* m_lerPipePartMatrix; /**< LER transformation matrix from TURTLE space into geant4 space. */

      std::string m_filenameHER; /**< The filename of the HER TURTLE Touschek file. */
      std::string m_filenameLER; /**< The filename of the LER TURTLE Touschek file. */
      bool m_readHER;            /**< If set to true reads the HER data and adds it to the MCParticle collection. */
      bool m_readLER;            /**< If set to true reads the LER data and adds it to the MCParticle collection. */
      int  m_maxParticles;       /**< The maximum number of particles per event that should be read. -1 means all of the particles are read. */
    };

  }//end namespace Generators

} // end namespace Belle2

#endif /* TOUSCHEKINPUTMODULE_H_ */
