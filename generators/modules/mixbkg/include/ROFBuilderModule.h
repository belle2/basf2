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
#include <generators/dataobjects/MCParticle.h>
#include <generators/dataobjects/MCParticleGraph.h>

#include <pxd/dataobjects/PXDSimHit.h>

#include <TFile.h>
#include <TTree.h>
#include <TClonesArray.h>

#include <string>
#include <vector>
#include <map>

namespace Belle2 {

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
    int m_mcParticleWriteMode;           /**< The MCParticle write mode:
                                                                           0 = no MCParticle are saved,
                                                                           1 = only the MCParticle which caused a SimHit in the subdetector (default),
                                                                           2 = like 0 but in addition all mother particles are stored,
                                                                           3 = all MCParticles*/
    std::string m_simHitMCPartRelationName; /**< The name of the SimHit to MCParticle relation. */

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


  private:

    /** Adds the SimHits and MCParticles of the current event to the readout frame. */
    template <class SIMHIT>
    void addSimHitsToROF();

    /** Fill the ROOT readout frame tree and add the MCParticles and the relations if the MCParticle write mode is set. */
    void fillROFTree();

    MCParticleGraph::GraphParticle& createGraphParticle(MCParticleGraph& graph, MCParticle& mcParticle, int motherIndex);

    /**
     * Adds a new particle (+ daughters) to the graph.
     * @param graph Reference to the MCParticleGraph.
     * @param mcParticle Reference to the mother particle.
     * @param motherIndex Index of the mother particle.
     * @param useUniqueID Set it to true to store a unique number for each node into the trackID.
     * @param keepList List which specifies the indices of the MCParticle which should be kept. Set it to NULL to skip the list.
     */
    void addParticleToEventGraph(MCParticleGraph& graph, MCParticle& mcParticle, int motherIndex, const std::vector<bool> &keepList);

    /**
     * uniqueIDList relates MCParticle index (list index) to unique ID
     */
    void addParticleToROFGraph(MCParticle& mcParticle, int motherIndex, std::vector<int> &uniqueIDList);

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
    int nSimHits = collection->GetEntries();
    std::vector<int> mcpToSimHitMapEvent;
    if (m_mcParticleWriteMode > 0) m_mcpToSimHitMap.resize(m_mcpToSimHitMap.size() + nSimHits);

    int colIndex = m_readoutFrame->GetLast() + 1;
    int simHitEvtOffset = colIndex;
    for (int iSimHit = 0; iSimHit < nSimHits; ++iSimHit) {
      SIMHIT* origSimHit = collection[iSimHit];
      new((*m_readoutFrame)[colIndex]) SIMHIT(*origSimHit);

      //Store the MCParticle index for the given SimHit index (list index) in the vector
      if (m_mcParticleWriteMode > 0) {
        if (mcPartSimHitIndex.getFirstFrom(origSimHit) != NULL) {
          const MCParticle* mcPart = mcPartSimHitIndex.getFirstFrom(origSimHit)->from;
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
            if (mcPartSimHitIndex.getFirstFrom(simHit) == NULL) continue;
            const MCParticle* mcPart = mcPartSimHitIndex.getFirstFrom(simHit)->from;
            if (mcPart != NULL) keepParticle[mcPart->getArrayIndex()] = true;
          }
          break;
        case 2: //seen in the subdetector + mothers
          for (int iSimHit = 0; iSimHit < nSimHits; ++iSimHit) {
            SIMHIT* simHit = collection[iSimHit];
            if (mcPartSimHitIndex.getFirstFrom(simHit) == NULL) continue;
            const MCParticle* mcPart = mcPartSimHitIndex.getFirstFrom(simHit)->from;
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
        if (currParticle->getMother() != NULL) break;
        addParticleToEventGraph(eventGraph, *currParticle, 0, keepParticle);
      }
      eventGraph.generateList("ROFBuilderMCParticleEvent");
      StoreArray<MCParticle> mcParticleEventCol("ROFBuilderMCParticleEvent");
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
