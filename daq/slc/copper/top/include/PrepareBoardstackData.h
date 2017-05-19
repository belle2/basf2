#ifndef __PREPAREBOARDSTACKDATA__
#define __PREPAREBOARDSTACKDATA__

#include <map>
#include <string>

namespace Belle2 {
  class HSLB;
  class DBObject;
  class RCCallback;
}

namespace PrepBoardstackData {

  extern std::map<std::string, int> registerValueMap;

  void LoadDefaultRegisterValues();
  void UpdateRegisterFromDatabase(Belle2::HSLB& hslb, const Belle2::DBObject& databaseObject);

  void PrepareBoardStack(Belle2::HSLB& hslb);
  void PrepareBoardStack(Belle2::HSLB& hslb, Belle2::RCCallback& callback);
  void PrepareCarrier(Belle2::HSLB& hslb, Belle2::RCCallback& callback, const unsigned carrier);
  void PrepareAsic(Belle2::HSLB& hslb, Belle2::RCCallback& callback, const unsigned carrier,
                   const unsigned asic);

  void SetLookback(Belle2::HSLB& hslb, const int lookback);
  void SetLookback(Belle2::HSLB& hslb, const unsigned carrier, const unsigned asic,
                   const int lookback);

  void SetFEMode(Belle2::HSLB& hslb, const int femode);

}
#endif
