/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef __EXTERNAL__
#include "trg/cdc/HandleRoot.h"
#else
#include "HandleRoot.h"
#endif
#include <utility>
#include <iostream>
#include <TBranchObject.h>
#include <tuple>

using std::cout;
using std::endl;
using std::vector;
using std::map;
using std::string;
using std::to_string;
using std::get;

namespace HandleRoot {

  void initializeEvent(
    std::map<std::string, TVectorD*>& eventMapTVectorD,
    std::map<std::string, TClonesArray*>& trackMapTVectorD
  )
  {
    for (map<string, TVectorD*>::iterator it = eventMapTVectorD.begin(); it != eventMapTVectorD.end(); ++it) {
      it->second->Clear();
    }
    for (map<string, TClonesArray*>::iterator it = trackMapTVectorD.begin(); it != trackMapTVectorD.end(); ++it) {
      it->second->Clear();
    }
  }

  void initializeEvent(
    std::map<std::string, TClonesArray*>& trackMapTVectorD
  )
  {
    map<string, TVectorD*> eventMapTVectorD;
    initializeEvent(eventMapTVectorD, trackMapTVectorD);
  }

  void initializeRoot(const std::string& prefix, TTree** runTree, TTree** eventTree,
                      std::map<std::string, TVectorD*>& runMapTVectorD,
                      std::map<std::string, TVectorD*>& eventMapTVectorD,
                      std::map<std::string, TClonesArray*>& trackMapTVectorD,
                      std::map<std::string, double>& constMapD, std::map<std::string, std::vector<double> >& constMapV,
                      std::map<std::string, double>& eventMapD, std::map<std::string, std::vector<double> >& eventMapV,
                      std::map<std::string, double>& trackMapD, std::map<std::string, std::vector<double> >& trackMapV
                     )
  {

    // Add Run tree
    (*runTree) = new TTree((prefix + "runTree").c_str(), "run");
    // Make storage and save for saving m_mConstD.
    for (map<string, double>::iterator it = constMapD.begin(); it != constMapD.end(); ++it) {
      runMapTVectorD[prefix + "Double" + (*it).first] = new TVectorD(1, &(*it).second);
    }
    // Make storage and save for saving m_mConstV
    for (map<string, vector<double> >::iterator it = constMapV.begin(); it != constMapV.end(); ++it) {
      unsigned t_vectorSize = it->second.size();
      runMapTVectorD[prefix + "Vector" + (*it).first] = new TVectorD(t_vectorSize, &((*it).second)[0]);
    }
    // Adding branch
    for (map<string, TVectorD*>::iterator it = runMapTVectorD.begin(); it != runMapTVectorD.end(); ++it) {
      (*runTree)->Branch(it->first.c_str(), &it->second, 32000, 0);
    }
    // Fill tree
    (*runTree)->Fill();


    // Make event tree
    (*eventTree) = new TTree((prefix + "eventTree").c_str(), "event");
    // Events
    // Make storage for saving m_mEventD
    for (map<string, double>::iterator it = eventMapD.begin(); it != eventMapD.end(); ++it) {
      eventMapTVectorD[prefix + "EventDouble" + (*it).first] = new TVectorD(1);
    }
    // Make storage for saving m_mEventV
    for (map<string, vector<double> >::iterator it = eventMapV.begin(); it != eventMapV.end(); ++it) {
      unsigned t_vectorSize = it->second.size();
      eventMapTVectorD[prefix + "EventVector" + (*it).first] = new TVectorD(t_vectorSize);
    }
    // Adding branch
    for (map<string, TVectorD*>::iterator it = eventMapTVectorD.begin(); it != eventMapTVectorD.end(); ++it) {
      (*eventTree)->Branch(it->first.c_str(), &it->second, 32000, 0);
    }
    // Tracks
    // Make storage for saving m_mDouble
    for (map<string, double>::iterator it = trackMapD.begin(); it != trackMapD.end(); ++it) {
      trackMapTVectorD[prefix + "TrackDouble" + (*it).first] = new TClonesArray("TVectorD");
    }
    // Make storage for saving m_mVector
    for (map<string, vector<double> >::iterator it = trackMapV.begin(); it != trackMapV.end(); ++it) {
      trackMapTVectorD[prefix + "TrackVector" + (*it).first] = new TClonesArray("TVectorD");
    }
    // Adding branch
    for (map<string, TClonesArray*>::iterator it = trackMapTVectorD.begin(); it != trackMapTVectorD.end(); ++it) {
      (*eventTree)->Branch(it->first.c_str(), &it->second, 32000, 0);
    }

  }

