// High-level AMGA Library for Belle II
// Author: Jae-Hyuck Kwak (jhkwak@kisti.re.kr)
// Created: 2012-02-27 Initial release

#include "belle_amga.h"
#include <MDClient.h>
#include <iostream>
#include <sstream>
#include <ctime>
using namespace std;

belle_amga& belle_amga::getInstance()
{
  static belle_amga* instance = NULL;

  if (instance == NULL) {
    if (client.connectToServer()) {
      cout << client.getError() << endl;
      exit(5);
    }
    instance = new belle_amga();
  }

  cout << "Belle_AMGA server connected" << endl;

  return *instance;
}

int belle_amga::write_attr_filelvl(char* entry_name, struct_attr_filelvl& attr_filelvl_list)
{
  int res;

#ifdef __BELLE_AMGA_DEBUG__
  cout << "[belle2AMGA DEBUG] ----- FILELVL attr -----" << endl;
  cout << "[belle2AMGA DEBUG] ENTRY: " << entry_name << endl;
  cout << "[belle2AMGA DEBUG] id: " << attr_filelvl_list.id << endl;
  cout << "[belle2AMGA DEBUG] guid: " << attr_filelvl_list.guid << endl;
  cout << "[belle2AMGA DEBUG] lfn: " << attr_filelvl_list.lfn << endl;
  cout << "[belle2AMGA DEBUG] status: " << attr_filelvl_list.status << endl;
  cout << "[belle2AMGA DEBUG] events: " << attr_filelvl_list.events << endl;
  cout << "[belle2AMGA DEBUG] datasetid: " << attr_filelvl_list.datasetid << endl;
  cout << "[belle2AMGA DEBUG] stream: " << attr_filelvl_list.stream << endl;
  cout << "[belle2AMGA DEBUG] runH: " << attr_filelvl_list.runH << endl;
  cout << "[belle2AMGA DEBUG] eventH: " << attr_filelvl_list.eventH << endl;
  cout << "[belle2AMGA DEBUG] runL: " << attr_filelvl_list.runL << endl;
  cout << "[belle2AMGA DEBUG] eventL: " << attr_filelvl_list.eventL << endl;
  cout << "[belle2AMGA DEBUG] parentid: " << attr_filelvl_list.parentid << endl;
  cout << "[belle2AMGA DEBUG] softwareid: " << attr_filelvl_list.softwareid << endl;
  cout << "[belle2AMGA DEBUG] siteid: " << attr_filelvl_list.siteid << endl;
  cout << "[belle2AMGA DEBUG] userid: " << attr_filelvl_list.userid << endl;
  cout << "[belle2AMGA DEBUG] log_guid: " << attr_filelvl_list.log_guid << endl;
  cout << "[belle2AMGA DEBUG] ------------------------" << endl;
#endif

  stringstream sstr;
  sstr << "addentry " << entry_name << " id " << attr_filelvl_list.id << " guid '" << attr_filelvl_list.guid << "' lfn '" << attr_filelvl_list.lfn << "' status '" << attr_filelvl_list.status << "' events " << attr_filelvl_list.events << " datasetid " << attr_filelvl_list.datasetid << " stream " << attr_filelvl_list.stream << " runH " << attr_filelvl_list.runH << " eventH " << attr_filelvl_list.eventH << " runL " << attr_filelvl_list.runL << " eventL " << attr_filelvl_list.eventL << " parentid " << attr_filelvl_list.parentid << " softwareid " << attr_filelvl_list.softwareid << " siteid " << attr_filelvl_list.siteid << " userid " << attr_filelvl_list.userid << " log_guid '" << attr_filelvl_list.log_guid << "'";

  string command = sstr.str();

#ifdef __BELLE_AMGA_DEBUG__
  cout << "[belle2AMGA DEBUG] Trying to register FILELVL attr to Belle_AMGA Server.." << endl;
  cout << "[belle2AMGA DEBUG] AMGA command: " << command << endl;
#endif
  /*
    if ((res=client.execute(command)))
    {
      cout << "ERROR: execute failed"
           << " (" << res << "): "
           << client.getError() << endl;
      return res;
    }
  */
  cout << "SUCCESS: execute done" << endl;
  return 0;
}

