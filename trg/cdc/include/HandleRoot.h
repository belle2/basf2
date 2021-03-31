#ifndef HANDLEROOT_H
#define HANDLEROOT_H

#include <TTree.h>
#include <TFile.h>
#include <TClonesArray.h>
#include <TVectorD.h>
#include <map>
#include <string>

namespace HandleRoot {
  // Write functions.
  void initializeEvent(
    std::map<std::string, TVectorD*>& eventMapTVectorD,
    std::map<std::string, TClonesArray*>& trackMapTVectorD
  );
  void initializeEvent(
    std::map<std::string, TClonesArray*>& trackMapTVectorD
  );
  void initializeRoot(const std::string& prefix, TTree** runTree, TTree** eventTree,
                      std::map<std::string, TVectorD*>& runMapTVectorD,
                      std::map<std::string, TVectorD*>& eventMapTVectorD,
                      std::map<std::string, TClonesArray*>& trackMapTVectorD,
                      std::map<std::string, double>& constMapD, std::map<std::string, std::vector<double> >& constMapV,
                      std::map<std::string, double>& eventMapD, std::map<std::string, std::vector<double> >& eventMapV,
                      std::map<std::string, double>& trackMapD, std::map<std::string, std::vector<double> >& trackMapV
                     );
  void initializeRoot(const std::string& prefix, TTree** runTree, TTree** eventTree,
                      std::map<std::string, TVectorD*>& runMapTVectorD,
                      std::map<std::string, TClonesArray*>& trackMapTVectorD,
                      std::map<std::string, double>& constMapD, std::map<std::string, std::vector<double> >& constMapV,
                      std::map<std::string, double>& trackMapD, std::map<std::string, std::vector<double> >& trackMapV
                     );
  // Requries trackMapD to have key "iSave".
  void saveTrackValues(const std::string& prefix,
                       const std::map<std::string, TClonesArray*>& trackMapTVectorD,
                       std::map<std::string, double>& trackMapD, std::map<std::string, std::vector<double> >& trackMapV
                      );
  void saveEventValues(const std::string& prefix,
                       std::map<std::string, TVectorD*>& eventMapTVectorD,
                       std::map<std::string, double>& eventMapD, std::map<std::string, std::vector<double> >& eventMapV
                      );
  void writeRoot(TFile* file);

  // Read functions.
  void initializeBranches(const std::string& prefix, TFile* file, TTree** runTree, TTree** eventTree,
                          std::map<std::string, TVectorD*>& runMapTVectorD,
                          std::map<std::string, TVectorD*>& eventMapTVectorD,
                          std::map<std::string, TClonesArray*>& trackMapTVectorD
                         );
  void initializeBranches(const std::string& prefix, TFile* file, TTree** runTree, TTree** eventTree,
                          std::map<std::string, TVectorD*>& runMapTVectorD,
                          std::map<std::string, TClonesArray*>& trackMapTVectorD
                         );
  void getRunValues(std::string prefix,
                    std::map<std::string, TVectorD*>& runMapTVectorD,
                    std::map<std::string, double>& constMapD, std::map<std::string, std::vector<double> >& constMapV
                   );
  void getEventValues(const std::string& prefix,
                      std::map<std::string, TVectorD*>& eventMapTVectorD,
                      std::map<std::string, double>& eventMapD, std::map<std::string, std::vector<double> >& eventMapV
                     );
  void getTrackValues(const std::string& prefix, int iTrack,
                      std::map<std::string, TClonesArray*>& trackMapTVectorD,
                      std::map<std::string, double>& trackMapD, std::map<std::string, std::vector<double> >& trackMapV
                     );

  // Common functions
  void terminateRoot(
    std::map<std::string, TVectorD*>& runMapTVectorD,
    std::map<std::string, TVectorD*>& eventMapTVectorD,
    std::map<std::string, TClonesArray*>& trackMapTVectorD
  );
  void terminateRoot(
    std::map<std::string, TVectorD*>& runMapTVectorD,
    std::map<std::string, TClonesArray*>& trackMapTVectorD
  );

  // Utility functions
  void convertSignalValuesToMaps(std::vector<std::tuple<std::string, double, int, double, double, int> > const& inValues,
                                 std::map<std::string, double>& trackMapD, std::map<std::string, std::vector<double> >& trackMapV
                                );

};

#endif
