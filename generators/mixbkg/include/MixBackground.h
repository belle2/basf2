/**************************************************************************
 * Belle II detector background library                                   *
 * Copyright(C) 2011  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MIXBACKGROUND_H
#define MIXBACKGROUND_H

#include <generators/mixbkg/DetectorBackground.h>

#include <string>
#include <map>

namespace Belle2 {

  namespace generators {

    /**
     * The background mixing library.
     *
     * This library mixes the SimHits from different background contributions
     * to the signal simulation.
     *
     * It follows a hierarchical approach where each background is divided into the
     * subdetector it affects, the component (process) it describes and the generator it
     * was produced with.
     */
    class MixBackground {

    public:

      /** Constructor. */
      MixBackground() : m_mcParticleWriteMode(-1) {};

      /** Destructor. */
      ~MixBackground();

      /**
       * Sets the analysis mode of the mixing library.
       * @param If set to true all background MCParticles and background information are stored into separate collections.
       */
      void setAnalysisMode(bool analysisMode = true) { m_analysisMode = analysisMode; }

      /**
       * Adds a new ROF Root file to the background mixing library.
       * The file content is read and the file is automatically loaded into the correct Component/Generator section.
       * @param filename The filename of the ROF Root file. Follows the same rules as explained in the TChain.Add() method.
       * */
      void addFile(const std::string& filename);

      /**
       * Returns the number of subdetectors which contain background SimHits.
       * @return Number of subdetectors which are overlaid with background.
       */
      unsigned int getNumberSubdetectors() { return m_detectorBackgrounds.size(); }

      /**
       * Returns the first subdetector which contains the background data.
       * @return Pointer to the first background subdetector.
       */
      DetectorBackgroundBase* getFirstSubdetector();

      /**
       * Returns the next subdetector which contains the background data.
       * Please make sure the method getFirstSubdetector() was called before
       * you call this method.
       * @return Pointer to the next background subdetector.
       */
      DetectorBackgroundBase* getNextSubdetector();

      /**
       * Mix the background SimHits into the DataStore.
       * Every time this method is called the internal frame counter in each
       * background components is increased by one. If the internal counter
       * of a background component is greater than the available number of frames,
       * the counter starts from 0.
       */
      void fillDataStore();


    protected:

      int m_mcParticleWriteMode; /**< The MonteCarlo write mode. Makes sure only files having the same mode are loaded into the library. */
      bool m_analysisMode; /**< Activates the analysis mode which stores all background MCParticles and background information into separate collections. */
      std::map<int, DetectorBackgroundBase*> m_detectorBackgrounds;     /**< Map of the created detector backgrounds.*/
      std::map<int, DetectorBackgroundBase*>::iterator m_FirstNextIter; /**< Internal iterator which is used for the getFirst* and getNext* methods. */
    };
  }
}


#endif /* MIXBACKGROUND_H */
