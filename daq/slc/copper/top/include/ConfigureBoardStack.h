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

namespace ConfigBoardstack {

  struct ThresholdData {

    /**
     *@brief struct holding values from the trigger threshold configuration file
     */
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

  extern bool useExternalLogFile;/**use external file for logging*/
  extern std::fstream externalLogFile;/**log file instance*/

  extern std::map<std::string, int> registerValueMap;/*map to store default values for registers*/
  /**
   *@brief load standard values to registerValueMap, string names should be mapped to database
   *entries of the form top[X].carrier[Y].asic[Z].valuename
   *@param hslb connected to board stack
   */
  void LoadDefaultRegisterValues(Belle2::HSLB& hslb);
  /**
   *@brief update the default register map values from the database
   *@param database object
   */
  void UpdateRegisterFromDatabase(const Belle2::DBObject& databaseObject);

  void PrintRegisterValueMap();

  /**
   *@brief configure board stack
   *@param hslb connected to board stack
   */
  void ConfigureBoardStack(Belle2::HSLB& hslb);
  /**
   *@brief configure board stack
   *@param hslb connected to board stack
   *@param run control callback
   */
  void ConfigureBoardStack(Belle2::HSLB& hslb, Belle2::RCCallback& callback);
  /**
   *@brief configure carrier
   *@param hslb connected to board stack
   *@param run control callback
   *@param carrier ID
   */
  void ConfigureCarrier(Belle2::HSLB& hslb, Belle2::RCCallback& callback, const unsigned carrier);
  /**
   *@brief configure board stack
   *@param hslb connected to board stack
   *@param run control callback
   *@param carrier ID
   *@param asic ID
   */
  void ConfigureAsic(Belle2::HSLB& hslb, Belle2::RCCallback& callback, const unsigned carrier,
                     const unsigned asic);


  /**
   *@brief Verify the asic delay lock loop has locked
   *@param hslb connected to board stack
   *@param carrier ID
   *@param asic ID
   *@return returns 0 in case dll is locket
   */
  int VerifyDLLAsic(Belle2::HSLB& hslb, const unsigned carrier, const unsigned asic);
  /**
   *@brief Verify the asic delay lock loop has locked and set callback values
   *@param hslb connected to board stack
   *@param run control call back
   *@param carrier ID
   *@param asic ID
   */
  void VerifyDLLAsic(Belle2::HSLB& hslb, Belle2::RCCallback& callback, const unsigned carrier,
                     const unsigned asic);
  /**
   *@brief start asic delay lock loop
   *@param hslb connected to board stack
   *@param carrier ID
   *@param asic ID
   */
  int StartDLLAsic(Belle2::HSLB& hslb, const unsigned carrier, const unsigned asic);
  /**
   *@brief start asic delay lock loop
   *@param hslb connected to board stack
   *@param carrier ID
   *@param asic ID
   *@param value for vadjn feedback
   */
  int StartDLLAsic(Belle2::HSLB& hslb, const unsigned carrier, const unsigned asic, const unsigned vadjn);

  /**
   *@brief Enable amplifiers and chip biases
   *@param hslb connected to board stack
   *@param carrier ID
   *@param mask
   */
  int JuiceCarrier(Belle2::HSLB& hslb, const unsigned carrier, const unsigned mask);
  /**
   *@brief Enable amplifiers
   *@param hslb connected to board stack
   *@param carrier ID
   *@param mask
   */
  int JuiceCarrierNoASIC(Belle2::HSLB& hslb, const unsigned carrier, const unsigned mask);
  /**
   *@brief Enable chip biases
   *@param hslb connected to board stack
   *@param carrier ID
   *@param asic ID
   *@param enable VBias and VBias2
   */
  int JuiceASIC(Belle2::HSLB& hslb, const unsigned carrier, const unsigned asic, bool enable);
  /**
   *compute various bit masks for carrier juicing
   */
  int MapCA(const unsigned carrier, const unsigned asic);
  int MapMC(const unsigned mask, const unsigned carrier);
  int MapC(const unsigned carrier);

  /**
   *@brief compute look up value for threshold data map
   */
  int LookupValue(const int carrier, const int asic, const int channel);
  /**
   *@brief compute look up value for sstout fead back value
   */
  int LookupValue(const int carrier, const int asic);
  /**
   *@brief read trigger threshold data from file
   *@param path to threshold data file
   *@return map containing threshold data
   */
  std::map<int, ThresholdData> ReadThreshold(const std::string& thresholdFilePath);
  /**
   *@brief read sstout fead back values from file
   *@param path to fead back value file
   *@return map containing fead back data
   */
  std::map<int, int> ReadFeadbackValue(const std::string& fbFilePath);

  /**
   *@brief initialize the board stack callbacks
   *@param hslb connected to board stacks
   *@param run control callback
   */
  void InitBoardStackCallbacks(Belle2::HSLB& hslb, Belle2::RCCallback& callback);
  /**
   *@brief initialize the board stack callbacks
   *@param hslb connected to board stacks
   *@param run control callback
   *@param carrier ID
   */
  void InitCarrierCallbacks(Belle2::HSLB& hslb, Belle2::RCCallback& callback, const unsigned carrier);
  /**
   *@brief initialize the board stack callbacks
   *@param hslb connected to board stacks
   *@param run control callback
   *@param carrier ID
   *@param asic ID
   */
  void InitASICCallbacks(Belle2::HSLB& hslb, Belle2::RCCallback& callback, const unsigned carrier,
                         const unsigned asic);
}

#endif
