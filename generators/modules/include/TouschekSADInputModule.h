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
#include <generators/dataobjects/MCParticleGraph.h>

#include <TGeoMatrix.h>
#include <string>

namespace Belle2 {

  namespace Generators {

    /**
     * The TouschekSAD Input module.
     *
     * Reads in the Touschek data from a SAD file and stores it into
     * the MCParticle collection.
    */
    class TouschekSADInputModule : public Module {

    public:

      /**
       * Constructor.
       * Sets the description and the parameters of the module.
       */
      TouschekSADInputModule();

      /** Destructor. */
      virtual ~TouschekSADInputModule();

      /**
       * Checks the validity of the module parameters.
       * Checks if the filepath of the given filename exists.
       */
      virtual void initialize();

      /** Reads the data and stores it into the MCParticle collection. */
      virtual void event();


    protected:

      TouschekReaderSAD* m_readerLER;   /**< The Touschek reader object for the LER data. */
      TGeoHMatrix* m_lerPipePartMatrix; /**< LER transformation matrix from Touschek space into geant4 space. */

      std::string m_filenameLER; /**< The filename of the LER TURTLE Touschek file. */
      int  m_maxParticles; /**< The maximum number of particles per event that should be read. */
    };

  }//end namespace Generators

} // end namespace Belle2

#endif /* TOUSCHEKSADINPUTMODULE_H_ */
