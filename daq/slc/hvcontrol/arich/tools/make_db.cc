#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main(int argc, char** argv)
{

  //  char fname[32] = "gb_chmap.csv";
  ifstream ifs_gb("../data/gb_map_pratical.csv");
  string str_gb;

  if (!ifs_gb) {
    printf("no file (gb)\n");
    printf("Did you add one line? CHECK!");
  }

  //  getline(ifs,str);
  //  cout << str << endl;

  int gb_crate[2500];
  int gb_slot[2500];
  int gb_channel[2500];
  int gb_sector[2500];
  int gb_cable[2500];
  int gb_pin[2500];
  int gb_modid[2500];
  char gb_type[2500][10];
  int gb_volt[2500];

  printf("read file\n");

  //    cout<<getline(ifs,str)<<endl;
  //    cout<<ifs<<endl;
  //    cout<<str<<endl;


  for (int l = 0; l < 2500; l++) {
    gb_crate[l] = -1;
    gb_slot[l] = -1;
    gb_channel[l] = -1;
    gb_sector[l] = -1;
    gb_cable[l] = -1;
    gb_pin[l] = -1;
    gb_modid[l] = 0;
    gb_volt[l] = 0;
  }

  int i_gb = 0;
  getline(ifs_gb, str_gb);
  while (getline(ifs_gb, str_gb)) {
    //    printf("read file ....\n");
    sscanf(str_gb.data(), "%d,%d,%d,%d,%d,%d,%d,%[^,],%d",
           &gb_crate[i_gb], &gb_slot[i_gb], &gb_channel[i_gb], &gb_sector[i_gb], &gb_cable[i_gb], &gb_pin[i_gb], &gb_modid[i_gb],
           gb_type[i_gb], &gb_volt[i_gb]);
    //    printf("crate(%d),slot(%d),channel(%d) : connector(%d),pin(%d),id(%d),type(%s),modid(%d)\n",crate[i],slot[i],channel[i],connector[i],pin[i],id[i],type[i],modid[i]);
    i_gb++;
  }




  ofstream ofs("../data/arich-input_yone.conf");
  //  ofstream ofs("arich-input_yone.conf");


  ofs << "nodename                    : ARICH_HV" << endl
      << "config                      : test:kek:peak:000" << endl << endl;


  ofs << endl
      << "crate[1].name                  : arichps1" << endl
      << "crate[1].host                  : 192.168.0.101" << endl
      << "crate[1].port                  : 22" << endl
      << "crate[1].usech                 : 1" << endl << endl;
  /*
  ofs << endl
      << "crate[2].name                  : arichps2" << endl
      << "crate[2].host                  : 192.168.0.102" << endl
      << "crate[2].port                  : 22" << endl
      << "crate[2].usech                 : 1" << endl << endl;

  ofs << endl
      << "crate[3].name                  : arichps3" << endl
      << "crate[3].host                  : 192.168.0.103" << endl
      << "crate[3].port                  : 22" << endl
      << "crate[3].usech                 : 1" << endl << endl;
  */

  string type = "guard";
  string spare = "spare";
  string none = "none";

  int k_gb = 0;

  for (int cr = 1; cr < 4; cr++) {
    k_gb = 0;
    ofs << endl
        << "crate[" << cr << "].name                  : arichps" << cr << endl
        << "crate[" << cr << "].host                  : 192.168.0.10" << cr << endl
        << "crate[" << cr << "].port                  : 22" << endl
        << "crate[" << cr << "].usech                 : 1" << endl << endl;

    for (int sl = 0; sl < 16; sl++) {
      for (int ch = 0; ch < 48; ch++) {

        for (int num = 0; num < 2500; num++) {
          if ((gb_crate[num] == cr) && (gb_slot[num] == sl) && (gb_channel[num] == ch)) {

            ofs << "crate[" << gb_crate[num]  << "].channel[" << k_gb << "].slot       : " << gb_slot[num] << endl
                << "crate[" << gb_crate[num]  << "].channel[" << k_gb << "].channel    : " << gb_channel[num] << endl
                << "crate[" << gb_crate[num]  << "].channel[" << k_gb << "].type       : " << gb_type[num] << endl;

            /*
              if(gb_modid[num]==0){
              ofs <<"none"<<endl;
              }else{
              ofs << gb_type[num] << endl;
              }
            */

            ofs << "crate[" << gb_crate[num]  << "].channel[" << k_gb << "].modid      : " << gb_modid[num] << endl
                << "crate[" << gb_crate[num]  << "].channel[" << k_gb << "].sector     : " << gb_sector[num] << endl
                << "crate[" << gb_crate[num]  << "].channel[" << k_gb << "].turnon     : true" << endl
                << "crate[" << gb_crate[num]  << "].channel[" << k_gb << "].mask       : no" << endl
                << "crate[" << gb_crate[num]  << "].channel[" << k_gb << "].rampup     : 10.000000" << endl
                << "crate[" << gb_crate[num]  << "].channel[" << k_gb << "].rampdown   : 10.000000" << endl
                << "crate[" << gb_crate[num]  << "].channel[" << k_gb << "].vdemand    : " << gb_volt[num] << ".000000" << endl;
            /*
            if(gb_type[num] == type){
            ofs<<"175.000000"<<endl;
            //      ofs<<gb_volt[num]<<".000000"<<endl;
            }else if(gb_volt[num]>0){
            ofs<<"175.000000"<< endl;
            //      ofs<<gb_volt[num]<<".000000"<< endl;
            }else{
            ofs<<gb_volt[num]<<".000000"<<endl;
            }
            */
            //    ofs << "crate[0].channel[" << k_gb << "].vlimit     : " << "180.000000"<< endl
            ofs << "crate[" << gb_crate[num]  << "].channel[" << k_gb << "].vlimit     : " << (gb_volt[num] > 0 ? gb_volt[num] + 5 : 0) <<
                ".000000" << endl
                << "crate[" << gb_crate[num]  << "].channel[" << k_gb << "].climit     : 30.000000" << endl
                << "crate[" << gb_crate[num]  << "].channel[" << k_gb << "].voffset    : 0.000000" << endl
                << "crate[" << gb_crate[num]  << "].channel[" << k_gb << "].vslope     : 1.000000" << endl
                << "crate[" << gb_crate[num]  << "].channel[" << k_gb << "].coffset    : 0.000000" << endl
                << "crate[" << gb_crate[num]  << "].channel[" << k_gb << "].cslope     : 1.000000" << endl;
            k_gb++;
          }
        }
      }

    }
  }

  /*
  for(int slot_tmp=14;slot_tmp<16;slot_tmp++){
    for(int channel_tmp=0;channel_tmp<48;channel_tmp++){
      ofs << "crate[0].channel[" << k_gb << "].slot       : " << slot_tmp<<endl
    << "crate[0].channel[" << k_gb << "].channel    : " << channel_tmp << endl
    << "crate[0].channel[" << k_gb << "].type       : none " << endl
    << "crate[0].channel[" << k_gb << "].modid      : -1 " << endl
    << "crate[0].channel[" << k_gb << "].turnon     : true" << endl
    << "crate[0].channel[" << k_gb << "].rampup     : 10.000000" << endl
    << "crate[0].channel[" << k_gb << "].rampdown   : 10.000000" << endl
    << "crate[0].channel[" << k_gb << "].vdemand    : 10.000000" << endl
    << "crate[0].channel[" << k_gb << "].vlimit     : 20.000000" << endl
    << "crate[0].channel[" << k_gb << "].climit     : 10.000000" << endl
    << "crate[0].channel[" << k_gb << "].voffset    : 0.000000" << endl
    << "crate[0].channel[" << k_gb << "].vslope     : 1.000000" << endl
    << "crate[0].channel[" << k_gb << "].coffset    : 0.000000" << endl
    << "crate[0].channel[" << k_gb << "].cslope     : 1.000000" << endl;
      k_gb++;
    }
  }
  */





  //for hv database


  ifstream ifs_hv("../data/hv_map_pratical.csv");
  string str_hv;

  if (!ifs_hv) {
    printf("no file (hv)\n");
  }

  int hv_crate[500];
  int hv_slot[500];
  int hv_channel[500];
  int hv_sector[500];
  int hv_cable[500];
  int hv_pin[500];
  int hv_modid[500];
  char hv_type[500][10];
  int hv_volt[500];

  for (int l = 0; l < 500; l++) {
    hv_crate[l] = -1;
    hv_slot[l] = -1;
    hv_channel[l] = -1;
    hv_sector[l] = -1;
    hv_cable[l] = -1;
    hv_pin[l] = -1;
    hv_modid[l] = 0;
    hv_volt[l] = 0;
  }



  int i_hv = 0;
  getline(ifs_hv, str_hv);
  while (getline(ifs_hv, str_hv)) {
    //    cout << str_hv << endl;
    //    printf("read file ....\n");
    sscanf(str_hv.data(), "%d,%d,%d,%d,%d,%d,%d,%[^,],%d",
           &hv_crate[i_hv], &hv_slot[i_hv], &hv_channel[i_hv], &hv_sector[i_hv], &hv_cable[i_hv], &hv_pin[i_hv], &hv_modid[i_hv],
           hv_type[i_hv], &hv_volt[i_hv]);
    //    sscanf(str_hv.data(),"%d,%d,%d,%d,%d,%d,%[^,],%d",&hv_crate[i_hv],&hv_slot[i_hv],&hv_channel[i_hv],&hv_connector[i_hv],&hv_pin[i_hv],&hv_id[i_hv],hv_type[i_hv],&hv_modid[i_hv]);
    //    printf("crate(%d),slot(%d),channel(%d) : connector(%d),pin(%d),id(%d),type(%s),modid(%d)\n",crate[i],slot[i],channel[i],connector[i],pin[i],id[i],type[i],modid[i]);
    i_hv++;
  }

  /*
  ofs << endl
      << "crate[4].name                  : arichps4" << endl
      << "crate[4].host                  : 192.168.0.104" << endl
      << "crate[4].port                  : 22" << endl
      << "crate[4].usech                 : 1" << endl << endl;

  ofs << endl
      << "crate[5].name                  : arichps5" << endl
      << "crate[5].host                  : 192.168.0.105" << endl
      << "crate[5].port                  : 22" << endl
      << "crate[5].usech                 : 1" << endl << endl;

  ofs << endl
      << "crate[6].name                  : arichps6" << endl
      << "crate[6].host                  : 192.168.0.106" << endl
      << "crate[6].port                  : 22" << endl
      << "crate[6].usech                 : 1" << endl << endl;

  ofs << endl
      << "crate[7].name                  : arichps7" << endl
      << "crate[7].host                  : 192.168.0.107" << endl
      << "crate[7].port                  : 22" << endl
      << "crate[7].usech                 : 1" << endl << endl;
  */

  int k_hv = 0;

  for (int cr = 4; cr < 8; cr++) {
    k_hv = 0;
    ofs << endl
        << "crate[" << cr << "].name                  : arichps" << cr << endl
        << "crate[" << cr << "].host                  : 192.168.0.10" << cr << endl
        << "crate[" << cr << "].port                  : 22" << endl
        << "crate[" << cr << "].usech                 : 1" << endl << endl;

    for (int sl = 0; sl < 16; sl = sl + 2) {
      for (int ch = 0; ch < 16; ch++) {

        for (int num = 0; num < 500; num++) {
          //    cout << "cr = " << cr << endl;
          //    cout << hv_crate[num] << "," << hv_slot[num]<<","<< hv_channel[num]<< endl;
          if ((hv_crate[num] == cr) && (hv_slot[num] == sl) && (hv_channel[num] == ch)) {
            ofs << "crate[" << hv_crate[num]  << "].channel[" << k_hv << "].slot       : " << hv_slot[num] << endl
                << "crate[" << hv_crate[num]  << "].channel[" << k_hv << "].channel    : " << hv_channel[num] << endl
                << "crate[" << hv_crate[num]  << "].channel[" << k_hv << "].type       : " << hv_type[num] << endl;
            /*
              if(hv_modid[num]<=70){
              ofs << hv_type[num] << endl;
              }else{
              ofs << "none" << endl;
              }
            */
            ofs << "crate[" << hv_crate[num]  << "].channel[" << k_hv << "].modid      : " << hv_modid[num] << endl
                << "crate[" << hv_crate[num]  << "].channel[" << k_hv << "].sector     : " << hv_sector[num] << endl
                << "crate[" << hv_crate[num]  << "].channel[" << k_hv << "].turnon     : true" << endl
                << "crate[" << hv_crate[num]  << "].channel[" << k_hv << "].mask       : no" << endl
                << "crate[" << hv_crate[num]  << "].channel[" << k_hv << "].rampup     : 100.000000" << endl
                << "crate[" << hv_crate[num]  << "].channel[" << k_hv << "].rampdown   : 100.000000" << endl
                << "crate[" << hv_crate[num]  << "].channel[" << k_hv << "].vdemand    : " << hv_volt[num] << ".000000" << endl
                << "crate[" << hv_crate[num]  << "].channel[" << k_hv << "].vlimit     : " << (hv_volt[num] > 0 ? 8500 : 0) << ".000000"  << endl
                << "crate[" << hv_crate[num]  << "].channel[" << k_hv << "].climit     : 10.000000" << endl
                << "crate[" << hv_crate[num]  << "].channel[" << k_hv << "].voffset    : 0.000000" << endl
                << "crate[" << hv_crate[num]  << "].channel[" << k_hv << "].vslope     : 1.000000" << endl
                << "crate[" << hv_crate[num]  << "].channel[" << k_hv << "].coffset    : 0.000000" << endl
                << "crate[" << hv_crate[num]  << "].channel[" << k_hv << "].cslope     : 1.000000" << endl;
            k_hv++;
          }
        }
      }

    }
  }
  /*
    for(int slot_tmp=14;slot_tmp<15;slot_tmp++){
      for(int channel_tmp=0;channel_tmp<16;channel_tmp++){
        ofs << "crate[1].channel[" << k_hv << "].slot       : " << slot_tmp<<endl
      << "crate[1].channel[" << k_hv << "].channel    : " << channel_tmp << endl
      << "crate[1].channel[" << k_hv << "].type       : none " << endl
      << "crate[1].channel[" << k_hv << "].modid      : -1 " << endl
      << "crate[1].channel[" << k_hv << "].turnon     : true" << endl
      << "crate[1].channel[" << k_hv << "].rampup     : 100.000000" << endl
      << "crate[1].channel[" << k_hv << "].rampdown   : 100.000000" << endl
      << "crate[1].channel[" << k_hv << "].vdemand    : 100.000000" << endl
      << "crate[1].channel[" << k_hv << "].vlimit     : 110.000000" << endl
      << "crate[1].channel[" << k_hv << "].climit     : 10.000000" << endl
      << "crate[1].channel[" << k_hv << "].voffset    : 0.000000" << endl
      << "crate[1].channel[" << k_hv << "].vslope     : 1.000000" << endl
      << "crate[1].channel[" << k_hv << "].coffset    : 0.000000" << endl
      << "crate[1].channel[" << k_hv << "].cslope     : 1.000000" << endl;
        k_hv++;
      }
    }
  */



  return 0;
}
