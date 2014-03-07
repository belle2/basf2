/**************************************************************************
 * Belle II detector background library                                   *
 * Copyright(C) 2011  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef COMPONENT_H
#define COMPONENT_H

#include <framework/core/FrameworkExceptions.h>
#include <framework/logging/Logger.h>
#include <background/mixbkg/Generator.h>

#include <string>
#include <map>


namespace Belle2 {

  namespace background {

    /**
     * This class represents a background component (e.g. Touschek)
     * It consists of various generators which produced this background component.
     */
    template<class SIMHITS>
    class Component {

    public :

      /** Constructor. */
      Component() : m_enabled(true) {};

      /** Destructor. */
      ~Component();

      /**
       * Adds a new ROOT ROF file to the component.
       *
       * @param component The name of the background component (e.g. Touschek)
       * @param generator The name of the background generator (e.g. SAD_LER)
       * @param filename The filename of the ROOT ROF file which should be added to the mixing library.
       * @param simHitCollection The name of the collection into which the SimHits should be added.
       * @param simHitRelation The name of the collection which connects the SimHits with the MCParticles.
       */
      void addFile(const std::string& component, const std::string& generator,
                   const std::string& filename, const std::string& simHitCollection,
                   const std::string& simHitRelation);

      /**
       * Returns if this component is enabled or disabled.
       * @return True if the component is enabled, false if it disabled.
       */
      bool isEnabled() { return m_enabled; }

      /** Enables this component.
       * Only if the component is enabled, its content is mixed into the DataStore.
       * @param enabled If true the component is enabled if false it is disabled.
       */
      void enable(bool enabled = true) { m_enabled = enabled; }

      /**
       * Adds or retrieves a background generator.
       * If a background generator with the specified name does not yet exist it is created otherwise the existing generator is returned.
       *
       * @param name The name of the background generator which should be added/retrieved (e.g. SAD_LER)
       * @return Reference to the background generator.
       */
      Generator<SIMHITS>& addGenerator(const std::string& name);

      /**
       * Returns the highest number of readout frames this component contains.
       * The value is given by the highest number of the readout frames among all generators.
       * @return The maximum number of readout frames.
       */
      unsigned int getMaxNumberReadoutFrames();

      /**
       * Returns the lowest number of readout frames this component contains.
       * The value is given by the lowest number of the readout frames among all background generators.
       * @return The minimum number of readout frames.
       */
      unsigned int getMinNumberReadoutFrames();

      /**
       * Mix the background SimHits into the DataStore.
       * Every time this method is called the internal frame counter in each
       * background components is increased by one. If the internal counter
       * of a background component is greater than the available number of frames,
       * the counter starts from 0.
       *
       * @param analysisMode If set to true all background MCParticles and background information are stored into separate collections.
       */
      void fillDataStore(bool analysisMode = false);


    private:

      bool m_enabled; /**< Specifies if this component is enabled or disabled. */
      std::map<std::string, Generator<SIMHITS>* > m_generators; /**< The map of generators for this component. */
      typedef typename std::map<std::string, Generator<SIMHITS>* >::iterator MapIterator; /**< Convenient typedef for the map iterator. */
    };


    //===================================
    //  Implementation
    //===================================
    template<class SIMHITS>
    Component<SIMHITS>::~Component()
    {
      for (MapIterator mapIter = m_generators.begin(); mapIter != m_generators.end(); ++mapIter) {
        delete mapIter->second;
      }
      m_generators.clear();
    }


    template<class SIMHITS>
    inline void Component<SIMHITS>::addFile(const std::string& component, const std::string& generator,
                                            const std::string& filename, const std::string& simHitCollection,
                                            const std::string& simHitRelation)
    {
      //Check if the generator with the name already exists. If not create the generator.
      MapIterator mapIter = m_generators.find(generator);
      if (mapIter == m_generators.end()) {
        B2INFO(">> Created new background generator '" << generator << "' for '" << SIMHITS::Class_Name() << "/" << component << "'.");
        addGenerator(generator).addFile(component, generator, filename, simHitCollection, simHitRelation);
      } else mapIter->second->addFile(component, generator, filename, simHitCollection, simHitRelation);
    }


    template<class SIMHITS>
    inline Generator<SIMHITS>& Component<SIMHITS>::addGenerator(const std::string& name)
    {
      Generator<SIMHITS>* newGenerator = new Generator<SIMHITS>();
      m_generators.insert(make_pair(name, newGenerator));
      return *newGenerator;
    }


    template<class SIMHITS>
    inline unsigned int Component<SIMHITS>::getMaxNumberReadoutFrames()
    {
      unsigned int maxNum = 0;
      for (MapIterator mapIter = m_generators.begin(); mapIter != m_generators.end(); ++mapIter) {
        if (mapIter->second->getMaxNumberReadoutFrames() > maxNum) {
          maxNum = mapIter->second->getMaxNumberReadoutFrames();
        }
      }
      return maxNum;
    }


    template<class SIMHITS>
    inline unsigned int Component<SIMHITS>::getMinNumberReadoutFrames()
    {
      unsigned int minNum = 0;
      for (MapIterator mapIter = m_generators.begin(); mapIter != m_generators.end(); ++mapIter) {
        if ((minNum == 0) && (mapIter->second->getMinNumberReadoutFrames() > 0)) {
          minNum = mapIter->second->getMinNumberReadoutFrames();
        } else {
          if (mapIter->second->getMinNumberReadoutFrames() < minNum) {
            minNum = mapIter->second->getMinNumberReadoutFrames();
          }
        }
      }
      return minNum;
    }


    template<class SIMHITS>
    void Component<SIMHITS>::fillDataStore(bool analysisMode)
    {
      if (!m_enabled) return;

      for (MapIterator mapIter = m_generators.begin(); mapIter != m_generators.end(); ++mapIter) {
        mapIter->second->fillDataStore(analysisMode);
      }
    }

  }
}


#endif /* COMPONENT_H */