  void initializeRoot(const std::string& prefix, TTree** runTree, TTree** eventTree,
                      std::map<std::string, TVectorD*>& runMapTVectorD,
                      std::map<std::string, TClonesArray*>& trackMapTVectorD,
                      std::map<std::string, double>& constMapD, std::map<std::string, std::vector<double> >& constMapV,
                      std::map<std::string, double>& trackMapD, std::map<std::string, std::vector<double> >& trackMapV
                     )
  {
    map<string, TVectorD*> eventMapTVectorD;
    map<string, double> eventMapD;
    map<string, vector<double> > eventMapV;
    initializeRoot(prefix, runTree, eventTree,
                   runMapTVectorD, eventMapTVectorD, trackMapTVectorD,
                   constMapD, constMapV,
                   eventMapD, eventMapV,
                   trackMapD, trackMapV
                  );
  }

  void saveTrackValues(const std::string& prefix,
                       const std::map<std::string, TClonesArray*>& trackMapTVectorD,
                       std::map<std::string, double>& trackMapD, std::map<std::string, std::vector<double> >& trackMapV
                      )
  {
    // Save m_mDouble
    for (map<string, double >::iterator it = trackMapD.begin(); it != trackMapD.end(); ++it) {
      new((*trackMapTVectorD.at(prefix + "TrackDouble" + (*it).first))[trackMapD["iSave"]]) TVectorD(1, &(*it).second);
    }
    // Save m_mVector
    for (map<string, vector<double> >::iterator it = trackMapV.begin(); it != trackMapV.end(); ++it) {
      unsigned t_vectorSize = it->second.size();
      new((*trackMapTVectorD.at(prefix + "TrackVector" + (*it).first))[trackMapD["iSave"]]) TVectorD(t_vectorSize, &((*it).second)[0]);
    }

    trackMapD["iSave"]++;
  }

  void saveEventValues(const std::string& prefix,
                       std::map<std::string, TVectorD*>& eventMapTVectorD,
                       std::map<std::string, double>& eventMapD, std::map<std::string, std::vector<double> >& eventMapV
                      )
  {
    // Save m_mEventD
    for (map<string, double>::iterator it = eventMapD.begin(); it != eventMapD.end(); ++it) {
      eventMapTVectorD[prefix + "EventDouble" + (*it).first]->Use(1, &(*it).second);
    }
    // Save m_mEventV
    for (map<string, vector<double> >::iterator it = eventMapV.begin(); it != eventMapV.end(); ++it) {
      unsigned t_vectorSize = it->second.size();
      if (t_vectorSize != 0) eventMapTVectorD[prefix + "EventVector" + (*it).first]->Use(t_vectorSize, &((*it).second)[0]);
    }
  }

  void writeRoot(TFile* file)
  {
    if (file) {
      file->Write();
      file->Close();
    }
  }

  void terminateRoot(
    std::map<std::string, TVectorD*>& runMapTVectorD,
    std::map<std::string, TVectorD*>& eventMapTVectorD,
    std::map<std::string, TClonesArray*>& trackMapTVectorD
  )
  {
    for (map<string, TVectorD*>::iterator it = runMapTVectorD.begin(); it != runMapTVectorD.end(); ++it) {
      delete it->second;
    }
    for (map<string, TVectorD*>::iterator it = eventMapTVectorD.begin(); it != eventMapTVectorD.end(); ++it) {
      delete it->second;
    }
    for (map<string, TClonesArray*>::iterator it = trackMapTVectorD.begin(); it != trackMapTVectorD.end(); ++it) {
      delete it->second;
    }
  }

