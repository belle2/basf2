/**************************************************************************
 * Belle II detector background library                                   *
 * Copyright(C) 2011  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef DETECTORBACKGROUND_H
#define DETECTORBACKGROUND_H

#include <framework/core/FrameworkExceptions.h>
#include <framework/logging/Logger.h>
#include <background/mixbkg/Component.h>

#include <typeinfo>
#include <string>
#include <map>


namespace Belle2 {

  namespace background {

    /**
     * The base class for each subdetector background contribution.
     */
    class DetectorBackgroundBase {
    public:

      /** Constructor. */
      DetectorBackgroundBase() {};

      /** Destructor. */
      virtual ~DetectorBackgroundBase() {};

      /**
       * Returns the name of the SimHit class which is managed by this subdetector.
       *
       * @return The name of the SimHit class.
       */
      virtual std::string getSimHitClassname() = 0;

      /**
       * Adds a new ROOT ROF file to the subdetector.
       *
       * @param component The name of the background component (e.g. Touschek)
       * @param generator The name of the background generator (e.g. SAD_LER)
       * @param filename The filename of the ROOT ROF file which should be added to the mixing library.
       * @param simHitCollection The name of the collection into which the SimHits should be added.
       * @param simHitRelation The name of the collection which connects the SimHits with the MCParticles.
       */
      virtual void addFile(const std::string& component, const std::string& generator,
                           const std::string& filename, const std::string& simHitCollection,
                           const std::string& simHitRelation) = 0;

      /**
       * Returns the highest number of readout frames this subdetector contains.
       * The value is given by the highest number of the readout frames among all background
       * components and their generators of the subdetector.
       * @return The maximum number of readout frames.
       */
      virtual unsigned int getMaxNumberReadoutFrames() = 0;

      /**
       * Returns the lowest number of readout frames this subdetector contains.
       * The value is given by the lowest number of the readout frames among all background
       * components and their generators of the subdetector.
       * If you would like to add no background readout frame twice for this subdetector,
       * call the method fillDataStore() only as often as the number that this method returns.
       * @return The minimum number of readout frames.
       */
      virtual unsigned int getMinNumberReadoutFrames() = 0;

      /**
       * Mix the background SimHits into the DataStore.
       * Every time this method is called the internal frame counter in each
       * background components is increased by one. If the internal counter
       * of a background component is greater than the available number of frames,
       * the counter starts from 0.
       *
       * @param debugMode If set to true all background MCParticles and background information are stored into separate collections.
       */
      virtual void fillDataStore(bool debugMode = false) = 0;
    };

    //=========================================================================
    /**
     * The template detector background class.
     * This class represents the background contribution for each subdetector.
     */
    template<class SIMHITS>
    class DetectorBackground : public DetectorBackgroundBase {

    public :

      //Exception definition
      /** The exception is thrown if the index for the component is out of bounds. */
      BELLE2_DEFINE_EXCEPTION(ComponentOutOfBounds, "The index for the component is out of bounds !");

      /** Constructor. */
      DetectorBackground() {};

      /** Destructor. */
      virtual ~DetectorBackground();

      /**
       * Returns the name of the SimHit class which is managed by this subdetector.
       *
       * @return The name of the SimHit class.
       */
      std::string getSimHitClassname() { return SIMHITS::Class_Name(); };

      /**
       * Adds a new ROOT ROF file to the subdetector.
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
       * Adds or retrieves a background component.
       * If a background component with the specified name does not yet exist it is created otherwise the existing component is returned.
       *
       * @param name The name of the background component which should be added/retrieved (e.g. Touschek)
       * @return Reference to the background component.
       */
      Component<SIMHITS>& addComponent(const std::string& name);

      /**
       * Returns the highest number of readout frames this subdetector contains.
       * The value is given by the highest number of the readout frames among all background
       * components and their generators of the subdetector.
       * @return The maximum number of readout frames.
       */
      unsigned int getMaxNumberReadoutFrames();

      /**
       * Returns the lowest number of readout frames this subdetector contains.
       * The value is given by the lowest number of the readout frames among all background
       * components and their generators of the subdetector.
       * If you would like to add no background readout frame twice for this subdetector,
       * call the method fillDataStore() only as often as the number which this method returns.
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


    protected :


    private:

      std::map<std::string, Component<SIMHITS>* > m_components; /**< The map of background components for this subdetector. */
      typedef typename std::map<std::string, Component<SIMHITS>* >::iterator MapIterator; /**< Convenient typdef for the map iterator. */
    };


    //===================================
    //  Implementation
    //===================================

    template<class SIMHITS>
    DetectorBackground<SIMHITS>::~DetectorBackground()
    {
      for (MapIterator mapIter = m_components.begin(); mapIter != m_components.end(); ++mapIter) {
        delete mapIter->second;
      }
      m_components.clear();
    }


    template<class SIMHITS>
    inline void DetectorBackground<SIMHITS>::addFile(const std::string& component, const std::string& generator,
                                                     const std::string& filename, const std::string& simHitCollection,
                                                     const std::string& simHitRelation)
    {
      //Check if the component with the name already exists. If not create the component.
      MapIterator mapIter = m_components.find(component);
      if (mapIter == m_components.end()) {
        B2INFO(">> Created new background component '" << component << "' for '" << SIMHITS::Class_Name() << "'.");
        addComponent(component).addFile(component, generator, filename, simHitCollection, simHitRelation);
      } else mapIter->second->addFile(component, generator, filename, simHitCollection, simHitRelation);
    }


    template<class SIMHITS>
    inline Component<SIMHITS>& DetectorBackground<SIMHITS>::addComponent(const std::string& name)
    {
      Component<SIMHITS>* newComponent = new Component<SIMHITS>();
      m_components.insert(make_pair(name, newComponent));
      return *newComponent;
    }


    template<class SIMHITS>
    inline unsigned int DetectorBackground<SIMHITS>::getMaxNumberReadoutFrames()
    {
      unsigned int maxNum = 0;
      for (MapIterator mapIter = m_components.begin(); mapIter != m_components.end(); ++mapIter) {
        if (mapIter->second->getMaxNumberReadoutFrames() > maxNum) {
          maxNum = mapIter->second->getMaxNumberReadoutFrames();
        }
      }
      return maxNum;
    }


    template<class SIMHITS>
    inline unsigned int DetectorBackground<SIMHITS>::getMinNumberReadoutFrames()
    {
      unsigned int minNum = 0;
      for (MapIterator mapIter = m_components.begin(); mapIter != m_components.end(); ++mapIter) {
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
    void DetectorBackground<SIMHITS>::fillDataStore(bool analysisMode)
    {
      for (MapIterator mapIter = m_components.begin(); mapIter != m_components.end(); ++mapIter) {
        mapIter->second->fillDataStore(analysisMode);
      }
    }
  }
}


#endif /* DETECTORBACKGROUND_H */