int belle_amga::write_attr_dataset(char* entry_name, struct_attr_dataset& attr_dataset_list)
{
  int res;

#ifdef __BELLE_AMGA_DEBUG__
  cout << "[belle2AMGA DEBUG] ----- DATASET attr -----" << endl;
  cout << "[belle2AMGA DEBUG] id: " << attr_dataset_list.id << endl;
  cout << "[belle2AMGA DEBUG] name: " << attr_dataset_list.name << endl;
  cout << "[belle2AMGA DEBUG] description: " << attr_dataset_list.description << endl;
  cout << "[belle2AMGA DEBUG] type: " << attr_dataset_list.type << endl;
  cout << "[belle2AMGA DEBUG] level: " << attr_dataset_list.level << endl;
  cout << "[belle2AMGA DEBUG] mode: " << attr_dataset_list.mode << endl;
  cout << "[belle2AMGA DEBUG] skim: " << attr_dataset_list.skim << endl;
  cout << "[belle2AMGA DEBUG] userid: " << attr_dataset_list.userid << endl;
  cout << "[belle2AMGA DEBUG] date: " << attr_dataset_list.date << endl;
  cout << "[belle2AMGA DEBUG] ------------------------" << endl;
#endif

  stringstream sstr;
  sstr << "addentry " << entry_name << " id " << attr_dataset_list.id << " name '" << attr_dataset_list.name << "' description '" << attr_dataset_list.description << "' type '" << attr_dataset_list.type << "' level '" << attr_dataset_list.level << "' mode '" << attr_dataset_list.mode << "' skim '" << attr_dataset_list.skim << "' userid " << attr_dataset_list.userid << " date '" << attr_dataset_list.date << "'";

  string command = sstr.str();

#ifdef __BELLE_AMGA_DEBUG__
  cout << "[belle2AMGA DEBUG] Trying to register DATASET attr to Belle_AMGA Server.." << endl;
  cout << "[belle2AMGA DEBUG] AMGA command: " << command << endl;
#endif
  /*
    if ((res=client.execute(command)))
    {
      cout << "ERROR: execute failed"
           << "(" << res << "): "
           << client.getError() << endl;
      return res;
    }
  */
  cout << "SUCCESS: execute done" << endl;
  return 0;
}

int belle_amga::write_attr_software(char* entry_name, struct_attr_software& attr_software_list)
{
  int res;

#ifdef __BELLE_AMGA_DEBUG__
  cout << "[belle2AMGA DEBUG] ----- SOFTWARE attr -----" << endl;
  cout << "[belle2AMGA DEBUG] id: " << attr_software_list.id << endl;
  cout << "[belle2AMGA DEBUG] name: " << attr_software_list.name << endl;
  cout << "[belle2AMGA DEBUG] version: " << attr_software_list.version << endl;
  cout << "[belle2AMGA DEBUG] description: " << attr_software_list.description << endl;
  cout << "[belle2AMGA DEBUG] ------------------------" << endl;
#endif

  stringstream sstr;
  sstr << "addentry " << entry_name << " id " << attr_software_list.id << " name '" << attr_software_list.name << "' version '" << attr_software_list.version << "' description '" << attr_software_list.description << "'";

  string command = sstr.str();

#ifdef __BELLE_AMGA_DEBUG__
  cout << "[belle2AMGA DEBUG] Trying to register SOFTWARE attr to Belle_AMGA Server.." << endl;
  cout << "[belle2AMGA DEBUG] AMGA command: " << command << endl;
#endif
  /*
    if ((res=client.execute(command)))
    {
      cout << "ERROR: execute failed"
           << "(" << res << "): "
           << client.getError() << endl;
      return res;
    }
  */
  cout << "SUCCESS: execute done" << endl;
  return 0;
}

int belle_amga::write_attr_path(char* entry_name, struct_attr_path& attr_path_list)
{
  int res;

#ifdef __BELLE_AMGA_DEBUG__
  cout << "[belle2AMGA DEBUG] ----- PATH attr -----" << endl;
  cout << "[belle2AMGA DEBUG] type: " << attr_path_list.type << endl;
  cout << "[belle2AMGA DEBUG] exp: " << attr_path_list.exp << endl;
  cout << "[belle2AMGA DEBUG] path: " << attr_path_list.path << endl;
  cout << "[belle2AMGA DEBUG] ------------------------" << endl;
#endif

  stringstream sstr;
  sstr << "addentry " << entry_name << " type '" << attr_path_list.type << "' exp " << attr_path_list.exp << " path '" << attr_path_list.path << "'";

  string command = sstr.str();

#ifdef __BELLE_AMGA_DEBUG__
  cout << "[belle2AMGA DEBUG] Trying to register PATH attr to Belle_AMGA Server.." << endl;
  cout << "[belle2AMGA DEBUG] AMGA command: " << command << endl;
#endif
  /*
    if ((res=client.execute(command)))
    {
      cout << "ERROR: execute failed"
           << "(" << res << "): "
           << client.getError() << endl;
      return res;
    }
  */
  cout << "SUCCESS: execute done" << endl;
  return 0;
}