  void terminateRoot(
    std::map<std::string, TVectorD*>& runMapTVectorD,
    std::map<std::string, TClonesArray*>& trackMapTVectorD
  )
  {
    std::map<std::string, TVectorD*> eventMapTVectorD;
    terminateRoot(runMapTVectorD, eventMapTVectorD, trackMapTVectorD);
  }

  void initializeBranches(const std::string& prefix, TFile* file, TTree** runTree, TTree** eventTree,
                          std::map<std::string, TVectorD*>& runMapTVectorD,
                          std::map<std::string, TVectorD*>& eventMapTVectorD,
                          std::map<std::string, TClonesArray*>& trackMapTVectorD
                         )
  {
    // Get Trees
    (*runTree) = (TTree*)file->Get((prefix + "runTree").c_str());
    (*eventTree) = (TTree*)file->Get((prefix + "eventTree").c_str());
    // Set memory for constant information
    TObjArray* constBranchList = (*runTree)->GetListOfBranches();
    for (int iBranch = 0; iBranch < constBranchList->GetEntries(); iBranch++) {
      string t_branchName = constBranchList->At(iBranch)->GetName();
      runMapTVectorD[t_branchName] = new TVectorD();
      (*runTree)->SetBranchAddress(t_branchName.c_str(), &runMapTVectorD[t_branchName]);
    }
    // Set memory for event and track information
    TObjArray* trackBranchList = (*eventTree)->GetListOfBranches();
    for (int iBranch = 0; iBranch < trackBranchList->GetEntries(); iBranch++) {
      string t_branchName = trackBranchList->At(iBranch)->GetName();
      string t_className = ((TBranchObject*)trackBranchList->At(iBranch))->GetClassName();
      // Track
      if (t_className == "TClonesArray") {
        trackMapTVectorD[t_branchName] = new TClonesArray("TVectorD");
        (*eventTree)->SetBranchAddress(t_branchName.c_str(), &trackMapTVectorD[t_branchName]);
      } else if (t_className == "TVectorT<double>") {
        // Event
        eventMapTVectorD[t_branchName] = new TVectorD();
        (*eventTree)->SetBranchAddress(t_branchName.c_str(), &eventMapTVectorD[t_branchName]);
      } else {
        cout << "[Warning] HandleRoot::initializeBranches => Type of branch " << t_branchName << " is unkown." << endl;
      }
    }
  }

  void initializeBranches(const std::string& prefix, TFile* file, TTree** runTree, TTree** eventTree,
                          std::map<std::string, TVectorD*>& runMapTVectorD,
                          std::map<std::string, TClonesArray*>& trackMapTVectorD
                         )
  {
    map<string, TVectorD*> eventMapTVectorD;
    initializeBranches(prefix, file, runTree, eventTree,
                       runMapTVectorD, eventMapTVectorD, trackMapTVectorD
                      );
  }

  void getRunValues(std::string prefix,
                    std::map<std::string, TVectorD*>& runMapTVectorD,
                    std::map<std::string, double>& constMapD, std::map<std::string, std::vector<double> >& constMapV
                   )
  {
    // Get all constant information
    for (map<string, TVectorD*>::iterator it = runMapTVectorD.begin(); it != runMapTVectorD.end(); ++it) {
      string t_name = it->first.substr(prefix.size() + 6);
      string t_type = it->first.substr(prefix.size(), 6);
      if (t_type == "Double") {
        constMapD[t_name] = (*it->second)[0];
      } else if (t_type == "Vector") {
        unsigned nElements = it->second->GetNrows();
        constMapV[t_name] = vector<double> (nElements);
        for (unsigned i = 0; i < nElements; i++) {
          constMapV[t_name][i] = (*it->second)[i];
        }
      } else {
        cout << "[Error] HandleRoot::getRunValues => t_type: " << t_type << " is unknown." << endl;
      }
    }
  }

