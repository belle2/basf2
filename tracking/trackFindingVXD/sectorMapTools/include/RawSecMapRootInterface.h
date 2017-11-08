/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at)     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/sectorMapTools/FilterValueDataSet.h>

#include <framework/logging/Logger.h>

#include <framework/pcore/RootMergeable.h>
#include <framework/datastore/StoreObjPtr.h>

#include <framework/pcore/ProcHandler.h>

#include <TFile.h>
#include <TList.h>
#include <TTree.h>
#include <string>
// #include <map>
// #include <limits>       // std::numeric_limits


namespace Belle2 {

  /** To be used as an interface to root-stuff. */
  class RawSecMapRootInterface {
  protected:

    /** a pointer to the file where the Tree shall be stored. */
    TFile* m_file;

    /** name of the StoreObjPtr. */
    std::string m_name;

    /** interface to the TTree storing two-hit-variables. */
    StoreObjPtr<RootMergeable<TTree>> m_tree2Hit;

    /** Mask for storing dataSets to be piped into 2hit-tree. */
    FilterValueDataSet<SecIDPair> m_data2Hit;

    /** interface to the TTree storing three-hit-variables. */
    StoreObjPtr<RootMergeable<TTree>> m_tree3Hit;

    /** Mask for storing dataSets to be piped into 3hit-tree. */
    FilterValueDataSet<SecIDTriplet> m_data3Hit;


  public:


    /** Constructor - prepares ttree. Without calling the initializer-functions this Object is still not working! */
    RawSecMapRootInterface(std::string mapName,  std::string tag) :
//       m_file(file),
      m_name(mapName),
      m_tree2Hit((m_name + std::string("2Hit")), DataStore::c_Persistent),
      m_data2Hit( {}),
                m_tree3Hit((m_name + std::string("3Hit")), DataStore::c_Persistent),
                m_data3Hit({})

    {
      // the "create" option results in the file not being opened if already existing
      std::string fileName = mapName + "_" + tag +  ".root";
      m_file = new TFile((fileName).c_str(), "CREATE");
      if (!m_file->IsOpen()) {
        B2FATAL("File was not opened! File name: " << fileName << " (maybe it already esists!?)");
      }
      m_file->cd();
    }

    /** destructor deleting the rootFile. */
    ~RawSecMapRootInterface() { /*delete m_file;*/ }


    /** That class shall not be copied. */
//     RawSecMapRootInterface(const RawSecMapRootInterface& rawSecMapInterFace)  = delete;


    /** initialize the RawSecMapRootInterface for two-hit-combinations (to be called in Module::initialize(). */
    void initialize2Hit(std::vector<std::string> filterNames)
    {
      B2DEBUG(1, "RawSecMapRootInterface::initialize2Hit: start - got " << filterNames.size() << " filters");
      B2DEBUG(1, "and root file got size of: " << m_file->GetSize());
      m_file->cd();

      // preparing StoreObjPtr:
      bool registered = m_tree2Hit.registerInDataStore((m_name + std::string("2Hit")));
      bool constructed = m_tree2Hit.construct((m_name + std::string("2Hit")).c_str(), "Raw data of two-hit-combinations for a sectorMap");
      B2DEBUG(1, "RawSecMapRootInterface::initialize2Hit: isRegistered/isConstructed: " << registered << "/" << constructed);

      // preparing data-mask for 2-hit-combinations:
      m_data2Hit = FilterValueDataSet<SecIDPair>(filterNames);

      m_tree2Hit->get().Branch("expNo", &(m_data2Hit.expNo));
      m_tree2Hit->get().Branch("runNo", &(m_data2Hit.runNo));
      m_tree2Hit->get().Branch("evtNo", &(m_data2Hit.evtNo));
      m_tree2Hit->get().Branch("trackNo", &(m_data2Hit.trackNo));
      m_tree2Hit->get().Branch("pdg", &(m_data2Hit.pdg));
      m_tree2Hit->get().Branch("outerSecID", &(m_data2Hit.secIDs.outer));
      m_tree2Hit->get().Branch("innerSecID", &(m_data2Hit.secIDs.inner));

      B2DEBUG(1, "RawSecMapRootInterface::initialize2Hit: adding " << filterNames.size() << " filters as branches to ttree ");
      for (auto& name : filterNames) {
        double* valuePtr = m_data2Hit.getValuePtr(name);
        if (valuePtr != nullptr) {
          B2DEBUG(5, "RawSecMapRootInterface::initialize2Hit: adding now branch with name " << name);
          m_tree2Hit->get().Branch(name.c_str(), valuePtr);
        } else {
          B2ERROR("RawSecMapRootInterface::initialize2Hit: filterName " << name <<
                  " not known! this is unintended behavior - skipping filter");
        }

      }
      B2DEBUG(1, "RawSecMapRootInterface::initialize2Hit: nBranches/nEntries: " << m_tree2Hit->get().GetNbranches() << "/" <<
              m_tree2Hit->get().GetEntries());
    }



