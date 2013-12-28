/**************************************************************************
 * Belle II detector background library                                   *
 * Copyright(C) 2011  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GENERATOR_H
#define GENERATOR_H

#include <framework/core/FrameworkExceptions.h>
#include <framework/logging/Logger.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/dataobjects/RelationElement.h>

#include <mdst/dataobjects/MCParticle.h>
#include <generators/dataobjects/BackgroundInfo.h>

#include <TChain.h>
#include <TClonesArray.h>

#include <string>


namespace Belle2 {

  namespace background {

    /**
     * This class represents a background generator (e.g. SAD_LER).
     * It manages a list of the ROOT ROF files from which the SimHits are read and added to
     * the DataStore.
     */
    template<class SIMHITS>
    class Generator {

    public :

      /** Constructor. */
      Generator();

      /** Destructor. */
      ~Generator();

      /**
       * Adds a new ROOT ROF file to the generator.
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
       * Returns if this generator is enabled or disabled.
       * @return True if the generator is enabled, false if it disabled.
       */
      bool isEnabled() { return m_enabled; }

      /** Enables this generator.
       * Only if the generator is enabled, its content is mixed into the DataStore.
       * @param enabled If true the generator is enabled if false it is disabled.
       */
      void enable(bool enabled = true) { m_enabled = enabled; }

      /**
       * Returns the highest number of readout frames this generator contains.
       * @return The maximum number of readout frames.
       */
      unsigned int getMaxNumberReadoutFrames();

      /**
       * Returns the lowest number of readout frames this generator contains.
       * @return The minimum number of readout frames.
       */
      unsigned int getMinNumberReadoutFrames() { return getMaxNumberReadoutFrames(); };

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

      /** If necessary, this method calculates the total number of readout frames which are available in this generator. */
      void updateNumberROF();


    private:

      bool m_enabled;                 /**< Specifies if this component is enabled or disabled. */
      TChain* m_files;                /**< The TChain of ROF root files. */
      int m_numberROFs;               /**< Caches the number of ROFs. */
      int m_index;                    /**< The index of the current ROF. */
      TClonesArray* m_readoutFrame;   /**< The current readout frame (a TClonesArray of SimHits). */
      TClonesArray* m_mcParticles;    /**< The current MCParticle list (a TClonesArray of MCParticles). */
      TClonesArray* m_mcPartRels;     /**< The current relation between a MCParticle and a SimHit (a TClonesArray of relations). */
      std::string m_simHitCollection; /**< The name of the SimHit Collection. */
      std::string m_simHitRelation;   /**< The name of the SimHit to MCParticle Collection. */

      std::string m_component;        /**< The name of the component which contains this generator. */
      std::string m_generator;        /**< The name of this generator. */
    };


    //===================================
    //  Implementation
    //===================================
    template<class SIMHITS>
    Generator<SIMHITS>::Generator() : m_enabled(true), m_files(new TChain("ROFTree")), m_numberROFs(-1), m_index(0),
      m_readoutFrame(new TClonesArray(SIMHITS::Class_Name())),
      m_mcParticles(new TClonesArray(MCParticle::Class_Name())),
      m_mcPartRels(new TClonesArray(RelationElement::Class_Name())),
      m_simHitCollection(""), m_simHitRelation(""),
      m_component(""), m_generator("")
    {
      m_files->SetBranchAddress("ReadoutFrames", &m_readoutFrame);
      m_files->SetBranchAddress("MCParticles", &m_mcParticles);
      m_files->SetBranchAddress("MCPartRels", &m_mcPartRels);
    }


    template<class SIMHITS>
    Generator<SIMHITS>::~Generator()
    {
      m_files->Clear();

      delete m_readoutFrame;
      delete m_mcParticles;
      delete m_mcPartRels;
    }


    template<class SIMHITS>
    inline void Generator<SIMHITS>::addFile(const std::string& component, const std::string& generator,
                                            const std::string& filename, const std::string& simHitCollection,
                                            const std::string& simHitRelation)
    {
      //Check if the collection name is consistent
      if ((!m_simHitCollection.empty()) && (m_simHitCollection.compare(simHitCollection) != 0)) {
        B2FATAL("The name of the added SimHit collection (" << simHitCollection << ") differs from the existing one (" << m_simHitCollection << ") !");
      }

      if ((!m_simHitRelation.empty()) && (m_simHitRelation.compare(simHitRelation) != 0)) {
        B2FATAL("The name of the added Relation collection (" << simHitRelation << ") differs from the existing one (" << m_simHitRelation << ") !");
      }

      m_simHitCollection = simHitCollection;
      m_simHitRelation = simHitRelation;

      m_component = component;
      m_generator = generator;

      //Add the file to the chain
      m_numberROFs = -1;
      m_files->Add(filename.c_str());

      //Check if the file is valid
      if (m_files->IsZombie()) {
        B2FATAL("The background ROF file " << filename << " is not valid !");
      }

      B2INFO(">> Added file '" << filename << "' to '" << SIMHITS::Class_Name() << "/" << component << "/" << generator << "'")
    }


    template<class SIMHITS>
    inline unsigned int Generator<SIMHITS>::getMaxNumberReadoutFrames()
    {
      updateNumberROF();
      return m_numberROFs;
    }


    template<class SIMHITS>
    inline void Generator<SIMHITS>::fillDataStore(bool analysisMode)
    {
      if (!m_enabled) return;

      updateNumberROF();
      if (m_numberROFs == 0) return;
      if (m_index >= m_numberROFs) m_index = 0;

      //Get ROF from chain
      m_files->GetEntry(m_index);
      m_index++;

      //Get the collections and relations
      StoreArray<SIMHITS> simHitArray(m_simHitCollection);
      StoreArray<MCParticle> mcPartCollection;
      RelationArray simHitToMCPartCollection(mcPartCollection, simHitArray, m_simHitRelation);

      //Loop over the SimHit content of the ROF and add the SimHits to the DataStore SimHit collection.
      int simHitIndexOffset = simHitArray.getEntries(); //The index offset in the global SimHit DataStoreArray
      int nSimHits = m_readoutFrame->GetEntriesFast();
      for (int iSimHit = 0; iSimHit < nSimHits; ++iSimHit) {
        simHitArray.appendNew(SIMHITS(*(dynamic_cast<SIMHITS*>(m_readoutFrame->At(iSimHit)))));
      }

      //Loop over the SimHit to MCParticle relation and add the MCParticles and Relations
      int mcPartIndexOffset = mcPartCollection.getEntries(); //The index offset in the global MCParticle DataStoreArray
      int mcIndex = mcPartIndexOffset;
      int nRel = m_mcPartRels->GetEntriesFast();
      std::vector<int> addedToCol;
      addedToCol.resize(m_mcParticles->GetEntriesFast(), -1);
      for (int iRel = 0; iRel < nRel; ++iRel) {
        RelationElement* relation = dynamic_cast<RelationElement*>(m_mcPartRels->At(iRel));
        MCParticle& currParticle = *(dynamic_cast<MCParticle*>(m_mcParticles->At(relation->getFromIndex())));

        //Check if the MCParticle has already been added to the collection
        if (addedToCol[relation->getFromIndex()] < 0) {
          addedToCol[relation->getFromIndex()] = mcIndex;

          //Create a new MCParticle to make sure the mother and daughter information is set to the value 0.
          MCParticle newParticle;
          newParticle.setStatus(currParticle.getStatus());
          newParticle.setPDG(currParticle.getPDG());
          newParticle.setMass(currParticle.getMass());
          newParticle.setCharge(currParticle.getCharge());
          newParticle.setEnergy(currParticle.getEnergy());
          newParticle.setMomentum(currParticle.getMomentum());
          newParticle.setValidVertex(currParticle.hasValidVertex());
          newParticle.setProductionTime(currParticle.getProductionTime());
          newParticle.setProductionVertex(currParticle.getProductionVertex());
          newParticle.setDecayTime(currParticle.getDecayTime());
          newParticle.setDecayVertex(currParticle.getDecayVertex());

          //Store the MCParticle and its relation into the appropriate collections
          mcPartCollection.appendNew(newParticle);
          mcIndex++;
        }

        simHitToMCPartCollection.add(addedToCol[relation->getFromIndex()], relation->getToIndex() + simHitIndexOffset);
      }

      //Only continue if the analysis mode is set
      if (!analysisMode) return;

      //Create the analysis mode specific background DataStore collections
      StoreArray<BackgroundInfo> bkgInfoCollection;
      StoreArray<MCParticle> mcPartBkgCollection("BackgroundMCParticles");
      RelationArray bkgInfoRelCollection(mcPartCollection, bkgInfoCollection, "BackgroundInfoRelation");
      RelationArray bkgSimHitRelCollection(mcPartCollection, bkgInfoCollection, "BackgroundMCSimHitRelation");

      //Add the background info to the new collection 'BkgInfo'.
      bkgInfoCollection.appendNew(BackgroundInfo(m_component, m_generator));

      //Add the MCParticles and a relation to the background info
      int nPart = m_mcParticles->GetEntriesFast();
      for (int iPart = 0; iPart < nPart; ++iPart) {
        mcPartBkgCollection.appendNew(MCParticle(*(dynamic_cast<MCParticle*>(m_mcParticles->At(iPart)))));
        bkgInfoRelCollection.add(mcPartBkgCollection.getEntries() - 1, bkgInfoCollection.getEntries() - 1);
      }

      //Add the SimHit to MCParticle relations
      nRel = m_mcPartRels->GetEntriesFast();
      for (int iRel = 0; iRel < nRel; ++iRel) {
        RelationElement* relation = dynamic_cast<RelationElement*>(m_mcPartRels->At(iRel));
        bkgSimHitRelCollection.add(relation->getFromIndex(), relation->getToIndex() + simHitIndexOffset);
      }
    }


    template<class SIMHITS>
    inline void Generator<SIMHITS>::updateNumberROF()
    {
      if (m_numberROFs < 0) {
        m_numberROFs = m_files->GetEntries();
      }
    }
  }
}

#endif /* GENERATOR_H */
