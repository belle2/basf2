/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck / Klemens Lautenbach                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDUnpackerModule_H
#define PXDUnpackerModule_H

#include <framework/core/Module.h>
#include <pxd/dataobjects/PXDRawHit.h>
#include <pxd/dataobjects/PXDRawAdc.h>
#include <pxd/dataobjects/PXDRawPedestal.h>
#include <pxd/dataobjects/PXDRawROIs.h>
#include <pxd/dataobjects/PXDRawCluster.h>
#include <vxd/dataobjects/VxdID.h>
#include <rawdata/dataobjects/RawPXD.h>
#include <framework/datastore/StoreArray.h>
#include <arpa/inet.h>

namespace Belle2 {

  namespace PXD {

#define ONSEN_MAX_TYPE_ERR  64


    /** The PXDUnpacker module.
     *
     * This module is responsible for unpacking the Raw PXD data to Pixels in v_cellID and u_cellID (global tracking coordinates system)
     */
    class PXDUnpackerModule : public Module {

      // enum { ONSEN_MAX_TYPE_ERR=64};

    public:
      /** Constructor defining the parameters */
      PXDUnpackerModule();

      /** Initialize the module */
      virtual void initialize();
      /** do the unpacking */
      virtual void event();
      /** Terminate the module */
      virtual void terminate();

      int get_errors(int block_number);

      int get_Nrerrors(void);

      std::string get_errTypes(int nr);

      std::string get_errWeight(int nr);

    private:

      std::string m_RawPXDsName;  /**< The name of the StoreArray of processed RawPXDs */
      std::string m_PXDRawHitsName;  /**< The name of the StoreArray of PXDRawHits to be generated */
      std::string m_PXDRawAdcsName;  /**< The name of the StoreArray of PXDRawAdcs to be generated */
      std::string m_PXDRawPedestalsName;  /**< The name of the StoreArray of PXDRawPedestals to be generated */
      std::string m_PXDRawROIsName;  /**< The name of the StoreArray of PXDRawROIs to be generated */
      std::string m_RawClusterName;  /**< The name of the StoreArray of PXDRawROIs to be generated */
      bool m_ignore_headernrframes;

      /**  Swap the endianess of the ONSEN header yes/no */
      bool m_headerEndianSwap;
      /**  ignore missing DATCON */
      bool m_ignoreDATCON;
      /** Only unpack, but Do Not Store anything to file */
      bool m_doNotStore;
      /**< Set to one if data should be remapped according to Belle II node 10 */
      bool m_RemapFlag;
      /** Event Number from MetaInfo */
      unsigned long m_meta_event_nr;
      /** Run Number from MetaInfo */
      unsigned long m_meta_run_nr;
      /** Subrun Number from MetaInfo */
      unsigned long m_meta_subrun_nr;
      /** Experiment from MetaInfo */
      unsigned long m_meta_experiment;
      /** For DESY TB data */
      int m_DESY16_FixTrigOffset;
      /**< Set to one if Start of row flag should be ignored - HOT FIX for gradient */
      bool m_ignoreSorFlag;

      /** Event counter */
      unsigned int m_unpackedEventsCount;
      /** Error counters */
      unsigned int m_errorCounter[ONSEN_MAX_TYPE_ERR];

      /** Input array for PXD Raw. */
      StoreArray<RawPXD> m_storeRawPXD;
      /** Output array for Raw Hits. */
      StoreArray<PXDRawHit> m_storeRawHits;
      /** Output array for Raw ROIs. */
      StoreArray<PXDRawROIs> m_storeROIs;
      /** Output array for Raw Adcs. */
      StoreArray<PXDRawAdc> m_storeRawAdc;
      /** Output array for Raw Adc:Pedestals. */
      StoreArray<PXDRawPedestal> m_storeRawPedestal;
      /** Output array for Clusters. */
      StoreArray<PXDRawCluster> m_storeRawCluster;

      /** Unpack one event (several frames) stored in RawPXD object.
       * @param px RawPXD data object
       */
      void unpack_event(RawPXD& px);

      /** Unpack one frame (within an event).
       * @param data pointer to frame
       * @param len length of frame
       * @param Frame_Number current frame number
       * @param Frames_in_event number of frames in PxdRaw object (subevent)
       */
      void unpack_dhc_frame(void* data, const int len, const int Frame_Number, const int Frames_in_event);

