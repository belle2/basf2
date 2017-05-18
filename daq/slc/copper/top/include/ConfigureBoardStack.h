#ifndef __CONFIGUREBOARDSTACK__
#define __CONFIGUREBOARDSTACK__

#include <map>
#include <string>
#include <fstream>

namespace Belle2 {
  class HSLB;
  class DBObject;
  class RCCallback;
}

//TODO: 1. using the database to read configuration values
//      2. usage of callbacks for logging vs. an log file
//      3. general code clean up and testing

namespace ConfigBoardstack {

  struct ThresholdData {

    ThresholdData()
    {
      mean = 0;
      width = 0;
      threesigma = 0;
    }

    ThresholdData(float fmean, float fwidth, float fthreesigma)
    {
      mean = fmean;
      width = fwidth;
      threesigma = fthreesigma;
    }

    ThresholdData(const ThresholdData& other)
    {
      mean = other.mean;
      width = other.width;
      threesigma = other.threesigma;
    }

    float mean;
    float width;
    float threesigma;
  };

  extern bool useExternalLogFile;
  extern std::fstream externalLogFile;

  extern std::map<std::string, int> registerValueMap;/*map to store default values for registers*/
  /**
   *@brief load standard values to registerValueMap, string names should be mapped to database
   *entries of the form top[X].carrier[Y].asic[Z].valuename
   */
  void LoadDefaultRegisterValues(Belle2::HSLB& hslb);
  /**
   *@brief update the default register map values from the database
   */
  void UpdateRegisterFromDatabase(const Belle2::DBObject& databaseObject);

  void PrintRegisterValueMap();

  /**
   *function for configuration (read database/default config from register map)
   */
  void ConfigureBoardStack(Belle2::HSLB& hslb);
  void ConfigureBoardStack(Belle2::HSLB& hslb, Belle2::RCCallback& callback);
  void ConfigureCarrier(Belle2::HSLB& hslb, Belle2::RCCallback& callback, const unsigned carrier);
  void ConfigureAsic(Belle2::HSLB& hslb, Belle2::RCCallback& callback, const unsigned carrier,
                     const unsigned asic);


  /**
   *ASIC configuration
   */
  int VerifyDLLAsic(Belle2::HSLB& hslb, const unsigned carrier, const unsigned asic);
  int StartDLLAsic(Belle2::HSLB& hslb, const unsigned carrier, const unsigned asic);
  int StartDLLAsic(Belle2::HSLB& hslb, const unsigned carrier, const unsigned asic, const unsigned vadjn);
  //wrapper functions with call back for logging
  void VerifyDLLAsic(Belle2::HSLB& hslb, Belle2::RCCallback& callback, const unsigned carrier,
                     const unsigned asic);

  /**
   *Power to ASICs
   */
  int JuiceCarrier(Belle2::HSLB& hslb, const unsigned carrier, const unsigned mask);
  int JuiceCarrierNoASIC(Belle2::HSLB& hslb, const unsigned carrier, const unsigned mask);
  int JuiceASIC(Belle2::HSLB& hslb, const unsigned carrier, const unsigned asic, bool enable);
  int MapCA(const unsigned carrier, const unsigned asic);
  int MapMC(const unsigned mask, const unsigned carrier);
  int MapC(const unsigned carrier);

  /**
   *Reading values from files
   */
  int LookupValue(const int carrier, const int asic, const int channel);
  int LookupValue(const int carrier, const int asic);
  std::map<int, ThresholdData> ReadThreshold(const std::string& thresholdFilePath);
  std::map<int, int> ReadFeadbackValue(const std::string& fbFilePath);

  /*NSM Callbacks (too inform about configuration status ?!)*/
  void InitBoardStackCallbacks(Belle2::HSLB& hslb, Belle2::RCCallback& callback);
  void InitCarrierCallbacks(Belle2::HSLB& hslb, Belle2::RCCallback& callback, const unsigned carrier);
  void InitASICCallbacks(Belle2::HSLB& hslb, Belle2::RCCallback& callback, const unsigned carrier,
                         const unsigned asic);
}

#endif