    /** initialize the RawSecMapRootInterface for three-hit-combinations (to be called in Module::initialize(). */
    void initialize3Hit(std::vector<std::string> filterNames)
    {
      B2DEBUG(1, "RawSecMapRootInterface::initialize3Hit: start");
      m_file->cd();

      // preparing StoreObjPtr:
      bool registered = m_tree3Hit.registerInDataStore((m_name + std::string("3Hit")));
      bool constructed = m_tree3Hit.construct((m_name + std::string("3Hit")).c_str(),
                                              "Raw data of three-hit-combinations for a sectorMap");
      B2DEBUG(1, "RawSecMapRootInterface::initialize3Hit: isRegistered/isConstructed: " << registered << "/" << constructed);

      // preparing data-mask for 2-hit-combinations:
      m_data3Hit = FilterValueDataSet<SecIDTriplet>(filterNames);

      m_tree3Hit->get().Branch("expNo", &(m_data3Hit.expNo));
      m_tree3Hit->get().Branch("runNo", &(m_data3Hit.runNo));
      m_tree3Hit->get().Branch("evtNo", &(m_data3Hit.evtNo));
      m_tree3Hit->get().Branch("trackNo", &(m_data3Hit.trackNo));
      m_tree3Hit->get().Branch("pdg", &(m_data3Hit.pdg));
      m_tree3Hit->get().Branch("outerSecID", &(m_data3Hit.secIDs.outer));
      m_tree3Hit->get().Branch("centerSecID", &(m_data3Hit.secIDs.center));
      m_tree3Hit->get().Branch("innerSecID", &(m_data3Hit.secIDs.inner));

      B2DEBUG(1, "RawSecMapRootInterface::initialize3Hit: adding " << filterNames.size() << " filters as branches to ttree ");
      for (auto& name : filterNames) {
        double* valuePtr = m_data3Hit.getValuePtr(name);
        if (valuePtr != nullptr) {
          B2DEBUG(5, "RawSecMapRootInterface::initialize3Hit: adding now branch with name " << name);
          m_tree3Hit->get().Branch(name.c_str(), valuePtr);
        } else {
          B2ERROR("RawSecMapRootInterface::initialize3Hit: filterName " << name <<
                  " not known! this is unintended behavior - skipping filter");
        }

      }
      B2DEBUG(1, "RawSecMapRootInterface::initialize3Hit: nBranches/nEntries: " << m_tree3Hit->get().GetNbranches() << "/" <<
              m_tree3Hit->get().GetEntries());
    }

    /** returns a reference to the 2-hit-dataset so one can set the relevant values. */
    FilterValueDataSet<SecIDPair>& get2HitDataSet() { return m_data2Hit; }


    /** returns a reference to the 3-hit-dataset so one can set the relevant values. */
    FilterValueDataSet<SecIDTriplet>& get3HitDataSet() { return m_data3Hit; }



    /** fill two-hit-combinations in tree, triggers an Error if values not set yet.
     * (data is reset during each fill2Hit-call).*/
    void fill2Hit()
    {
      if (!m_data2Hit.isValid()) {
        B2ERROR("RawSecMapRootInterface::fill2Hit: attempt to fill invalid data in the tree! -> unintended behavior, data will not be filled.");
        m_data2Hit.reset();
        return;
      }
      m_tree2Hit->get().Fill();
      m_data2Hit.reset();
    }


    /** fill three-hit-combinations in tree, triggers an Error if values not set yet.
    * (data is reset during each fill3Hit-call).*/
    void fill3Hit()
    {
      if (!m_data3Hit.isValid()) {
        B2ERROR("RawSecMapRootInterface::fill3Hit: attempt to fill invalid data in the tree! -> unintended behavior, data will not be filled.");
        m_data3Hit.reset();
        return;
      }
      m_tree3Hit->get().Fill();
      m_data3Hit.reset();
    }


    /** write all trees to file at end of processing. */
    void write()
    {
      B2DEBUG(1, "RawSecMapRootInterface::write: start");
      m_file->cd();

      if (!ProcHandler::parallelProcessingUsed() or ProcHandler::isOutputProcess()) {
        //use TFile you created in initialize()
        m_tree2Hit->write(m_file);
        m_tree3Hit->write(m_file);

      }
    }

  };
}