      /** Unpack DHP data within one DHE frame
       * @param data pointer to dhp data
       * @param len length of dhp data
       * @param dhe_first_readout_frame_lo 16 bit of the first readout frame from DHE Start
       * @param dhe_ID raw DHE ID from DHC frame
       * @param dhe_DHPport raw DHP port from DHC frame
       * @param dhe_reformat flag if DHE did reformatting
       * @param toffset triggered row (offset)
       * @param vxd_id vertex Detector ID
       */
      void unpack_dhp(void* data, unsigned int len, unsigned int dhe_first_readout_frame_lo, unsigned int dhe_ID, unsigned dhe_DHPport,
                      unsigned dhe_reformat, unsigned short toffset, VxdID vxd_id);

      /** Unpack DHP RAW data within one DHE frame (pedestals, etc)
       * @param data pointer to dhp data
       * @param len length of dhp data
       * @param dhe_ID raw DHE ID from DHC frame
       * @param dhe_DHPport raw DHP port from DHC frame
       * @param vxd_id vertex Detector ID
       */
      void unpack_dhp_raw(void* data, unsigned int len, unsigned int dhe_ID, unsigned dhe_DHPport, VxdID vxd_id);

      /** Unpack DHP/FCE data within one DHE frame
       * @param data pointer to dhp data
       * @param len length of dhp data
       * @param dhe_first_readout_frame_lo 16 bit of the first readout frame from DHE Start
       * @param dhe_ID raw DHE ID from DHC frame
       * @param dhe_DHPport raw DHP port from DHC frame
       * @param dhe_reformat flag if DHE did reformatting
       * @param toffset triggered row (offset)
       * @param vxd_id vertex Detector ID
       */


      void unpack_fce(unsigned short* data, unsigned int length, VxdID vxd_id);

      /**
       * @param length length of cluster data
       * @param data pointer to cluster data
       * @param vxd_id vertex Detector ID
       */

      int nr5bits(int i) const;/// helper function to "count" nr of set bits within lower 5 bits

      //Remaps rows of inner forward (IF) and outer backward (OB) modules of the PXD
      unsigned int remap_row_IF_OB(unsigned int row_false, unsigned int col_false, unsigned int dhp_id, unsigned int dhe_ID);

      //Remaps cols of inner forward (IF) and outer backward (OB) modules of the PXD
      unsigned int remap_col_IF_OB(unsigned int row_false, unsigned int col_false, unsigned int dhp_id);

      //Remaps rows of inner backward (IB) and outer forward (OF) modules of the PXD
      unsigned int remap_row_IB_OF(unsigned int row_false, unsigned int col_false, unsigned int dhp_id, unsigned int dhe_ID);

      //Remaps cols of inner backward (IB) and outer forward (OF) modules of the PXD
      unsigned int remap_col_IB_OF(unsigned int row_false, unsigned int col_false, unsigned int dhp_id);

