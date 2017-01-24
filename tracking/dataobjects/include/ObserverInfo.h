/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Lueck                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <TVector3.h>

#include <vxd/dataobjects/VxdID.h>

#include <map>

namespace Belle2 {

  // helper struct to store the SpacePoint information as coding convention prohibits to use the SpacePoint class here
  struct SpacePointInfo {
    TVector3 position = {0., 0., 0.};
    TVector3 positionError = {1., 1., 1.};
    // normalized coordinates of spacepoint in local coordinates .first is u , .second is v -direction
    std::pair<double, double> normalizedLocal = {0., 0.};
    // .first if SP has a u cluster, .second if SP has a v-cluster assigned
    std::pair<bool, bool> clustersAssigned = {false, false};
    Belle2::VxdID vxdID = Belle2::VxdID();
    int sensorType = -1;
    double qualityIndicator = 0.5;
    bool isAssigned = false;
  };

  class ObserverInfo : public TObject {
  public:
    ObserverInfo();
    ~ObserverInfo();

    // TODO : protect the getters for returning values which are not in the map!?
    double getResult(std::string  key) { return  m_results[key]; };
    void setResult(std::string  key, double& val) { m_results[key] = val; };

    bool getWasAccepted(std::string  key) { return m_wasAccepted[key]; };
    void setWasAccepted(std::string  key, bool& val) { m_wasAccepted[key] = val; };

    bool getWasUsed(std::string key) { return m_wasUsed[key]; };
    void setWasUsed(std::string key, bool& val) { m_wasUsed[key] = val; };

    std::vector<SpacePointInfo> getHits() { return m_hits; };
    SpacePointInfo getHit(int i)
    {
      if (i < 0 || i > m_hits.size()) return SpacePointInfo();  // return a dummy object
      else return m_hits[i];
    };
    void setHits(std::vector<SpacePointInfo>& newHits) { m_hits = newHits; };

    int getMainMCParticleID() {return m_mainMCParticleID;};
    void setMainMCParticleID(int& id) { m_mainMCParticleID = id; };

    double getMainPurity() { return m_mainPurity; };
    void setMainPurity(double& val) { m_mainPurity = val; };
  private:
    /** stores the results calculated (->value) for a selectionVariableName (->Key). */
    // //   static std::map<std::string, std::unique_ptr<double>> s_results;
    std::map<std::string, double> m_results;


    /** stores if hits were accepted (->value) for a selectionVariableName (->Key). */
    // //   static std::map<std::string, std::unique_ptr<bool>> s_wasAccepted;
    std::map<std::string, bool> m_wasAccepted;


    /** stores if the filter was actually used this time (->value) for a selectionVariableName (->Key). */
    // //   static std::map<std::string, std::unique_ptr<bool>> s_wasUsed;
    std::map<std::string, bool> m_wasUsed;

    /** stores the hits used in the filter, should have the order as used in the filter from outer to inner! */
    std::vector<SpacePointInfo>  m_hits;

    /** dominating mcParticleID. */
    int m_mainMCParticleID;

    /** purity for the dominating particleID. */
    double m_mainPurity;

    //! Needed to make the ROOT object storable
    ClassDef(ObserverInfo, 1)
  };

}// end namespace Belle2
