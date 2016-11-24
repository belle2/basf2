#ifndef _topreg_h
#define _topreg_h

////////////////////
// SCROD registers
////////////////////
#define SCROD_UNDEFINED            0xffffff

// Scrod(%d):AxiVersion(0):*      (register, scrod)
#define SCROD_AxiVersion_FpgaVersion   0x200
#define SCROD_AxiVersion_ScratchPad  0x201
#define SCROD_AxiVersion_DeviceDnaHigh   0x202
#define SCROD_AxiVersion_DeviceDnaLow  0x203
#define SCROD_AxiVersion_FdSerialHigh  0x204
#define SCROD_AxiVersion_FdSerialLow   0x205
#define SCROD_AxiVersion_MasterReset   0x206
#define SCROD_AxiVersion_FpgaReload  0x207
#define SCROD_AxiVersion_UserID          0x208
#define SCROD_AxiVersion_AxiReset  0x210
#define SCROD_AxiVersion_UserConstants   0x300

// Scrod(%d):AxiCommon(0):*     (register, scrod)
#define SCROD_AxiCommon_rxReadyCnt   0x413
#define SCROD_AxiCommon_txReadyCnt   0x414
#define SCROD_AxiCommon_statusBits   0x470
#define SCROD_AxiCommon_eventCnt   0x481
#define SCROD_AxiCommon_carrierDataSelect  0x482
#define SCROD_AxiCommon_regTrigger   0x4AA
#define SCROD_AxiCommon_trigMask   0x4AB
#define SCROD_AxiCommon_fanoutResetL   0x4B0
#define SCROD_AxiCommon_rollOverEn   0x4F0
#define SCROD_AxiCommon_cntRst     0x4FF

// Scrod(%d):ScrodPs(0):*     (register, scrod)
#define SCROD_PS_elfVersion  0x1810
#define SCROD_PS_liveCounter   0x1811
#define SCROD_PS_pendingWaves  0x1812
#define SCROD_PS_procEventCnt  0x1813
#define SCROD_PS_totalWaves  0x1814
#define SCROD_PS_temp1           0x1815
#define SCROD_PS_temp2           0x1816
#define SCROD_PS_humidity        0x1817
#define SCROD_PS_temp0           0x181C
#define SCROD_PS_featureExtMode  0x1820
#define SCROD_PS_CFDthreshold  0x1821
#define SCROD_PS_CFGPercent  0x1822
#define SCROD_PS_carriersDetected  0x1823
#define SCROD_PS_PedCalcMode   0x1827
#define SCROD_PS_PedCalcStartStatus  0x1828
#define SCROD_PS_PedCalcTimeout  0x1829
#define SCROD_PS_PedCalcNumAvg   0x182A
#define SCROD_PS_ReadbackPedsStatus  0x182D
#define SCROD_PS_ConfigAsicStatus  0x1830
#define SCROD_PS_ConfigAsicErrorDet  0x1831

// feature extraction modes
#define featExtractMode_normal   0
#define featExtractMode_passthru   1
#define featExtractMode_full_norm  2
#define featExtractMode_full_ped   3
#define featExtractMode_debug  4    // not defined yet

// pedestal calc modes
#define pedCalcMode_normal   0
#define pedCalcMode_zeros  1
#define pedCalcMode_pattern  2

#define SCROD_Trigger_     0x1000

