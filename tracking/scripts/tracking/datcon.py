"""Contains calls for the DATCON PXD data reduction chain"""

import basf2


def add_DATCON_simulation(path):
    """Add DATCON to simulation with standard parameters defined in the single modules.
       The DATCON simulation is build up of six blocks:
       1) SVDShaperDigitToDATCONSVDDigitConverter: Converts SVDShaperDigits into DATCONSVDDigits
       2) DATCONSVDSimpleClusterizer: Performs simple clusterisation of SVD hit information
       3) DATCONSVDSpacePointCreator: Creates SpacePoints with the DATCONSVDClusters
       4) DATCONTracking: Tracking algorithms for pattern recognition and track findng
       5) DATCONPXDExtrapolation: Extrapolation to the PXD of the tracks found and calculation of
           Most Probable Hits / PXDIntercepts
       6) DATCONROICalculation: Calculation of ROI on the PXD

    Arguments
    ---------
    path : basf2.Path
        Module path to be adjusted.
    """
    path.add_module('SVDShaperDigitToDATCONSVDDigitConverter')

    path.add_module('DATCONSVDSimpleClusterizer', NoiseLevelADU=5, NoiseCutADU=5)

    path.add_module('DATCONSVDSpacePointCreator')

    path.add_module('DATCONTracking')

    path.add_module('DATCONPXDExtrapolation')

    path.add_module('DATCONROICalculation')
