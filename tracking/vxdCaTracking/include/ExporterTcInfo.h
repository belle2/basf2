/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>
#include <algorithm>
#include <string>
#include <TVector3.h>


namespace Belle2 {

  /** Bundles information for a single track candidate to be stored by EventInfo
   * (needed for HitExporter, which is needed by NonRootDataExportModule) */
  class ExporterTcInfo {
  public:
    /** Standard constructor. */
    ExporterTcInfo(int pdg, int motherID):
      m_pdgCode(pdg),
      m_motherID(motherID) {}


    /** Destructor. */
    ~ExporterTcInfo() {}


    /** adds already formatted hit information.
     * The .first value of the pair is the radius of the hit, so the vector can be sorted using it */
    void addHit(std::pair<double, std::string> hit) { m_hits.push_back(hit); }


    /** adds IDs of hits for unique identification */
    void addHitIDs(const std::vector<int>& ids) { m_hitIDs = ids; }


    /** returns number of Hits attached to this tc */
    int getNHits() { return m_hits.size(); }


    /** returns the hits added one by one using addHit */
    std::vector<std::pair<double, std::string>>* getHits() { return &m_hits; }


    /** returns hitIDs of the hits attached to this TC (in no particular order!) */
    std::vector<int>* getHitIDs() { return &m_hitIDs; }


    /** Adds Info about the tc itself.
     * It is already formatted but lacks of the last information needed,
     * the index of the first McPoint */
    void addInfo(const std::string& info) { m_tcInfo = info; }


    /** returns info about tc in formatted without having the last piece of the Information */
    std::string getInfo() { return m_tcInfo; }


    /** returns pdgCode of tc in the tc-file */
    int getPdgCode() { return m_pdgCode; }


    /** returns index of the mother particle in the simulation */
    int getMotherID() { return m_motherID; }


  protected:
    /// pdgCode of tc in the tc-file. TODO: implement a check which proves that suggested and real fileIndex are the same!
    int m_pdgCode;

    /// Index of the mother particle in the simulation.
    int m_motherID;

    /// Contains already formatted hit information.
    /// The .first value of the pair is the radius of the hit, so the vector can be sorted using it.
    std::vector<std::pair<double, std::string>> m_hits;

    /// Stores the hitIDs of the hits attached to this TC, can be treated independently of hits themselves.
    /// It's only important to know, which tc had which hits.
    std::vector<int> m_hitIDs;

    /// contains Info about the tc itself.
    /// It is already formatted but lacks of the last information needed, the index of the first McPoint.
    std::string m_tcInfo;
  };
}