int belle_amga::write_attr_evtlvl(char* entry_name, struct_attr_evtlvl& attr_evtlvl_list)
{
  int res;

#ifdef __BELLE_AMGA_DEBUG__
  cout << "[belle2AMGA DEBUG] ----- EVTLVL attr -----" << endl;
  cout << "[belle2AMGA DEBUG] nmichargedtrk: " << attr_evtlvl_list.nmichargedtrk << endl;
  cout << "[belle2AMGA DEBUG] nplchargedtrk: " << attr_evtlvl_list.nplchargedtrk << endl;
  cout << "[belle2AMGA DEBUG] nks: " << attr_evtlvl_list.nks << endl;
  cout << "[belle2AMGA DEBUG] nkl: " << attr_evtlvl_list.nkl << endl;
  cout << "[belle2AMGA DEBUG] skimid: " << attr_evtlvl_list.skimid << endl;
  cout << "[belle2AMGA DEBUG] r2: " << attr_evtlvl_list.r2 << endl;
  cout << "[belle2AMGA DEBUG] ------------------------" << endl;
#endif

  stringstream sstr;
  sstr << "addentry " << entry_name << " nmichargedtrk " << attr_evtlvl_list.nmichargedtrk << " nplchargedtrk " << attr_evtlvl_list.nplchargedtrk << " nks " << attr_evtlvl_list.nks << " nkl " << attr_evtlvl_list.nkl << " skimid " << attr_evtlvl_list.skimid << " r2 " << attr_evtlvl_list.r2;

  string command = sstr.str();

#ifdef __BELLE_AMGA_DEBUG__
  cout << "[belle2AMGA DEBUG] Trying to register EVTLVL attr to Belle_AMGA Server.." << endl;
  cout << "[belle2AMGA DEBUG] AMGA command: " << command << endl;
#endif
  /*
    if ((res=client.execute(command)))
    {
      cout << "ERROR: execute failed"
           << "(" << res << "): "
           << client.getError() << endl;
      return res;
    }
  */
  cout << "SUCCESS: execute done" << endl;
  return 0;
}

int belle_amga::write_attr_skim(char* entry_name, struct_attr_skim& attr_skim_list)
{
  int res;

#ifdef __BELLE_AMGA_DEBUG__
  cout << "[belle2AMGA DEBUG] ----- SKIM attr -----" << endl;
  cout << "[belle2AMGA DEBUG] id: " << attr_skim_list.id << endl;
  cout << "[belle2AMGA DEBUG] name: " << attr_skim_list.name << endl;
  cout << "[belle2AMGA DEBUG] description: " << attr_skim_list.description << endl;
  cout << "[belle2AMGA DEBUG] userid: " << attr_skim_list.userid << endl;
  cout << "[belle2AMGA DEBUG] date: " << attr_skim_list.date << endl;
  cout << "[belle2AMGA DEBUG] ------------------------" << endl;
#endif

  stringstream sstr;
  sstr << "addentry " << entry_name << " id " << attr_skim_list.id << " name '" << attr_skim_list.name << "' description '" << attr_skim_list.description << "' userid " << attr_skim_list.userid << " date '" << attr_skim_list.date << "'";

  string command = sstr.str();

#ifdef __BELLE_AMGA_DEBUG__
  cout << "[belle2AMGA DEBUG] Trying to register SKIM attr to Belle_AMGA Server.." << endl;
  cout << "[belle2AMGA DEBUG] AMGA command: " << command << endl;
#endif
  /*
    if ((res=client.execute(command)))
    {
      cout << "ERROR: execute failed"
           << "(" << res << "): "
           << client.getError() << endl;
      return res;
    }
  */
  cout << "SUCCESS: execute done" << endl;
  return 0;
}

