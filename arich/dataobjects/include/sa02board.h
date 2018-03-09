#ifndef SA02BOARD_H
#define SA02BOARD_H

#define DEFAULT_IP_ADDRESS "192.168.10.16"
#define DEFAULT_PORT  24
#define DEFAULT_RBCP_PORT  4660

enum { N_SA0X = 4, NCH_SA0X = 36, NCH_BOARD = 144 };

//enum { NB_HEADER = 34, NB_DATA = 178 }; // number of bytes (not bits) <---origin
enum { NB_HEADER = 34, Merger_DATA = 12, FE_DATA = 166 }; // number of bytes (not bits) <---koba

/* threshold voltage setting */
enum { PTM1_WR_ADDR = 0xb1 }; // VTH1 is ch1 of AD5232

#define VTH_MIN -1.2700
#define VTH_STEP 0.0024  // [V] for SA02-Lju ver.1.1;
//#define VTH_MIN -1.045
//#define VTH_STEP 0.00831  // [V] for SA03testboard ver.1.1;



#endif
