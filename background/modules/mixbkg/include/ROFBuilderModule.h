/**************************************************************************
 * Belle II detector background library                                   *
 * Copyright(C) 2011  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ROFBUILDERMODULE_H
#define ROFBUILDERMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/dataobjects/RelationElement.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/MCParticleGraph.h>

#include <TFile.h>
#include <TTree.h>
#include <TClonesArray.h>
#include <TRandom.h>

#include <string>
#include <vector>
#include <map>
#include <algorithm>

namespace Belle2 {

  /**
   * Class RandomTimer: Provide exponentially distributied arrival times of beam
   * background events.
   * The class also wraps back the time sequence when the end of the acceptance window
   * has been reached.
   */
  class RandomTimer {
  public:
    /** Constructor.
     * @param tau time per event
     * @param start start time of acceptance window.
     * @param size size of acceptance window.
     */
    RandomTimer(float tau, float start, float size):
      m_tau(tau), m_end(start + size), m_time(start), m_endFrame(false), m_overFlow(false)
    { m_size = static_cast<float>(size); }
    /** Get next event time.
     * @return time to be set for the following event.
     */
    float getNextTime() {
      if (!m_overFlow) m_time += static_cast<float>(gRandom->Exp(m_tau));
      if (m_time > m_end) {
        m_time -= m_size;
        if (m_time > m_end) m_overFlow = true;
        m_endFrame = true;
      } else {
        m_endFrame = false;
        m_overFlow = false;
      }
      return m_time;
    }
    /** Check for end of frame.
     * @return end-of-frame flag.
     */
    bool isEndOfFrame() const {return m_endFrame;}
    /** Check for overflow flag.
     * @return overflow flag (if acceptance window is less than time per event).
     */
    bool isOverFlow() const {return m_overFlow; }
  private:
    double m_tau;   /**< mean time between events.*/
    float m_size; /**< size of acceptance window (in time units.*/
    double m_end;   /**< end time of acceptance window.*/
    float m_time;  /**< current time within the window. */
    bool m_endFrame; /**< indicates end-of-frame status. */
    bool m_overFlow; /**< indicates that current time is longer than event time.*/
  };


  /**
   * The ROFBuilder module.
   *
   * This module builds the readout frames for the background mixing.
   * It generates a ROOT file containing readout frames for one
   * subdetector, one background process and generator. Each readout frame
   * is defined as a collection of SimHits.
   *
   * The Root file contains a tree containing the content of the file,
   * which makes the output of this module self-describing.
   *
   * Since the MCParticle information might be required for the digitization, it is
   * stored along with the SimHits. In order to minimize the data, only the MCParticles
   * which caused a SimHit are saved by default. But can optionally be extended by setting
   * the 'MCParticleWriteMode' parameter of the module.
   *
   * The 'time-aware" mode of the ROFBuilder expects random order of input events
   * and builds sequences of SimHits (and the underlying MCParticles, if
   * desired) randomized in event occurrence times. If the input sequence has to
   * be randomized, use the EventRandomizer module.
   * For the time-aware mode, set the corresponding parameter to true, and set
   * the start time and size of the acceptance window of your subdetector (in ns).
   * Also specify the time interval equivalent of the input data (in us).
   * There is no fixed amount of events per ROF, since the number is randomized,
   * and the corresponding parameter only has effect in the "timeless" mode.
   *
   * NOTES ON CURRENT IMPLEMENTATION (October 2012)
   * To make the module work with a specific subdetector,
   * 1. the subdetector's SimHits have to inherit from the SimHitBase class (currently in
   * generators/dataobjects) and
   * 2. (if time-aware mode is desired), re-implement SimHitBase::shiftInTime(float delta)
   * to do something like m_simHitTime += delta, whatever simHitTime is and is
   * called.
   * 3. The subdetector than has to be added to the list of values for the
   * ROFBuilder::m_subdetector andm_simHitClassNames (I will do this for all
   * subdetectors).
   * Please look at PXDSimHit or SVDSimHit implementations to see what to do.
   * In particular, don't forget to change the version number of your SimHit in
   * the ClassDef(..) call.
   * By inheriting from SimHitBase, your SimHits also inherit the m_backgroundTag
   * attribute (and the corresponding setter and getter), by which their origin
   * can later be identified.
   * - A principal problem with this implementation is that it effectively
   * discards long-lived background - the acceptance window applies to event (=
   * mother MCParticle) times rather than to SimHits, so SimHits that take time
   * to appear will simply be outside of the acceptance. My impression is that
   * there is a measurable fraction of delayed background, so this will have to
   * be improved.
   * CHANGE SEPTEMBER 2013:
   * The event-randomization feature was removed from ROFBuilderModule. Use the
   * EventRandomizer module to randomize input events. As before, be aware that
   * event randomization takes lots of time.
   */
  class ROFBuilderModule : public Module {

  public:

    /** Constructor. */
    ROFBuilderModule();

    /** Destructor. */
    virtual ~ROFBuilderModule() {}

    /** Initializes the module. */
    virtual void initialize();

    /** Method is called for each event. */
    virtual void event();

    /** Terminates the module. */
    virtual void terminate();


  protected:

    //Parameters
    int m_subdetector;                   /**< The subdetector for which the ROF file is created (1 = PXD, 2 = SVD). */
    double      m_eventsPerReadoutFrame; /**< The number of events that represent one readout frame. */
    std::string m_simHitCollectionName;  /**< The name of the SimHit collection. */
    std::string m_outputRootFileName;    /**< The name of the output Root file. */
    std::string m_componentName;         /**< The name of the background component (e.g. Touschek, QED etc.). */
    std::string m_generatorName;         /**< The name of the generator which was used to create the background (e.g. SAD_LER). */
    unsigned int m_backgroundTag;        /**< The background tag for the SimHits in the generated ROFs.*/
    int m_mcParticleWriteMode;           /**< The MCParticle write mode:
                                                                           0 = no MCParticle are saved,
                                                                           1 = only the MCParticle which caused a SimHit in the subdetector (default),
                                                                           2 = like 0 but in addition all mother particles are stored,
                                                                           3 = all MCParticles*/
    std::string m_simHitMCPartRelationName; /**< The name of the SimHit to MCParticle relation. */
    bool m_timeAwareMode;                 /**< If true, the events will be read in random order
                                               and number of events per ROF calculated from time
                                               window data.*/
    double m_windowStart;                 /**< Start of ROF windomw.*/
    double m_windowSize;                  /**< Size of ROF window.*/
    double m_baseSampleSize;              /**< Size of the base data sample.*/
    //Variables
    MCParticleGraph m_rofMCParticleGraph;        /**< The MCParticle graph for one full readout frame. */
    std::vector<int> m_mcpToSimHitMap;           /**< List for one full readout frame that maps a SimHit index (the list index) to a MCParticle index. */
    int m_rofGraphUniqueID;                      /**< Unique identifier for the nodes of the ROF graph. */
    TFile* m_outputRootFile;                     /**< The output root file. */
    TTree* m_rofTree;                            /**< The readout frame root tree. */
    TTree* m_contentTree;                        /**< The header root tree which contains information about the file. */
    TClonesArray* m_readoutFrame;                /**< The current readout frame (a TClonesArray of SimHits). */
    TClonesArray* m_mcParticles;                 /**< The current MCParticle list (a TClonesArray of MCParticles). */
    TClonesArray* m_mcPartRels;                  /**< The current relation between a MCParticle and a SimHit (a TClonesArray of relations). */
    int m_event;                                 /**< The current event number. */
    int m_currReadoutFrameIdx;                   /**< The index of the current readout frame. */
    std::vector<std::string> m_SimHitClassNames; /**< Stores the SimHit class name for each subdetector type. */
    int m_numberSimHits;                         /**< The current number of SimHits. */
    RandomTimer* m_timer;                        /**< The timer generates random event times. */
    float m_eventTime;                           /**< The time shift for current event. */

    static const std::string s_auxMCParticlesName; /**< Name of the StoreArray used for MCParticleGraph::generateList().*/


  private:

    /** Adds the SimHits and MCParticles of the current event to the readout frame. */
    template <class SIMHIT>
    void addSimHitsToROF();

    /** Fill the ROOT readout frame tree and add the MCParticles and the relations if the MCParticle write mode is set. */
    void fillROFTree();

    /** Add an MCParticle to a MCParticleGraph.
     * @param graph Reference to the MCParticleGraph.
     * @param mcParticle Reference to the MCParticle.
     * @param motherIndex Index of the mother particle.
     */
    MCParticleGraph::GraphParticle& createGraphParticle(MCParticleGraph& graph, MCParticle& mcParticle, int motherIndex);

    /**
     * Adds a new particle (+ daughters) to the graph.
     * @param graph Reference to the MCParticleGraph.
     * @param mcParticle Reference to the mother particle.
     * @param motherIndex Index of the mother particle.
     * @param useUniqueID Set it to true to store a unique number for each node into the trackID.
     * @param keepList List which specifies the indices of the MCParticle which should be kept. Set it to NULL to skip the list.
     */
    void addParticleToEventGraph(MCParticleGraph& graph, MCParticle& mcParticle, int motherIndex, const std::vector<bool>& keepList);

    /** Add MCParticle (and its daughters) to the ROF MCParticleGraph, keeping
     * track of their list indices.
     * @param mcParticle reference to the MCParticle
     * @param motherIndex index of mother particle
     * @param uniqueIDList relates MCParticle index (list index) to unique ID
     */
    void addParticleToROFGraph(MCParticle& mcParticle, int motherIndex, std::vector<int>& uniqueIDList);

  };


  //--------------------------------------------------
  //     Implementation of template methods
  //--------------------------------------------------
  template <class SIMHIT>
  void ROFBuilderModule::addSimHitsToROF()
  {
    //------------------------------------------------
    // Collections and RelationIndex
    //------------------------------------------------
    StoreArray<MCParticle> mcPartCollection;
    StoreArray<SIMHIT> collection(m_simHitCollectionName);
    RelationIndex<MCParticle, SIMHIT> mcPartSimHitIndex(mcPartCollection, collection, m_simHitMCPartRelationName);
    if (mcPartCollection.getEntries() > 0) dynamic_cast<MCParticle*>(mcPartCollection[0])->fixParticleList();

    //------------------------------------------------
    // SimHits and their relations to MCParticles
    //------------------------------------------------
    int nSimHits = collection.getEntries();
    if (m_mcParticleWriteMode > 0) m_mcpToSimHitMap.resize(m_mcpToSimHitMap.size() + nSimHits);

    int colIndex = m_readoutFrame->GetLast() + 1;
    int simHitEvtOffset = colIndex;
    for (int iSimHit = 0; iSimHit < nSimHits; ++iSimHit) {
      SIMHIT* origSimHit = collection[iSimHit];
      SIMHIT* newSimHit = new((*m_readoutFrame)[colIndex]) SIMHIT(*origSimHit);
      // In time-aware mode, shift the SimHit in time
      if (m_timeAwareMode) newSimHit->shiftInTime(m_eventTime);
      // Set the background flag of the SimHit
      newSimHit->setBackgroundTag(m_backgroundTag);

      //Store the MCParticle index for the given SimHit index (list index) in the vector
      if (m_mcParticleWriteMode > 0) {
        if (mcPartSimHitIndex.getFirstElementTo(origSimHit) != NULL) {
          const MCParticle* mcPart = mcPartSimHitIndex.getFirstElementTo(origSimHit)->from;
          if (mcPart != NULL) m_mcpToSimHitMap[colIndex] = mcPart->getArrayIndex();
          else m_mcpToSimHitMap[colIndex] = -1;
        }
      }
      colIndex++;
    }
    m_numberSimHits += nSimHits;


    //-------------------------------------------------------
    // MCParticles (only of the MCParticle write mode is set)
    //-------------------------------------------------------
    if (m_mcParticleWriteMode > 0) {
      int nMCPart = mcPartCollection.getEntries();

      //Create a list of the indices of the MCParticles which should be kept.
      std::vector<bool> keepParticle;
      keepParticle.resize(nMCPart, false);
      switch (m_mcParticleWriteMode) {
        case 1: //seen in the subdetector
          for (int iSimHit = 0; iSimHit < nSimHits; ++iSimHit) {
            SIMHIT* simHit = collection[iSimHit];
            if (mcPartSimHitIndex.getFirstElementTo(simHit) == NULL) continue;
            const MCParticle* mcPart = mcPartSimHitIndex.getFirstElementTo(simHit)->from;
            if (mcPart != NULL) keepParticle[mcPart->getArrayIndex()] = true;
          }
          break;
        case 2: //seen in the subdetector + mothers
          for (int iSimHit = 0; iSimHit < nSimHits; ++iSimHit) {
            SIMHIT* simHit = collection[iSimHit];
            if (mcPartSimHitIndex.getFirstElementTo(simHit) == NULL) continue;
            const MCParticle* mcPart = mcPartSimHitIndex.getFirstElementTo(simHit)->from;
            if (mcPart != NULL) keepParticle[mcPart->getArrayIndex()] = true;
            MCParticle* mother = mcPart->getMother();
            while (mother != NULL) {
              keepParticle[mother->getArrayIndex()] = true;
              mother = mother->getMother();
            }
          }
          break;
        case 3: //all
          for (int iPart = 0; iPart < nMCPart; ++iPart) keepParticle[iPart] = true;
          break;
      }

      //Create a new, compressed MCParticle list for the event.
      //The MCParticle collection has to be sorted breadth first.
      MCParticleGraph eventGraph;
      for (int iPart = 0; iPart < nMCPart; ++iPart) {
        MCParticle* currParticle = mcPartCollection[iPart];
        if (m_timeAwareMode)
          currParticle->setProductionTime(currParticle->getProductionTime() + m_eventTime);
        if (currParticle->getMother() != NULL) break;
        addParticleToEventGraph(eventGraph, *currParticle, 0, keepParticle);
      }
      eventGraph.generateList(s_auxMCParticlesName.c_str(), MCParticleGraph::c_clearParticles);
      StoreArray<MCParticle> mcParticleEventCol(s_auxMCParticlesName.c_str());
      int nMCPartEvent = mcParticleEventCol.getEntries();

      //Create a list that relates the old MCParticle index (list index) with the new (after compressing) MCParticle index.
      std::vector<int> mapOldToNewIndexEvent;
      mapOldToNewIndexEvent.resize(nMCPart, -1);
      for (unsigned int iPart = 0; iPart < eventGraph.size(); ++iPart) {
        MCParticleGraph::GraphParticle& currParticle = eventGraph[iPart];
        mapOldToNewIndexEvent[currParticle.getTrackID()] = currParticle.getArrayIndex();
      }

      //Add the compressed MCParticle list to the readout-frame MCParticleGraph.
      //Give each node in the graph an unique identifier (since vertexID is not accessible we 'misuse' the trackID)
      //and create a list which relates the event MCParticle indices with the unique identifiers.
      std::vector<int> mcpToUniqueIDMap;
      mcpToUniqueIDMap.resize(nMCPartEvent);
      for (int iPart = 0; iPart < nMCPartEvent; ++iPart) {
        MCParticle* currParticle = mcParticleEventCol[iPart];
        if (currParticle->getMother() != NULL) break;
        addParticleToROFGraph(*currParticle, 0, mcpToUniqueIDMap);
      }

      //Replace the values of the ROF SimHit list (m_mcpToSimHitMap) with the unique identifiers
      for (unsigned int iSimHit = simHitEvtOffset; iSimHit < m_mcpToSimHitMap.size(); ++iSimHit) {
        if (m_mcpToSimHitMap[iSimHit] > -1) {

          //Replace the original MCParticle index with the one after the compression
          m_mcpToSimHitMap[iSimHit] = mapOldToNewIndexEvent[m_mcpToSimHitMap[iSimHit]];

          //Replace the updated MCParticle index with the unique ID of the ROF MCParticle graph
          if (m_mcpToSimHitMap[iSimHit] > -1) {
            m_mcpToSimHitMap[iSimHit] = mcpToUniqueIDMap[m_mcpToSimHitMap[iSimHit]];
          }
        }
      }
    }
  }

} // end namespace Belle2


#endif /* ROFBUILDERMODULE_H */
