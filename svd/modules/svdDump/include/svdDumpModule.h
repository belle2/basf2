#ifndef SVD_DUMP_MODULE_H
#define SVD_DUMP_MODULE_H
//+
// File : svdClsHistoManagerModule.h
// Description : A module to create histogram ROOT file
//             : for SVD clusterizer study
//
// Author : Katsuro Nakamura, KEK
// Date : 15 - May - 2014
//-

#include <svd/online/SVDOnlineToOfflineMap.h>
#include <framework/core/Module.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <rawdata/dataobjects/RawSVD.h>

#include <string>
#include <ctime>
#include <vector>
#include <fstream>

namespace Belle2 {

  /*! Class definition of svdClsHistoManager module */
  class svdDumpModule : public Module {
  public:
    //! Constructor and Destructor
    svdDumpModule();
    virtual ~svdDumpModule();

    //! module functions
    virtual void initialize();
    virtual void beginRun();
    virtual void endRun();
    virtual void event();
    virtual void terminate();

  private:
    StoreArray<RawSVD> m_rawSVD; /**< Array for RawSVD */

    unsigned long  m_event;

    int m_nFtbHeader;
    int m_nFtbTrailer;

    std::string    m_outputFileName; /**< Name of output file. */
    std::ofstream* m_outputFile;     /**< output file. */

    std::string    m_svdRawName;

  };
} // Namaspace Belle2

#endif /* SVD_DUMP_MODULE_H */
