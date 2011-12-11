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

#include <generators/dataobjects/MCParticle.h>
#include <generators/dataobjects/BackgroundInfo.h>

#include <TChain.h>
#include <TClonesArray.h>

#include <string>


namespace Belle2 {

  namespace generators {

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
      void addFile(const std::string &component, const std::string &generator,
                   const std::string &filename, const std::string &simHitCollection,
                   const std::string &simHitRelation);

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
       */
      void fillDataStore();


    protected :

      /** If necessary, this method calculates the total number of readout frames which are available in this generator. */
      void updateNumberROF();


    private:

      bool m_enabled;                 /**< Specifies if this component is enabled or disabled. */
      TChain *m_files;                /**< The TChain of ROF root files. */
      int m_numberROFs;               /**< Caches the number of ROFs. */
      int m_index;                    /**< The index of the current ROF. */
      TClonesArray *m_readoutFrame;   /**< The current readout frame (a TClonesArray of SimHits). */
      TClonesArray *m_mcParticles;    /**< The current MCParticle list (a TClonesArray of MCParticles). */
      TClonesArray *m_mcPartRels;     /**< The current relation between a MCParticle and a SimHit (a TClonesArray of relations). */
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
    inline void Generator<SIMHITS>::addFile(const std::string &component, const std::string &generator,
                                            const std::string &filename, const std::string &simHitCollection,
                                            const std::string &simHitRelation)
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
    inline void Generator<SIMHITS>::fillDataStore()
    {
      if (!m_enabled) return;

      updateNumberROF();
      if (m_numberROFs == 0) return;
      if (m_index >= m_numberROFs) m_index = 0;

      //Get ROF from chain
      m_files->GetEntry(m_index);

      //Loop over SimHit content of ROF and add it to the DataStore SimHit collection.
      StoreArray<SIMHITS> simHitArray(m_simHitCollection);
      int colIndex = simHitArray->GetLast() + 1;
      int simHitIndexOffset = colIndex; //The index offset in the global SimHit DataStoreArray
      int nSimHits = m_readoutFrame->GetEntries();
      std::vector<int> oldNewIndexList;
      for (int iSimHit = 0; iSimHit < nSimHits; ++iSimHit) {
        new(simHitArray->AddrAt(colIndex)) SIMHITS(*(dynamic_cast<SIMHITS*>(m_readoutFrame->At(iSimHit))));
        colIndex++;
      }

      //Add the information which background is mixed in to a new collection 'BkgInfo'.
      //Loop over the MCParticle content of the ROF and add it to the MCParticle DataStore collection.
      //Add the relation between a MCParticle and a SimHit into the appropriate collection.
      //Add the relation between a MCParticle and the background information into the 'BkgInfoRels' collection.

      //Access the standard DataStore collections
      StoreArray<MCParticle> mcPartCollection;
      RelationArray simHitToMCPartCollection(mcPartCollection, simHitArray, m_simHitRelation);

      //Create the background specific DataStore collections
      StoreArray<BackgroundInfo> bkgInfoCollection("BkgInfo");
      RelationArray bkgInfoRelCollection(mcPartCollection, bkgInfoCollection, "BkgInfoRels");

      //1) Add the background info to the new collection 'BkgInfo'.
      new(bkgInfoCollection->AddrAt(bkgInfoCollection->GetLast() + 1)) BackgroundInfo(m_component, m_generator);

      //2) Add the MCParticles and a relation to the background info
      colIndex = mcPartCollection->GetLast() + 1;
      int mcPartIndexOffset = colIndex; //The index offset in the global MCParticle DataStoreArray
      int nPart = m_mcParticles->GetEntries();
      for (int iPart = 0; iPart < nPart; ++iPart) {
        new(mcPartCollection->AddrAt(colIndex)) MCParticle(*(dynamic_cast<MCParticle*>(m_mcParticles->At(iPart))));
        bkgInfoRelCollection.add(colIndex, bkgInfoCollection->GetLast());
        colIndex++;
      }

      //3) Add the SimHit to MCParticle relations
      int nRel = m_mcPartRels->GetEntries();
      for (int iRel = 0; iRel < nRel; ++iRel) {
        RelationElement *relation = dynamic_cast<RelationElement*>(m_mcPartRels->At(iRel));
        simHitToMCPartCollection.add(relation->getFromIndex() + mcPartIndexOffset, relation->getToIndex() + simHitIndexOffset);
      }

      m_index++;
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