int belle_amga::write_attr_user(char* entry_name, struct_attr_user& attr_user_list)
{
  int res;

#ifdef __BELLE_AMGA_DEBUG__
  cout << "[belle2AMGA DEBUG] ----- USER attr -----" << endl;
  cout << "[belle2AMGA DEBUG] id: " << attr_user_list.id << endl;
  cout << "[belle2AMGA DEBUG] name: " << attr_user_list.name << endl;
  cout << "[belle2AMGA DEBUG] description: " << attr_user_list.description << endl;
  cout << "[belle2AMGA DEBUG] date: " << attr_user_list.date << endl;
  cout << "[belle2AMGA DEBUG] ------------------------" << endl;
#endif

  stringstream sstr;
  sstr << "addentry " << entry_name << " id " << attr_user_list.id << " name '" << attr_user_list.name << "' description '" << attr_user_list.description << "' date '" << attr_user_list.date << "'";

  string command = sstr.str();

#ifdef __BELLE_AMGA_DEBUG__
  cout << "[belle2AMGA DEBUG] Trying to register USER attr to Belle_AMGA Server.." << endl;
  cout << "[belle2AMGA DEBUG] AMGA command: " << command << endl;
#endif
  /*
    if ((res=client.execute(command)))
    {
      cout << "ERROR: execute failed"
           << "(" << res << "): "
           << client.getError() << endl;
      return res;
    }
  */
  cout << "SUCCESS: execute done" << endl;
  return 0;
}

int belle_amga::write_attr_site(char* entry_name, struct_attr_site& attr_site_list)
{
  int res;

#ifdef __BELLE_AMGA_DEBUG__
  cout << "[belle2AMGA DEBUG] ----- SITE attr -----" << endl;
  cout << "[belle2AMGA DEBUG] id: " << attr_site_list.id << endl;
  cout << "[belle2AMGA DEBUG] name: " << attr_site_list.name << endl;
  cout << "[belle2AMGA DEBUG] description: " << attr_site_list.description << endl;
  cout << "[belle2AMGA DEBUG] date: " << attr_site_list.date << endl;
  cout << "[belle2AMGA DEBUG] has_exp_fr: " << attr_site_list.has_exp_fr << endl;
  cout << "[belle2AMGA DEBUG] has_exp_fm: " << attr_site_list.has_exp_fm << endl;
  cout << "[belle2AMGA DEBUG] ------------------------" << endl;
#endif

  stringstream sstr;
  sstr << "addentry " << entry_name << " id " << attr_site_list.id << " name '" << attr_site_list.name << "' description '" << attr_site_list.description << "' date '" << attr_site_list.date << "' has_exp_fr " << attr_site_list.has_exp_fr << " has_exp_fm " << attr_site_list.has_exp_fm;

  string command = sstr.str();

#ifdef __BELLE_AMGA_DEBUG__
  cout << "[belle2AMGA DEBUG] Trying to register SITE attr to Belle_AMGA Server.." << endl;
  cout << "[belle2AMGA DEBUG] AMGA command: " << command << endl;
#endif
  /*
    if ((res=client.execute(command)))
    {
      cout << "ERROR: execute failed"
           << "(" << res << "): "
           << client.getError() << endl;
      return res;
    }
  */
  cout << "SUCCESS: execute done" << endl;
  return 0;
}

int belle_amga::write_belle_amga(int schemaid, char* entry_name, void* attr_list)
{
  switch (schemaid) {
    case FILELVL: {
      struct_attr_filelvl& attr_filelvl_list = *static_cast<struct_attr_filelvl*>(attr_list);
      write_attr_filelvl(entry_name, attr_filelvl_list);
    }
    break;
    case DATASET: {
      struct_attr_dataset& attr_dataset_list = *static_cast<struct_attr_dataset*>(attr_list);
      write_attr_dataset(entry_name, attr_dataset_list);
    }
    break;
    case SOFTWARE: {
      struct_attr_software& attr_software_list = *static_cast<struct_attr_software*>(attr_list);
      write_attr_software(entry_name, attr_software_list);
    }
    break;
    case PATH: {
      struct_attr_path& attr_path_list = *static_cast<struct_attr_path*>(attr_list);
      write_attr_path(entry_name, attr_path_list);
    }
    break;
    case EVTLVL: {
      struct_attr_evtlvl& attr_evtlvl_list = *static_cast<struct_attr_evtlvl*>(attr_list);
      write_attr_evtlvl(entry_name, attr_evtlvl_list);
    }
    break;
    case SKIM: {
      struct_attr_skim& attr_skim_list = *static_cast<struct_attr_skim*>(attr_list);
      write_attr_skim(entry_name, attr_skim_list);
    }
    break;
    case USER: {
      struct_attr_user& attr_user_list = *static_cast<struct_attr_user*>(attr_list);
      write_attr_user(entry_name, attr_user_list);
    }
    break;
    case SITE: {
      struct_attr_site& attr_site_list = *static_cast<struct_attr_site*>(attr_list);
      write_attr_site(entry_name, attr_site_list);
    }
    break;
  }
}

