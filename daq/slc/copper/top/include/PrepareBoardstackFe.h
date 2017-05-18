#ifndef __PREPAREBOARDSTACKFE__
#define __PREPAREBOARDSTACKFE__

namespace Belle2 {
  class HSLB;
  class DBObject;
  class RCCallback;
}

#include <fstream>

namespace PrepBoardstackFE {

  extern std::map<const std::string, int> registerValueMap;

  void LoadDefaultRegisterValues();
  void UpdateRegisterFromDatabase(Belle2::HSLB& hslb, const Belle2::DBObject& databaseObject);

  void InitCallbacks(Belle2::HSLB& hslb, Belle2::RCCallback& callback);

  void PrepareBoardStack(Belle2::HSLB& hslb, const int mode = -1,
                         const unsigned cfdthreshold = 100);
  int PrepareBoardStack(Belle2::HSLB& hslb, Belle2::RCCallback& callback, const int mode = -1,
                        const unsigned cfdthreshold = 100);

  int TakePedestals(Belle2::HSLB& hslb);
  int CheckPedestals(Belle2::HSLB& hslb);

  /**functions needed for pedestal check. taken from readhs.c and peddump.c*/
  int OpenCopper(const int& use_slot, const int& clef_1);
  int ReadCopper(const int& cprfd, uint32_t* const rawData, const unsigned& offset, const unsigned& maxReadPerEvent);
  int DumpPedestal(uint32_t const* const rawData, unsigned short* const pedestalData, std::fstream& pedestalFile);
  int CalculatePedestalProperties(unsigned short const* const pedestalData);
}

#endif
