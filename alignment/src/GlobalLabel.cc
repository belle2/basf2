/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <alignment/GlobalLabel.h>

#include <iostream>

using namespace std;

using namespace Belle2;

std::set<unsigned short> GlobalLabel::m_components = {};

GlobalLabel::GlobalLabel(GlobalLabel::gidTYPE globalLabel) :
  gid(0), eid(0), pid(0), tid(0), tif(0)
{
  if (globalLabel > maxLabel)
    return;
  gid = globalLabel;
  tif = gid / tifOffset;
  pid = gid % eidOffest / pidOffset;

  if (!tif) {
    eid = gid % tifOffset / eidOffest;
  } else {
    // Time-dep label
    gidTYPE teid = gid % tidOffset / teidOffset;
    gidTYPE teidpid = makeTEIDPID(teid, pid);
    gidTYPE eidpid = 0;
    auto& dict = GlobalLabel::getDictionary();
    auto it = dict.find(teidpid);
    if (it != dict.end())
      eidpid = it->second;

    eid = eidpid / eidOffest;
    tid = gid % tifOffset / tidOffset;
  }
}

void GlobalLabel::registerTimeDependent(GlobalLabel::gidTYPE start,
                                        GlobalLabel::gidTYPE end)
{
  auto& dict = GlobalLabel::getDictionary();
  auto& ints = GlobalLabel::getTimeIntervals();
  tif = 1;
  tid = start;
  gidTYPE eidpid  = makeEIDPID(eid, pid);

  auto it = ints.find(eidpid);
  if (it == ints.end()) {
    // Not found, insert new record
    gidTYPE teidpid = makeTEIDPID(dict.size() + 1, pid);
    dict.insert(make_pair(teidpid, eidpid));
    ints.insert(make_pair(eidpid, TimeInterval(teidpid, start, end)));
  } else {
    // Found, add time interval
    it->second.set(start, end);
  }
}

void GlobalLabel::clearTimeDependentParamaters()
{
  GlobalLabel::getDictionary().clear();
  GlobalLabel::getTimeIntervals().clear();
}

GlobalLabel::gidTYPE GlobalLabel::setParameterId(GlobalLabel::gidTYPE paramId)
{
  if (!getUniqueId() or paramId > maxPID) {
    return label();
  }
  construct(getUniqueId(), getElementId(), paramId);
  return label();
}

void GlobalLabel::construct(GlobalLabel::gidTYPE elementId,
                            GlobalLabel::gidTYPE paramId)
{
  if (elementId > maxEID || paramId > maxPID)
    return;
  pid = paramId;
  eid = elementId;

  gidTYPE eidpid = makeEIDPID(eid, pid);
  gidTYPE teidpid = 0;
  auto& ints = GlobalLabel::getTimeIntervals();
  auto it = ints.find(eidpid);
  if (it != ints.end())
    teidpid = it->second.teidpid();

  if (teidpid)
    tif = 1;
  else
    tif = 0;

  if (!tif)
    gid = (tif * tifOffset + eid * eidOffest + pid * pidOffset);
  else {
    tid = it->second.get(GlobalLabel::getCurrentTimeIntervalRef());
    gid = (tif * tifOffset + tid * tidOffset + teidpid);
  }
  /*
  if (!teidpid) {
    // time indep.
    tif = 0;
    gid = (tif * tifOffset + eid * eidOffest + pid * pidOffset);
  }
  else {
    tid = it->second.get(GlobalLabel::getCurrentTimeIntervalRef());
    if (tid == 0) {
      // actually the first instance of time dep. parameter -> def with orginal time. indep. label
      //FIXME: code copied from above!
      tif = 0;
      gid = (tif * tifOffset + eid * eidOffest + pid * pidOffset);
    } else {
      tif = 1;
      gid = (tif * tifOffset + tid * tidOffset + teidpid);
    }
  }
  */
}

void GlobalLabel::dump(int level) const
{
  cout << "GlobalLabel: gid=" << gid << endl;
  cout << "             eid=" << eid << endl;
  cout << "             pid=" << pid << endl;
  cout << "             tid=" << tid << endl;
  cout << "             tif=" << tif << endl;
  if (level == 0)
    return;
  cout << "    Time-dependent map:" <<  endl;
  cout << "    [EIDPID : TEIDPID] (registered time intervals)" << endl;
  for (auto& it : getTimeIntervals()) {
    cout << "     " << it.first << " : " << it.second.teidpid() << endl;
    cout << "     ";
    if (level > 1) {
      for (unsigned int i = 0; i <= GlobalLabel::maxTID; i++) {
        cout << it.second.get(i) << " ";
        if ((i + 1) % 40 == 0)
          cout << endl << "     ";
      }
      cout << endl;
    }
    cout << endl;
  }
  cout << endl;
}