  void getEventValues(const std::string& prefix,
                      std::map<std::string, TVectorD*>& eventMapTVectorD,
                      std::map<std::string, double>& eventMapD, std::map<std::string, std::vector<double> >& eventMapV
                     )
  {
    // Get all event information
    for (map<string, TVectorD*>::iterator it = eventMapTVectorD.begin(); it != eventMapTVectorD.end(); ++it) {
      string t_name = it->first.substr((prefix + "Event").size() + 6);
      string t_type = it->first.substr((prefix + "Event").size(), 6);
      if (t_type == "Double") {
        eventMapD[t_name] = (*it->second)[0];
      } else if (t_type == "Vector") {
        unsigned nElements = it->second->GetNrows();
        eventMapV[t_name] = vector<double> (nElements);
        for (unsigned i = 0; i < nElements; i++) {
          eventMapV[t_name][i] = (*it->second)[i];
        }
      } else {
        cout << "[Error] HandleRoot::getEventValues => t_type: " << t_type << " is unknown." << endl;
      }
    }
  }

  void getTrackValues(const std::string& prefix, int iTrack,
                      std::map<std::string, TClonesArray*>& trackMapTVectorD,
                      std::map<std::string, double>& trackMapD, std::map<std::string, std::vector<double> >& trackMapV
                     )
  {
    for (map<string, TClonesArray*>::iterator it = trackMapTVectorD.begin(); it != trackMapTVectorD.end(); ++it) {
      string t_name = it->first.substr((prefix + "Track").size() + 6);
      string t_type = it->first.substr((prefix + "Track").size(), 6);
      if (t_type == "Double") {
        trackMapD[t_name] = (*(TVectorD*)it->second->At(iTrack))[0];
      } else if (t_type == "Vector") {
        unsigned nElements = ((TVectorD*)it->second->At(iTrack))->GetNrows();
        trackMapV[t_name] = vector<double> (nElements);
        for (unsigned i = 0; i < nElements; i++) {
          trackMapV[t_name][i] = (*(TVectorD*)it->second->At(iTrack))[i];
        }
      } else {
        cout << "[Error] HandleRoot::getTrackValues => t_type: " << t_type << " is unknown." << endl;
      }
    }
  }

  void convertSignalValuesToMaps(std::vector<std::tuple<std::string, double, int, double, double, int> > const& inValues,
                                 std::map<std::string, double>& trackMapD, std::map<std::string, std::vector<double> >& trackMapV)
  {
    for (unsigned iValue = 0; iValue < inValues.size(); iValue++) {
      string const& t_name = get<0>(inValues[iValue]);
      double const& t_value = get<1>(inValues[iValue]);
      //int const & t_bitWidth = get<2>(inValues[iValue]);
      //double const & t_min = get<3>(inValues[iValue]);
      //double const & t_max = get<4>(inValues[iValue]);
      //int const & t_clock = get<5>(inValues[iValue]);
      // Divide vectors and doubles by name.
      size_t t_find = t_name.find("_");
      // Vector
      if (t_find != string::npos) {
        string t_vectorName = t_name.substr(0, t_find);
        int t_vectorIndex = stoi(t_name.substr(t_find + 1, t_name.size()));
        // Create vector if not in map.
        if (!trackMapV.count(t_vectorName)) {
          trackMapV[t_vectorName] = vector<double> (1);
        }
        // Increase vector size if vector size is too small
        int nIncrease = t_vectorIndex + 1 - trackMapV[t_vectorName].size();
        if (nIncrease > 0) {
          for (int iIncrease = 0; iIncrease < nIncrease; iIncrease++) trackMapV[t_vectorName].push_back(0);
        }
        // Fill to map
        trackMapV[t_vectorName][t_vectorIndex] = t_value;
      } else {
        // Double
        trackMapD[t_name] = t_value;
      }
    }
  }

} // namespace HandleRoot
