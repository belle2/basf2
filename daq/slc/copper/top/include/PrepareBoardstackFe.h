#ifndef __PREPAREBOARDSTACKFE__
#define __PREPAREBOARDSTACKFE__

namespace Belle2 {
  class HSLB;
  class DBObject;
  class RCCallback;
}

#include <fstream>
#include <stdint.h>

namespace PrepBoardstackFE {

  extern std::map<const std::string, int> registerValueMap;/*map to store default values for registers*/

  /**
   *@brief load standard values to registerValueMap.
   */
  void LoadDefaultRegisterValues();
  /**
   *@brief update the default register map values from the database
   *@param database object
   */
  void UpdateRegisterFromDatabase(const Belle2::DBObject& databaseObject);

  /**
   *@brief initialize callbacks
   *@param hslb connected to board stack
   *@param feature extraction mode
   */
  void InitCallbacks(Belle2::HSLB& hslb, Belle2::RCCallback& callback);

  /**
   *@brief Prepare board stack feature extraction (called from standalone c program)
   *@param hslb connected to board stack
   *@param feature extraction mode
   *@param threshold for constant fraction discrimination
   */
  void PrepareBoardStack(Belle2::HSLB& hslb, const int mode = -1,
                         const unsigned cfdthreshold = 100);
  /**
   *@brief Prepare board stack feature extraction
   *@param hslb connected to board stack
   *@param run control call back
   *@param feature extraction mode
   *@param threshold for constant fraction discrimination
   */
  int PrepareBoardStack(Belle2::HSLB& hslb, Belle2::RCCallback& callback, const int mode = -1,
                        const unsigned cfdthreshold = 100);

  /**
   *@brief acquire board stack pedestals
   *@param hslb connected ton board stack
   *@return 0 if accquisition was succesfull
   */
  int TakePedestals(Belle2::HSLB& hslb);
  /**
   *@brief read pedestals from board stack and check quality
   *@param hslb connected to board stack
   *@returns 0 in case of good pedestal quality
   */
  int CheckPedestals(Belle2::HSLB& hslb);

  /**functions needed for pedestal check. taken from readhs.c and peddump.c*/
  /**
   *@brief open copper for read
   *@param slot id
   *@param access mode
   */
  int OpenCopper(const int& use_slot, const int& clef_1);
  /**
   *@brief read data from copper
   *@param copper handle
   *@param raw data array
   *@param offset in data array
   *@param maximum number of words read to rawData array per read
   *@return number of words read
   */
  int ReadCopper(const int& cprfd, uint32_t* const rawData, const unsigned& offset, const unsigned& maxReadPerEvent);
  /**
   *@brief convert raw data, save in pedestal data array and save to pedestal file
   *@param rawData array
   *@param pedestal data array
   *@param path to pedestal file
   *@return 0 if conversion was successfull
   */
  int DumpPedestal(uint32_t const* const rawData, unsigned short* const pedestalData, std::fstream& pedestalFile);
  /**
   *@brief check pedestal quality
   *@param pedestal data array
   *@return 0 if pedestal quality is good
   */
  int CalculatePedestalProperties(unsigned short const* const pedestalData);
}

#endif
