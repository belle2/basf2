/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef SVD_DUMP_MODULE_H
#define SVD_DUMP_MODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <rawdata/dataobjects/RawSVD.h>

#include <string>
#include <fstream>

namespace Belle2 {

  /*! Class definition of svdClsHistoManager module */
  class svdDumpModule : public Module {
  public:
    //! Constructor and Destructor
    svdDumpModule();
    virtual ~svdDumpModule() {};

    //! module functions
    virtual void initialize() override; /**< requires RawSVDs adn create output file*/
    virtual void beginRun() override; /**< print begin run*/
    virtual void endRun() override; /**< print end run*/
    virtual void event() override; /**< dump RawSVDs*/
    virtual void terminate() override; /**< write output file*/

  private:
    StoreArray<RawSVD> m_rawSVD; /**< Array for RawSVD */

    unsigned long  m_event; /**<event number*/

    int m_nFtbHeader; /**<FTB header*/
    int m_nFtbTrailer; /**<FTB trailer*/

    std::string    m_outputFileName; /**< Name of output file. */
    std::ofstream* m_outputFile = nullptr;     /**< output file. */

    std::string    m_svdRawName; /**< raw name*/

  };
} // Namaspace Belle2

#endif /* SVD_DUMP_MODULE_H */
