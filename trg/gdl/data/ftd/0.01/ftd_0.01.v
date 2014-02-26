//
// ftd.v
//    0.01 : 2014/01/08 : YI : new ftd
//

module FTD (ftdout, ftdinf, ftdind, ftdinb, gclk2); 

//...IO connectors...
input  [29:0] ftdinf;  // from connector iof
input  [31:0] ftdind;  //                iod
input  [31:0] ftdinb;  //                iob

output [159:0] ftdout; // for FTD out monitoring

//...Clock...      
input gclk2;

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

always @(posedge gclk2) begin

    n_t3_full[0]   <= ftdinb[0]; //  0 : lowest bit :  from iob[0]
    n_t3_full[1]   <= ftdinb[1]; //  1
    n_t3_full[2]   <= ftdinb[2]; //  2
    n_t3_short[0]  <= ftdinb[3]; //  3
    n_t3_short[1]  <= ftdinb[4]; //  4
    n_t3_short[2]  <= ftdinb[5]; //  5
    n_t2_full[0]   <= ftdinb[6]; //  6
    n_t2_full[1]   <= ftdinb[7]; //  7
    n_t2_full[2]   <= ftdinb[8]; //  8
    n_t2_short[0]  <= ftdinb[9]; //  9
    n_t2_short[1]  <= ftdinb[10]; // 10
    n_t2_short[2]  <= ftdinb[11]; // 11
    cdc_bb         <= ftdinb[12]; // 12
    cdc_open45       <= ftdinb[13]; // 13
    cdc_timing     <= ftdinb[14]; // 14
    e_high         <= ftdinb[15]; // 15

    e_low          <= ftdinb[16]; // 16
    e_lum          <= ftdinb[17]; // 17
    ecl_bha        <= ftdinb[18]; // 18
    bha_type[0]    <= ftdinb[19]; // 19
    bha_type[1]    <= ftdinb[20]; // 20
    bha_type[2]    <= ftdinb[21]; // 21
    bha_type[3]    <= ftdinb[22]; // 22
    bha_type[4]    <= ftdinb[23]; // 23
    bha_type[5]    <= ftdinb[24]; // 24
    bha_type[6]    <= ftdinb[25]; // 25
    bha_type[7]    <= ftdinb[26]; // 26
    bha_type[8]    <= ftdinb[27]; // 27
    bha_type[9]    <= ftdinb[28]; // 28
    bha_type[10]   <= ftdinb[29]; // 29
    n_clus[0]      <= ftdinb[30]; // 30
    n_clus[1]      <= ftdinb[31]; // 31

    n_clus[2]      <= ftdind[0]; // 32 : from iod[0]
    n_clus[3]      <= ftdind[1]; // 33
    bg_ecl_veto    <= ftdind[2]; // 34
    ecl_timing_fwd <= ftdind[3]; // 35
    ecl_timing_bwd <= ftdind[4]; // 36
    ecl_timing_brl <= ftdind[5]; // 37
    ecl_timing     <= ftdind[6]; // 38
    n_top[0]       <= ftdind[7]; // 39
    n_top[1]       <= ftdind[8]; // 40
    n_top[2]       <= ftdind[9]; // 41
    top_bb         <= ftdind[10]; // 42
    n_klm[0]       <= ftdind[11]; // 43
    n_klm[1]       <= ftdind[12]; // 44
    n_klm[2]       <= ftdind[13]; // 45
    revo           <= ftdind[14]; // 46
    her_kick       <= ftdind[15]; // 47

    ler_kick       <= ftdind[16]; // 48
    bha_delay      <= ftdind[17]; // 49
    pseude_rand    <= ftdind[18]; // 50
    veto           <= ftdind[19]; // 51
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
bhabha_trk = ecl_bha & (n_t2>1) & cdc_bb;

gg         = e_high & (n_t2==0) & (n_t3==0);

mu_pair    = (n_klm>0) & (n_t2>1);

revolution = revo;
random     = pseude_rand;
bg         = bha_delay;

             };

endmodule
