/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anselm Vossen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef BLKMUnpackerModule_H
#define BLKMUnpackerModule_H

#include <framework/core/Module.h>

#include <framework/datastore/StoreArray.h>

#include <bklm/dataobjects/BKLMDigit.h>

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <map>
#include <string>

using namespace std;

namespace Belle2 {

  //could define a BKLM namespace, but other BKLM modules don't do it either...

  class BKLMUnpackerModule : public Module {
  public:

    BKLMUnpackerModule();
    virtual ~BKLMUnpackerModule();

    virtual void initialize();
    virtual void beginRun();
    virtual void event();

    virtual void endRun();
    virtual void terminate();

    struct BKLMLData {
      unsigned int whatever : 32;
    };

  private:
    //to be used to map electronics address to module id
    int electCooToInt(int copper, int finesse, int lane);
    void intToElectCoo(int id, int& copper, int& finesse, int& lane);
    map<int, int> electIdToModuleId;

    void loadMap();
    std::string m_mapFileName;
    int getModuleId(int copperId, int finesseNum, int lane, int channel, int axis);


  };



}
#endif