// Scrod(%d):AxiXadc(0):*     (register, scrod)
#define SCROD_XADC_Temperature     0x1280
#define SCROD_XADC_V_INT           0x1281
#define SCROD_XADC_V_AUX           0x1282
//#define SCROD_XADC_VrefP           0x1284
//#define SCROD_XADC_VrefN           0x1285
#define SCROD_XADC_V_BRAM          0x1286
//#define SCROD_XADC_SupplyOffsetA   0x1288
//#define SCROD_XADC_AdcOffsetA      0x1289
//#define SCROD_XADC_AdcGainA        0x128A
#define SCROD_XADC_V_PINT          0x128D
#define SCROD_XADC_V_PAUX          0x128E
#define SCROD_XADC_V_DDR           0x128F
#define SCROD_XADC_V_RAW1          0x1290 // AD00P-AD00N RAW1+                      SCROD revB/revB2
#define SCROD_XADC_V_RAW2          0x1291 // AD01P-AD01N RAW2+                      SCROD revB/revB2
#define SCROD_XADC_V_RAW3          0x1292 // AD02P-AD02N RAW3+                      SCROD revB/revB2
#define SCROD_XADC_I_AUXIO         0x1293 // AD03P-AD03N 1V8_AUXIO_VIMON+    LT3055 SCROD revB/revB2
#define SCROD_XADC_I_PINT          0x1294 // AD04P-AD04N 1V0_PINT_VIMON+     LT3055 SCROD revB/revB2
#define SCROD_XADC_I_GTX1p0        0x1295 // AD05P-AD05N 1V0_GTX_VIMON+      LT3086 SCROD revB/revB2
#define SCROD_XADC_I_2p5           0x1296 // AD06P-AD06N 2V5_VIMON+          LT3055 SCROD revB/revB2
#define SCROD_XADC_I_GTX1p2        0x1297 // AD07P-AD07N 1V2_GTX_VIMON+      LT3086 SCROD revB/revB2
#define SCROD_XADC_I_MIO2p5        0x1298 // AD08P-AD08N 2V5_MIO_VIMON+      LT3055 SCROD revB/revB2
#define SCROD_XADC_I_VCCO1p8       0x1299 // AD09P-AD09N 1V8_VCCO_VIMON+     LT3055 SCROD revB/revB2
#define SCROD_XADC_I_VCCO2p5       0x129a // AD10P-AD10N 2V5_VCCO_VIMON+     LT3055 SCROD revB/revB2
#define SCROD_XADC_I_PLL_PAUX      0x129b // AD11P-AD11N 1V8_PLL_PAUX_VIMON+ LT3055 SCROD revB/revB2
#define SCROD_XADC_I_BRAM          0x129c // AD12P-AD12N 1V0_BRAM_VIMON+     LT3055 SCROD revB/revB2
#define SCROD_XADC_I_AUX           0x129d // AD13P-AD13N 1V8_AUX_VIMON+      LT3055 SCROD revB/revB2
#define SCROD_XADC_I_INT           0x129e // AD14P-AD14N 1V0_INT_VIMON+      LT3086 SCROD revB/revB2
#define SCROD_XADC_I_DDR           0x129f // AD15P-AD15N 1V2_DDR_VIMON+      LT3086 SCROD revB/revB2
#define SCROD_XADC_I_3p3           0x1283 // VP0-VN0     3V3_VIMON+          LT3086 SCROD revB/revB2

// Scrod(%d):Pgp2bAxi(%d):*     (ignore this one; let me know if you find it somewhere!)
#define PGP0_CountReset    0x600
#define PGP0_ResetRx     0x601
#define PGP0_Flush       0x602
#define PGP0_Loopback    0x603
#define PGP0_LocData     0x604
#define PGP0_AutoStatus    0x605
#define PGP0_Status    0x608
#define PGP0_RxRemLinkData       0x609
#define PGP0_RxCellErrorCount    0x60A
#define PGP0_RxLinkDownCount     0x60B
#define PGP0_RxLinkErrorCount    0x60C
#define PGP0_RxRemOvfl0Count   0x60D
#define PGP0_RxRemOvfl1Count   0x60E
#define PGP0_RxRemOvfl2Count   0x60F
#define PGP0_RxRemOvfl3Count   0x610
#define PGP0_RxFrameErrorCount   0x611
#define PGP0_RxFrameCount        0x612
#define PGP0_TxLocOvfl0Count   0x613
#define PGP0_TxLocOvfl1Count   0x614
#define PGP0_TxLocOvfl2Count   0x615
#define PGP0_TxLocOvfl3Count   0x616
#define PGP0_TxFrameErrorCount   0x617
#define PGP0_TxFrameCount        0x618
#define PGP0_RxClkFreq           0x619
#define PGP0_TxClkFreq           0x61A


////////////////////
// CARRIER registers
////////////////////
#define CARRIER_UNDEFINED    0xffffff

// Scrod(%d):Carrier(%d):AxiVersion(0):*  (register, scrod, carrier)
#define CARRIER_AxiVersion_FpgaVersion     0x2200
#define CARRIER_AxiVersion_ScratchPad    0x2201
#define CARRIER_AxiVersion_DeviceDnaHigh   0x2202
#define CARRIER_AxiVersion_DeviceDnaLow  0x2203
#define CARRIER_AxiVersion_FdSerialHigh  0x2204
#define CARRIER_AxiVersion_FdSerialLow     0x2205
#define CARRIER_AxiVersion_MasterReset     0x2206
#define CARRIER_AxiVersion_FpgaReload    0x2207
#define CARRIER_AxiVersion_UserConstants   0x2300
#define CARRIER_AxiVersion_BuildStamp    0x2400

