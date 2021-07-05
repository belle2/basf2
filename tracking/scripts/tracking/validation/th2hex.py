##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import ROOT

import math
import numpy as np


def TH2Hex(name,
           title,
           n_x_bins,
           x_lower_bound,
           x_upper_bound,
           n_y_bins,
           y_lower_bound,
           y_upper_bound,
           y_orientation=False):
    """Constructor for a two dimensional ROOT histogram TH2Poly setup with hexagonal bins.

    In contrast to TH2Poly.Honeycomb this does not leave any space in the histogram range uncovered
    and allows for different scales in the x and y direction.

    Parameters
    ----------
    name : str
        The name of the histogram to identify the object in ROOT.
    title : str
        Title of the histogram
    n_x_bins : float or int
        Number of hexagons in the x direction to cover the whole range
    x_lower_bound : float
        The lower bound of the histogram in the x direction.
    x_upper_bound : float
        The upper bound of the histogram in the x direction.
    n_y_bins : float or int
        Number of hexagons in the y direction to cover the whole range
    y_lower_bound : float
        The lower bound of the histogram in the y direction.
    y_upper_bound : float
        The upper bound of the histogram in the x direction.
    y_orientation : bool, optional
        Switch whether the hexagones should be primarily aligned along the y coordinate
        instead of the x coordinate, which is the default

    Returns
    -------
    ROOT.TH2Poly
        Two dimensional histogram populated with
    """

    # Create the polygon histogram
    hex_histogram = ROOT.TH2Poly(name,
                                 title,
                                 x_lower_bound,
                                 x_upper_bound,
                                 y_lower_bound,
                                 y_upper_bound)

    # Construct points of a hexagon with unit radius
    # Go clockwise such that root understands what the interal of the polygon is
    pi = math.pi

    unit_radius_hex_xs = np.array([math.sin(2.0 * pi * i / 6.0) for i in range(-2, 4)])
    unit_radius_hex_ys = np.array([math.cos(2.0 * pi * i / 6.0) for i in range(-2, 4)])

    # Move the hex to a reference position such that the lower left corner (8 o'clock position) is at zero
    unit_radius_hex_xs -= unit_radius_hex_xs[0]
    unit_radius_hex_ys -= unit_radius_hex_ys[0]

    # Shrink the hexagon to unit width
    hex_x_width = unit_radius_hex_xs[3] - unit_radius_hex_ys[0]
    hex_y_width = unit_radius_hex_ys[2] - unit_radius_hex_ys[0]
    hex_y_protrusion = unit_radius_hex_ys[2] - unit_radius_hex_ys[1]

    unit_width_hex_xs = unit_radius_hex_xs / hex_x_width
    unit_width_hex_ys = unit_radius_hex_ys / hex_y_width

    # Small hack to cover the other orientation of the hexagons with the same code by swapping x, y
    if y_orientation:
        n_x_bins, n_y_bins = n_y_bins, n_x_bins
        x_lower_bound, y_lower_bound = y_lower_bound, x_lower_bound
        x_upper_bound, y_upper_bound = y_upper_bound, x_upper_bound

    # Expand to the bin width accounting for the protrusion in the y - direction.
    n_y_bins_protrusion_correction = hex_y_protrusion / hex_y_width

    base_x_bin_width = float(x_upper_bound - x_lower_bound) / n_x_bins
    base_y_bin_width = float(y_upper_bound - y_lower_bound) / (n_y_bins - n_y_bins_protrusion_correction)

    base_hex_xs = unit_width_hex_xs * base_x_bin_width
    base_hex_ys = unit_width_hex_ys * base_y_bin_width
    base_y_protrusion = base_hex_ys[2] - base_hex_ys[1]

    # Assume the lower edges of the bins
    x_bin_edges = np.linspace(x_lower_bound, x_upper_bound, n_x_bins + 1)
    even_x_lower_bin_edges = x_bin_edges[:-1]
    # Note: The odd rows have on hexagon more and are displaced by half a bin width
    odd_x_lower_bin_edges = x_bin_edges - base_x_bin_width / 2.0

    y_bin_edges = np.linspace(y_lower_bound, y_upper_bound + base_y_protrusion, n_y_bins + 1)
    y_lower_bin_edges = y_bin_edges[:-1]

    # Construct the hexagonal bins in the histogram
    for i_y_bin, y_lower_bin_edge in enumerate(y_lower_bin_edges):
        if i_y_bin % 2:
            x_lower_bin_edges = odd_x_lower_bin_edges
        else:
            x_lower_bin_edges = even_x_lower_bin_edges

        for x_lower_bin_edge in x_lower_bin_edges:
            hex_xs = base_hex_xs + x_lower_bin_edge
            hex_ys = base_hex_ys + y_lower_bin_edge

            # Swap back x and y introduced by the hack above
            if y_orientation:
                hex_histogram.AddBin(6, hex_ys, hex_xs)
            else:
                hex_histogram.AddBin(6, hex_xs, hex_ys)

    return hex_histogram


def test():
    # Test plot of a two dimensional gaus distribution with hex binning.
    n_data = 1000000
    normal_distributed_x_values = np.random.randn(n_data)
    normal_distributed_y_values = 2.0 * np.random.randn(n_data)
    weights = np.ones_like(normal_distributed_y_values)

    n_x_bins = 100
    x_lower_bound = -3
    x_upper_bound = 3

    n_y_bins = 100
    y_lower_bound = -6
    y_upper_bound = 6

    hex_histogram = TH2Hex("hex_normal",
                           "Two dimensional normal distribution in a hexagonal binning",
                           n_x_bins,
                           x_lower_bound,
                           x_upper_bound,
                           n_y_bins,
                           y_lower_bound,
                           y_upper_bound,
                           y_orientation=True)

    hex_histogram.FillN(n_data, normal_distributed_x_values, normal_distributed_y_values, weights)

    root_palette = {
        "deepsea": 51,  # Deep sea color map
        "grey": 52,  # Grey scale
        "darkbody": 53,  # Dark body radiator
        "hue": 54,  # Weird hue from blue over grey to yellow
        "rainbow": 55,  # A nicer rain bow color map
        "invdarkbody": 56,  # Inverted dark body radiator
    }

    ROOT.gStyle.SetPalette(root_palette["rainbow"])

    hex_histogram.Draw("colz")
    input()


if __name__ == "__main__":
    test()
