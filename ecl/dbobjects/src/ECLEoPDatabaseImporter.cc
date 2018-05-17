/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marco Milesi                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/dbobjects/ECLEoPDatabaseImporter.h>
#include <ecl/dbobjects/ECLEoPParams.h>

#include <framework/database/IntervalOfValidity.h>
#include <framework/database/Database.h>

#include <TClonesArray.h>

using namespace Belle2;
using namespace ECL;

ECLEoPDatabaseImporter::ECLEoPDatabaseImporter(std::string inputFileName, std::string name) :
  m_inputFileName(inputFileName),
  m_name(name)
{ }

void ECLEoPDatabaseImporter::importPDF()
{
  TClonesArray eopPDF("Belle2::ECLEoPPDF");

  new(eopPDF[0]) ECLEoPParams(m_inputFileName);

  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments
  Database::Instance().storeData(m_name, eopPDF[0], iov);
}