      //Lookup tables for DHP remapping
      int LUT_IF_OB[1024] = {240,
                             241, 242, 243, 244, 245, 246, 247, 248, 249, 1023, 1023, 1023, 1023, 1023, 1023, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238,
                             239, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204,
                             205, 206, 207, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170,
                             171, 172, 173, 174, 175, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 128, 129, 130, 131, 132, 133, 134, 135, 136,
                             137, 138, 139, 140, 141, 142, 143, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 96, 97, 98, 99, 100, 101, 102,
                             103, 104, 105, 106, 107, 108, 109, 110, 111, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 64, 65, 66, 67, 68,
                             69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 32, 33, 34,
                             35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 0,
                             1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 490, 491, 492, 493, 494, 495, 496, 497, 498, 499, 1023, 1023, 1023, 1023, 1023,
                             1023, 474, 475, 476, 477, 478, 479, 480, 481, 482, 483, 484, 485, 486, 487, 488, 489, 458, 459, 460, 461, 462, 463, 464, 465, 466, 467, 468, 469, 470,
                             471, 472, 473, 442, 443, 444, 445, 446, 447, 448, 449, 450, 451, 452, 453, 454, 455, 456, 457, 426, 427, 428, 429, 430, 431, 432, 433, 434, 435, 436,
                             437, 438, 439, 440, 441, 410, 411, 412, 413, 414, 415, 416, 417, 418, 419, 420, 421, 422, 423, 424, 425, 394, 395, 396, 397, 398, 399, 400, 401, 402,
                             403, 404, 405, 406, 407, 408, 409, 378, 379, 380, 381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 391, 392, 393, 362, 363, 364, 365, 366, 367, 368,
                             369, 370, 371, 372, 373, 374, 375, 376, 377, 346, 347, 348, 349, 350, 351, 352, 353, 354, 355, 356, 357, 358, 359, 360, 361, 330, 331, 332, 333, 334,
                             335, 336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 314, 315, 316, 317, 318, 319, 320, 321, 322, 323, 324, 325, 326, 327, 328, 329, 298, 299, 300,
                             301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293, 294, 295, 296, 297, 266,
                             267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 281, 250, 251, 252, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 263, 264,
                             265, 740, 741, 742, 743, 744, 745, 746, 747, 748, 749, 1023, 1023, 1023, 1023, 1023, 1023, 724, 725, 726, 727, 728, 729, 730, 731, 732, 733, 734, 735, 736,
                             737, 738, 739, 708, 709, 710, 711, 712, 713, 714, 715, 716, 717, 718, 719, 720, 721, 722, 723, 692, 693, 694, 695, 696, 697, 698, 699, 700, 701, 702,
                             703, 704, 705, 706, 707, 676, 677, 678, 679, 680, 681, 682, 683, 684, 685, 686, 687, 688, 689, 690, 691, 660, 661, 662, 663, 664, 665, 666, 667, 668,
                             669, 670, 671, 672, 673, 674, 675, 644, 645, 646, 647, 648, 649, 650, 651, 652, 653, 654, 655, 656, 657, 658, 659, 628, 629, 630, 631, 632, 633, 634,
                             635, 636, 637, 638, 639, 640, 641, 642, 643, 612, 613, 614, 615, 616, 617, 618, 619, 620, 621, 622, 623, 624, 625, 626, 627, 596, 597, 598, 599, 600,
                             601, 602, 603, 604, 605, 606, 607, 608, 609, 610, 611, 580, 581, 582, 583, 584, 585, 586, 587, 588, 589, 590, 591, 592, 593, 594, 595, 564, 565, 566,
                             567, 568, 569, 570, 571, 572, 573, 574, 575, 576, 577, 578, 579, 548, 549, 550, 551, 552, 553, 554, 555, 556, 557, 558, 559, 560, 561, 562, 563, 532,
                             533, 534, 535, 536, 537, 538, 539, 540, 541, 542, 543, 544, 545, 546, 547, 516, 517, 518, 519, 520, 521, 522, 523, 524, 525, 526, 527, 528, 529, 530,
                             531, 500, 501, 502, 503, 504, 505, 506, 507, 508, 509, 510, 511, 512, 513, 514, 515, 990, 991, 992, 993, 994, 995, 996, 997, 998, 999, 1023, 1023, 1023,
                             1023, 1023, 1023, 974, 975, 976, 977, 978, 979, 980, 981, 982, 983, 984, 985, 986, 987, 988, 989, 958, 959, 960, 961, 962, 963, 964, 965, 966, 967, 968,
                             969, 970, 971, 972, 973, 942, 943, 944, 945, 946, 947, 948, 949, 950, 951, 952, 953, 954, 955, 956, 957, 926, 927, 928, 929, 930, 931, 932, 933, 934,
                             935, 936, 937, 938, 939, 940, 941, 910, 911, 912, 913, 914, 915, 916, 917, 918, 919, 920, 921, 922, 923, 924, 925, 894, 895, 896, 897, 898, 899, 900,
                             901, 902, 903, 904, 905, 906, 907, 908, 909, 878, 879, 880, 881, 882, 883, 884, 885, 886, 887, 888, 889, 890, 891, 892, 893, 862, 863, 864, 865, 866,
                             867, 868, 869, 870, 871, 872, 873, 874, 875, 876, 877, 846, 847, 848, 849, 850, 851, 852, 853, 854, 855, 856, 857, 858, 859, 860, 861, 830, 831, 832,
                             833, 834, 835, 836, 837, 838, 839, 840, 841, 842, 843, 844, 845, 814, 815, 816, 817, 818, 819, 820, 821, 822, 823, 824, 825, 826, 827, 828, 829, 798,
                             799, 800, 801, 802, 803, 804, 805, 806, 807, 808, 809, 810, 811, 812, 813, 782, 783, 784, 785, 786, 787, 788, 789, 790, 791, 792, 793, 794, 795, 796,
                             797, 766, 767, 768, 769, 770, 771, 772, 773, 774, 775, 776, 777, 778, 779, 780, 781, 750, 751, 752, 753, 754, 755, 756, 757, 758, 759, 760, 761, 762,
                             763, 764, 765
                            };




