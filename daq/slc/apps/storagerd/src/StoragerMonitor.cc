#include "daq/slc/apps/storagerd/StoragerMonitor.h"

#include "daq/slc/apps/storagerd/storage_info_all.h"

#include <daq/slc/nsm/NSMData.h>
#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/Time.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include "daq/storage/storage_info.h"

#include <unistd.h>
#include <cstring>
#include <iostream>
#include <cstdio>
#include <cstdlib>

using namespace Belle2;

void StoragerMonitor::run()
{
}