// Scrod(%d):Carrier(%d):CarrierCommon(0):* (register, scrod, carrier)
#define CARRIER_AxiCommon_rxReadyCnt     0x2413
#define CARRIER_AxiCommon_txReadyCnt     0x2414
#define CARRIER_AxiCommon_statusBits     0x2470
#define CARRIER_AxiCommon_enableStreaming  0x2480
#define CARRIER_AxiCommon_eventCnt     0x2481
#define CARRIER_AxiCommon_irsxDataSelect   0x2482
#define CARRIER_AxiCommon_regTrigger     0x24AA
#define CARRIER_AxiCommon_trigMask     0x24AB
#define CARRIER_AxiCommon_iDelayRdy    0x24AF
#define CARRIER_AxiCommon_fanoutResetL     0x24B0
#define CARRIER_AxiCommon_rollOverEn     0x24F0
#define CARRIER_AxiCommon_cntRst     0x24FF

// Scrod(%d):Carrier(%d):CarrierPs(0):*   (register, scrod, carrier)
#define CARRIER_PS_asicRegEn   0x3800
#define CARRIER_PS_ampRegEn  0x3801
#define CARRIER_PS_ampEn   0x3802
#define CARRIER_PS_calEn   0x3803
#define CARRIER_PS_calCh   0x3804
#define CARRIER_PS_vPed0   0x3805
#define CARRIER_PS_vPed1   0x3806
#define CARRIER_PS_vPed2   0x3807
#define CARRIER_PS_vPed3   0x3808
#define CARRIER_PS_version   0x3810
#define CARRIER_PS_ID    0x3811
#define CARRIER_PS_temp  0x3812
#define CARRIER_PS_temp01  0x3813
#define CARRIER_PS_temp02  0x3814
#define CARRIER_PS_status  0x3815
#define CARRIER_PS_counter   0x3816
#define CARRIER_PS_ADC0  0x3820
#define CARRIER_PS_ADC1  0x3830
#define CARRIER_PS_XADC  0x3840

// Scrod(%d):Carrier(%d):AxiIrsXDirect(%d):*  (register, scrod, carrier, asic)
#define CARRIER_IRSX_irsxDirect  0x2800
#define CARRIER_IRSX_irsxRegWordOut  0x2801

// Scrod(%d):Carrier(%d):AxiIrsX(%d):*    (register, scrod, carrier, asic)
#define CARRIER_IRSX_spgIn     0x2601
#define CARRIER_IRSX_regLoadPeriod   0x2610
#define CARRIER_IRSX_regLatchPeriod  0x2611
#define CARRIER_IRSX_regClr    0x2612
#define CARRIER_IRSX_wrAddrMode  0x2620
#define CARRIER_IRSX_wrAddrBitSlip   0x2621
#define CARRIER_IRSX_wrAddrFixed   0x2622
#define CARRIER_IRSX_wrAddrStart   0x2623
#define CARRIER_IRSX_wrAddrStop  0x2624
#define CARRIER_IRSX_wrAddrJunk  0x2625
#define CARRIER_IRSX_writesAfterTrig   0x2626
#define CARRIER_IRSX_readoutMode   0x2630
#define CARRIER_IRSX_readoutBitSlip  0x2631
#define CARRIER_IRSX_readoutLookback   0x2632
#define CARRIER_IRSX_readoutWindows  0x2633
#define CARRIER_IRSX_readoutChannels   0x2634
#define CARRIER_IRSX_convertResetWait  0x2635
#define CARRIER_IRSX_hsDataDelay   0x2636
#define CARRIER_IRSX_enableTestPattern  0x2640
#define CARRIER_IRSX_scaler01all   0x2650
#define CARRIER_IRSX_scaler23all   0x2651
#define CARRIER_IRSX_scaler45all   0x2652
#define CARRIER_IRSX_scaler67all   0x2653
#define CARRIER_IRSX_scaler0     0x2654
#define CARRIER_IRSX_scaler1     0x2655
#define CARRIER_IRSX_scaler2     0x2656
#define CARRIER_IRSX_scaler3     0x2657
#define CARRIER_IRSX_scaler4     0x2658
#define CARRIER_IRSX_scaler5     0x2659
#define CARRIER_IRSX_scaler6     0x265A
#define CARRIER_IRSX_scaler7     0x265B
#define CARRIER_IRSX_scaler01dual  0x265C
#define CARRIER_IRSX_scaler23dual  0x265D
#define CARRIER_IRSX_scaler45dual  0x265E
#define CARRIER_IRSX_scaler67dual  0x265F
#define CARRIER_IRSX_trigWidth0  0x2660
#define CARRIER_IRSX_trigWidth1  0x2661
#define CARRIER_IRSX_trigWidth2  0x2662
#define CARRIER_IRSX_trigWidth3  0x2663
#define CARRIER_IRSX_trigWidth4  0x2664
#define CARRIER_IRSX_trigWidth5  0x2665
#define CARRIER_IRSX_trigWidth6  0x2666
#define CARRIER_IRSX_trigWidth7  0x2667
#define CARRIER_IRSX_dualTrigWidth0  0x2668
#define CARRIER_IRSX_dualTrigWidth1  0x2669
#define CARRIER_IRSX_dualTrigWidth2  0x266A
#define CARRIER_IRSX_dualTrigWidth3  0x266B
#define CARRIER_IRSX_wrAddrSpy   0x2680
#define CARRIER_IRSX_tpgData     0x2681
#define CARRIER_IRSX_scalerMontiming   0x2682
#define CARRIER_IRSX_phaseRead   0x2683
#define CARRIER_IRSX_dummyTrigEn   0x2690
#define CARRIER_IRSX_dummyTrigChs  0x2691