      int LUT_IB_OF[1024] = {243,
                             242, 241, 240, 247, 246, 245, 244, 251, 250, 1023, 1023, 1023, 1023, 1023, 1023, 227, 226, 225, 224, 231, 230, 229, 228, 235, 234, 233, 232, 239, 238, 237,
                             236, 211, 210, 209, 208, 215, 214, 213, 212, 219, 218, 217, 216, 223, 222, 221, 220, 195, 194, 193, 192, 199, 198, 197, 196, 203, 202, 201, 200, 207,
                             206, 205, 204, 179, 178, 177, 176, 183, 182, 181, 180, 187, 186, 185, 184, 191, 190, 189, 188, 163, 162, 161, 160, 167, 166, 165, 164, 171, 170, 169,
                             168, 175, 174, 173, 172, 147, 146, 145, 144, 151, 150, 149, 148, 155, 154, 153, 152, 159, 158, 157, 156, 131, 130, 129, 128, 135, 134, 133, 132, 139,
                             138, 137, 136, 143, 142, 141, 140, 115, 114, 113, 112, 119, 118, 117, 116, 123, 122, 121, 120, 127, 126, 125, 124, 99, 98, 97, 96, 103, 102, 101,
                             100, 107, 106, 105, 104, 111, 110, 109, 108, 83, 82, 81, 80, 87, 86, 85, 84, 91, 90, 89, 88, 95, 94, 93, 92, 67, 66, 65, 64, 71,
                             70, 69, 68, 75, 74, 73, 72, 79, 78, 77, 76, 51, 50, 49, 48, 55, 54, 53, 52, 59, 58, 57, 56, 63, 62, 61, 60, 35, 34, 33,
                             32, 39, 38, 37, 36, 43, 42, 41, 40, 47, 46, 45, 44, 19, 18, 17, 16, 23, 22, 21, 20, 27, 26, 25, 24, 31, 30, 29, 28, 3,
                             2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12, 489, 488, 495, 494, 493, 492, 499, 498, 497, 496, 1023, 1023, 1023, 1023, 1023,
                             1023, 473, 472, 479, 478, 477, 476, 483, 482, 481, 480, 487, 486, 485, 484, 491, 490, 457, 456, 463, 462, 461, 460, 467, 466, 465, 464, 471, 470, 469,
                             468, 475, 474, 441, 440, 447, 446, 445, 444, 451, 450, 449, 448, 455, 454, 453, 452, 459, 458, 425, 424, 431, 430, 429, 428, 435, 434, 433, 432, 439,
                             438, 437, 436, 443, 442, 409, 408, 415, 414, 413, 412, 419, 418, 417, 416, 423, 422, 421, 420, 427, 426, 393, 392, 399, 398, 397, 396, 403, 402, 401,
                             400, 407, 406, 405, 404, 411, 410, 377, 376, 383, 382, 381, 380, 387, 386, 385, 384, 391, 390, 389, 388, 395, 394, 361, 360, 367, 366, 365, 364, 371,
                             370, 369, 368, 375, 374, 373, 372, 379, 378, 345, 344, 351, 350, 349, 348, 355, 354, 353, 352, 359, 358, 357, 356, 363, 362, 329, 328, 335, 334, 333,
                             332, 339, 338, 337, 336, 343, 342, 341, 340, 347, 346, 313, 312, 319, 318, 317, 316, 323, 322, 321, 320, 327, 326, 325, 324, 331, 330, 297, 296, 303,
                             302, 301, 300, 307, 306, 305, 304, 311, 310, 309, 308, 315, 314, 281, 280, 287, 286, 285, 284, 291, 290, 289, 288, 295, 294, 293, 292, 299, 298, 265,
                             264, 271, 270, 269, 268, 275, 274, 273, 272, 279, 278, 277, 276, 283, 282, 249, 248, 255, 254, 253, 252, 259, 258, 257, 256, 263, 262, 261, 260, 267,
                             266, 743, 742, 741, 740, 747, 746, 745, 744, 751, 750, 1023, 1023, 1023, 1023, 1023, 1023, 727, 726, 725, 724, 731, 730, 729, 728, 735, 734, 733, 732, 739,
                             738, 737, 736, 711, 710, 709, 708, 715, 714, 713, 712, 719, 718, 717, 716, 723, 722, 721, 720, 695, 694, 693, 692, 699, 698, 697, 696, 703, 702, 701,
                             700, 707, 706, 705, 704, 679, 678, 677, 676, 683, 682, 681, 680, 687, 686, 685, 684, 691, 690, 689, 688, 663, 662, 661, 660, 667, 666, 665, 664, 671,
                             670, 669, 668, 675, 674, 673, 672, 647, 646, 645, 644, 651, 650, 649, 648, 655, 654, 653, 652, 659, 658, 657, 656, 631, 630, 629, 628, 635, 634, 633,
                             632, 639, 638, 637, 636, 643, 642, 641, 640, 615, 614, 613, 612, 619, 618, 617, 616, 623, 622, 621, 620, 627, 626, 625, 624, 599, 598, 597, 596, 603,
                             602, 601, 600, 607, 606, 605, 604, 611, 610, 609, 608, 583, 582, 581, 580, 587, 586, 585, 584, 591, 590, 589, 588, 595, 594, 593, 592, 567, 566, 565,
                             564, 571, 570, 569, 568, 575, 574, 573, 572, 579, 578, 577, 576, 551, 550, 549, 548, 555, 554, 553, 552, 559, 558, 557, 556, 563, 562, 561, 560, 535,
                             534, 533, 532, 539, 538, 537, 536, 543, 542, 541, 540, 547, 546, 545, 544, 519, 518, 517, 516, 523, 522, 521, 520, 527, 526, 525, 524, 531, 530, 529,
                             528, 503, 502, 501, 500, 507, 506, 505, 504, 511, 510, 509, 508, 515, 514, 513, 512, 989, 988, 995, 994, 993, 992, 999, 998, 997, 996, 1023, 1023, 1023,
                             1023, 1023, 1023, 973, 972, 979, 978, 977, 976, 983, 982, 981, 980, 987, 986, 985, 984, 991, 990, 957, 956, 963, 962, 961, 960, 967, 966, 965, 964, 971,
                             970, 969, 968, 975, 974, 941, 940, 947, 946, 945, 944, 951, 950, 949, 948, 955, 954, 953, 952, 959, 958, 925, 924, 931, 930, 929, 928, 935, 934, 933,
                             932, 939, 938, 937, 936, 943, 942, 909, 908, 915, 914, 913, 912, 919, 918, 917, 916, 923, 922, 921, 920, 927, 926, 893, 892, 899, 898, 897, 896, 903,
                             902, 901, 900, 907, 906, 905, 904, 911, 910, 877, 876, 883, 882, 881, 880, 887, 886, 885, 884, 891, 890, 889, 888, 895, 894, 861, 860, 867, 866, 865,
                             864, 871, 870, 869, 868, 875, 874, 873, 872, 879, 878, 845, 844, 851, 850, 849, 848, 855, 854, 853, 852, 859, 858, 857, 856, 863, 862, 829, 828, 835,
                             834, 833, 832, 839, 838, 837, 836, 843, 842, 841, 840, 847, 846, 813, 812, 819, 818, 817, 816, 823, 822, 821, 820, 827, 826, 825, 824, 831, 830, 797,
                             796, 803, 802, 801, 800, 807, 806, 805, 804, 811, 810, 809, 808, 815, 814, 781, 780, 787, 786, 785, 784, 791, 790, 789, 788, 795, 794, 793, 792, 799,
                             798, 765, 764, 771, 770, 769, 768, 775, 774, 773, 772, 779, 778, 777, 776, 783, 782, 749, 748, 755, 754, 753, 752, 759, 758, 757, 756, 763, 762, 761,
                             760, 767, 766
                            };

      void set_errors(int block_number, std::string errorWeight);

      /** Error Mask set per packet / event*/
      uint64_t m_errorMask;
      /** give verbose unpacking information -> will eb a parameter in next release */
      bool verbose = true;
      /** ignore missing datcon (dont show error) */
      bool ignore_datcon_flag = true;

      unsigned int dheID_order_error = 0;

    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2

#endif // PXDUnpackerModule_H
