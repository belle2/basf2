//
// ftd.v
//    0.01 : 2014/01/08 : YI : new ftd
//    0.02 : 2014/10/23 : YI : for UT3
//

module FTD (VERSION, CLK, SUBT, FTDOUT); 

//...Version
output  [9:0] VERSION;

//...Subtrigger input...
input  [71:0] SUBT;

//...FTD output...   
output [12:0] FTDOUT; // for FTD out monitoring

//...Clock...      
input CLK;

   assign VERSION = 10'd2;

//............
//   Inputs
//............

//...CDC...

reg      [2:0] n_t3_full;
reg     [2:0] n_t3_short;
reg      [2:0] n_t2_full;
reg     [2:0] n_t2_short;
reg               cdc_bb;
reg           cdc_open45;
reg 	      cdc_active;
reg     [2:0] cdc_timing;


//...ECL...

reg               e_high;
reg                e_low;
reg                e_lum;
reg              ecl_bha;
reg      [10:0] bha_type;
reg         [3:0] n_clus;
reg          bg_ecl_veto;
reg       ecl_timing_fwd;
reg       ecl_timing_bwd;
reg       ecl_timing_brl;
reg 	      ecl_active;
reg     [2:0] ecl_timing;

//...TOP...

reg          [2:0] n_top;
reg               top_bb;
reg 	      top_active;
reg     [3:0] top_timing;

//...KLM...

reg          [2:0] n_klm;

//..KEKB...

reg                 revo;
reg             her_kick;
reg             ler_kick;

//...GDL Internal...

reg            bha_delay;
reg          pseude_rand;
reg                 veto;
reg           [2:0] n_t3;
reg           [2:0] n_t2;

//.............
//   Outputs
//.............

reg                  zzx; // 0
reg                  ffs; // 1
reg                   zx; // 2
reg                   fs; // 3

reg                  hie; // 4
reg                   c4; // 5

reg               bhabha; // 6
reg           bhabha_trk; // 7

reg                   gg; // 8

reg              mu_pair; // 9

reg           revolution; // 10
reg               random; // 11
reg                   bg; // 12

//................
//   Assignment
//................

always @(posedge CLK) begin

    //................
    //   Inputs
    //................

    n_t3_full[0]   <= SUBT[0];
    n_t3_full[1]   <= SUBT[1];
    n_t3_full[2]   <= SUBT[2];
    n_t3_short[0]  <= SUBT[3];
    n_t3_short[1]  <= SUBT[4];
    n_t3_short[2]  <= SUBT[5];
    n_t2_full[0]   <= SUBT[6];
    n_t2_full[1]   <= SUBT[7];
    n_t2_full[2]   <= SUBT[8];
    n_t2_short[0]  <= SUBT[9];
    n_t2_short[1]  <= SUBT[10];
    n_t2_short[2]  <= SUBT[11];
    cdc_bb         <= SUBT[12];
    cdc_open45     <= SUBT[13];
    cdc_active     <= SUBT[14];
    cdc_timing[0]  <= SUBT[15];
    cdc_timing[1]  <= SUBT[16];
    cdc_timing[2]  <= SUBT[17];
    e_high         <= SUBT[18];

    e_low          <= SUBT[19];
    e_lum          <= SUBT[20];
    ecl_bha        <= SUBT[21];
    bha_type[0]    <= SUBT[22];
    bha_type[1]    <= SUBT[23];
    bha_type[2]    <= SUBT[24];
    bha_type[3]    <= SUBT[25];
    bha_type[4]    <= SUBT[26];
    bha_type[5]    <= SUBT[27];
    bha_type[6]    <= SUBT[28];
    bha_type[7]    <= SUBT[29];
    bha_type[8]    <= SUBT[30];
    bha_type[9]    <= SUBT[31];
    bha_type[10]   <= SUBT[32];
    n_clus[0]      <= SUBT[33];
    n_clus[1]      <= SUBT[34];

    n_clus[2]      <= SUBT[35];
    n_clus[3]      <= SUBT[36];
    bg_ecl_veto    <= SUBT[37];
    ecl_timing_fwd <= SUBT[38];
    ecl_timing_bwd <= SUBT[39];
    ecl_timing_brl <= SUBT[40];
    ecl_active     <= SUBT[41];
    ecl_timing[0]  <= SUBT[42];
    ecl_timing[1]  <= SUBT[43];
    ecl_timing[2]  <= SUBT[44];
    n_top[0]       <= SUBT[45];
    n_top[1]       <= SUBT[46];
    n_top[2]       <= SUBT[47];
    top_bb         <= SUBT[48];
    top_active     <= SUBT[49];
    top_timing[0]  <= SUBT[50];
    top_timing[1]  <= SUBT[51];
    top_timing[2]  <= SUBT[52];
    top_timing[3]  <= SUBT[53];

    n_klm[0]       <= SUBT[54];
    n_klm[1]       <= SUBT[55];
    n_klm[2]       <= SUBT[56];
   
    revo           <= SUBT[57];
    her_kick       <= SUBT[58];
    ler_kick       <= SUBT[59];
   
    bha_delay      <= SUBT[60];
    pseude_rand    <= SUBT[61];
    veto           <= SUBT[62];

    //...GDL internal... should be moved before FTD
    n_t3           <= n_t3_full + n_t3_short;
    n_t2           <= n_t2_full + n_t2_short;

    //................
    //   FTD logics
    //................

    zzx        = (n_t3_full>1) & (n_t3_short>0) & cdc_open45 & (!veto);  //  0
    ffs        = (n_t2_full>1) & (n_t2_short>0) & cdc_open45 & (!veto);  //  1
    zx         = (n_t3_full>0) & (n_t3_short>0) & cdc_open45 & (!ecl_bha) & (!veto);
	     		       		      		   	     //  2
    fs         = (n_t2_full>0) & (n_t2_short>0) & cdc_open45 & (!ecl_bha) & (!veto);
	     		       		      		   	     //  3

    hie        = e_high & (!ecl_bha) & (!veto);                           //  4
    c4         = (n_clus>3) & (!veto);                                   //  5
    bhabha     = ecl_bha & (!veto);                                       //  6
    bhabha_trk = ecl_bha & (n_t2>1) & cdc_bb & (!veto);

    gg         = e_high & (n_t2==0) & (n_t3==0) & (!veto);

    mu_pair    = (n_klm>0) & (n_t2>1) & (!veto);

    revolution = revo;
    random     = pseude_rand;
    bg         = bha_delay;

end; // always @ (posedge CLK)
   
assign FTDOUT = {bg,
                 random,
                 revolution,
                 mu_pair,
                 gg,
                 bhabha_trk,
                 bhabha,
                 c4,
                 hie,
                 fs,
                 zx,
                 ffs,
                 zzx};

endmodule