// wrAddr modes
#define wrAddrMode_Calibration  0
#define wrAddrMode_Cyclic       1
#define wrAddrMode_Full         2
#define wrAddrMode_Unused       3


// readout modes
#define readoutMode_Calibration  0
#define readoutMode_Forced       1
#define readoutMode_Lookback     2
#define readoutMode_ROI          3


// Scrod(%d):Carrier(%d):AxiXadc(0):*   (register, scrod, carrier)
#define CARRIER_XADC_Temperature     0x3C80
#define CARRIER_XADC_V_INT           0x3C81
#define CARRIER_XADC_V_AUX           0x3C82
//#define CARRIER_XADC_Vin             0x3C83 // not used on carrier revE/E2/E3
//#define CARRIER_XADC_VrefP           0x3C84
//#define CARRIER_XADC_VrefN           0x3C85
#define CARRIER_XADC_V_BRAM          0x3C86
//#define CARRIER_XADC_SupplyOffsetA   0x3C88
//#define CARRIER_XADC_AdcOffsetA      0x3C89
//#define CARRIER_XADC_AdcGainA        0x3C8A
#define CARRIER_XADC_V_PINT          0x3C8D
#define CARRIER_XADC_V_PAUX          0x3C8E
#define CARRIER_XADC_V_DDR           0x3C8F
//#define CARRIER_XADC_VinAux00        0x3C90 // not used on carrier revE/E2/E3
//#define CARRIER_XADC_VinAux01        0x3C91 // not used on carrier revE/E2/E3
//#define CARRIER_XADC_VinAux02        0x3C92 // not used on carrier revE/E2/E3
//#define CARRIER_XADC_VinAux03        0x3C93 // not used on carrier revE/E2/E3
//#define CARRIER_XADC_VinAux04        0x3C94 // not used on carrier revE/E2/E3
//#define CARRIER_XADC_VinAux05        0x3C95 // not used on carrier revE/E2/E3
//#define CARRIER_XADC_VinAux06        0x3C96 // not used on carrier revE/E2/E3
//#define CARRIER_XADC_VinAux07        0x3C97 // not used on carrier revE/E2/E3
//#define CARRIER_XADC_VinAux08        0x3C98 // not used on carrier revE/E2/E3
//#define CARRIER_XADC_VinAux09        0x3C99 // not used on carrier revE/E2/E3
//#define CARRIER_XADC_VinAux10        0x3C9a // not used on carrier revE/E2/E3
//#define CARRIER_XADC_VinAux11        0x3C9b // not used on carrier revE/E2/E3
//#define CARRIER_XADC_VinAux12        0x3C9c // not used on carrier revE/E2/E3
//#define CARRIER_XADC_VinAux13        0x3C9d // not used on carrier revE/E2/E3
//#define CARRIER_XADC_VinAux14        0x3C9e // not used on carrier revE/E2/E3
//#define CARRIER_XADC_VinAux15        0x3C9f // not used on carrier revE/E2/E3

#define SCROD_UNDEFINED 0xffffff
#define CARRIER_UNDEFINED 0xffffff
#define TIMEOUT 0xA16D32FF

#define Raw1_normal 1.715 // normal Voltage on Raw1 line.
#define Raw2_normal 3.049 // normal Voltage on Raw2 line.
#define Raw3_normal 4.374 // normal Voltage on Raw3 line.
#define Raw_margin  0.05  // safety margin on raw voltages

#define irsx168_normal  0x14 // normal conditon:spy port is off,WR_SSEL is in serial mode
#define irsx168_spy    (irsx168_normal | 0x40) // Spy port is on 

#define vPed  0x5C28
#define phase  1

#endif
