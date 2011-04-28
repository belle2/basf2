/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Hiroyuki Nakayama                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/bfieldmap/BFieldComponentRadial.h>

#include <framework/core/Environment.h>
#include <framework/core/ModuleUtils.h>
#include <framework/logging/Logger.h>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/gzip.hpp>


using namespace std;
using namespace Belle2;
namespace io = boost::iostreams;


BFieldComponentRadial::BFieldComponentRadial() : m_mapFilename("")
{

}


BFieldComponentRadial::~BFieldComponentRadial()
{

}


void BFieldComponentRadial::initialize()
{
  if (m_mapFilename.empty()) {
    B2ERROR("The filename for the radial magnetic field component is empty !")
    return;
  }

  string fullPath = Environment::Instance().getDataSearchPath() + "/" + m_mapFilename;

  if (!ModuleUtils::fileNameExists(fullPath)) {
    B2ERROR("The radial magnetic field map file '" << m_mapFilename << "' could not be found !")
    return;
  }

  //Load the map file
  io::filtering_istream fieldMapFile;
  fieldMapFile.push(io::gzip_decompressor());
  fieldMapFile.push(io::file_source(fullPath));

  //Create the magnetic field map [r,z] and read the data from the file
  m_mapBuffer = new BFieldPoint*[m_mapSize[0]];
  for (int i = 0; i < m_mapSize[0]; ++i)
    m_mapBuffer[i] = new BFieldPoint[m_mapSize[1]];

  double r, z, Br, Bz;
  for (int i = 0; i < m_mapSize[0]; ++i) {
    for (int j = 0; j < m_mapSize[1]; j++) {
      fieldMapFile >> r >> z >> Br >> Bz;

      //Since the accelerator group defines zero as the Belle center, move the
      //magnetic field to the IP.
      z += m_mapOffset;

      //Store the values
      m_mapBuffer[i][j].r = Br;
      m_mapBuffer[i][j].z = Bz;
    }
  }
}


TVector3 BFieldComponentRadial::calculate(const TVector3& point) const
{
  return TVector3(0.0, 0.0, 0.0);
}


void BFieldComponentRadial::terminate()
{
  //De-Allocate memory to prevent memory leak
  for (int i = 0; i < m_mapSize[0]; ++i)
    delete [] m_mapBuffer[i];
  delete [] m_mapBuffer;
}
