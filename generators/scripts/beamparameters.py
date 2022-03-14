#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Scripts and functions to set the BeamParameters from known configurations
"""

from basf2 import B2FATAL, B2INFO
import math
import numpy as np

#: names of parameters to set and some default values to be inherited by all presets
beamparameter_defaults = {
    "energyHER": 5.28,
    "energyLER": 5.28,
    "angleXHER": 0,
    "angleXLER": math.pi,
    "covHER": [0],
    "covLER": [0],
    "vertex": [0, 0, 0],
    "covVertex": [0]
}

#: presets for known configurations
# its a dictionary of name -> (parent, settings). A preset can inherit from a
# parent and only override the parameters which are actually different. For
# example Y4S-off could inherit from Y4S and just have different beam energies.
# All Belle2 settings inherit from SuperKEKB for now.
# A preset can either provide a vertex covariance matrix or bunch sizes for LER
# and HER. In the latter case the covariance matrix will be calculated from the
# bunch size by assuming normal distribution and multiplying the PDF
beamparameter_presets = {
    "SuperKEKB": (None, {
        # beam energies in GeV
        "energyHER": 7.,
        "energyLER": 4.,
        # beam angles with respect to the z axis in radian, negative gets
        # converted into pi - |angle|
        "angleXHER": 0.0415,
        "angleXLER": -0.0415,
        # covariance matrices for the beam parametrized in
        # (E, theta_x, theta_y) = energy and horizontal and vertical angular
        # spread
        "covHER": np.array([0.00513]) ** 2,
        "covLER": np.array([0.002375]) ** 2,
        # bunch size in cm
        "bunchHER": np.array([10.2e-3, 59e-6, 6.0]) * 1e-1,
        "bunchLER": np.array([7.75e-3, 59e-6, 5.0]) * 1e-1,
    }),
    "Y1S": ("SuperKEKB", {  # m(Y1S) = 9.460 GeV
        "energyHER": 6.263,
        "energyLER": 3.579,
    }),
    "Y2S": ("SuperKEKB", {  # m(Y2S) = 10.023 GeV
        "energyHER": 6.635,
        "energyLER": 3.792,
    }),
    "Y3S": ("SuperKEKB", {  # m(Y3S) = 10.355 GeV
        "energyHER": 6.855,
        "energyLER": 3.917,
    }),
    "Y4S": ("SuperKEKB", {  # m(Y4S) = 10.579 GeV
        "energyHER": 7.004,
        "energyLER": 4.002,
    }),
    "Y5S": ("SuperKEKB", {  # m(Y5S) = 10.876 GeV
        "energyHER": 7.200,
        "energyLER": 4.114,
    }),
    "Y1S-off": ("Y1S", {  # m(Y1S) - 60 MeV = 9.400 GeV
        "energyHER": 6.223,
        "energyLER": 3.556,
    }),
    "Y2S-off": ("Y2S", {  # m(Y2S) - 60 MeV = 9.963 GeV
        "energyHER": 6.596,
        "energyLER": 3.769,
    }),
    "Y3S-off": ("Y3S", {  # m(Y3S) - 60 MeV = 10.295 GeV
        "energyHER": 6.816,
        "energyLER": 3.895,
    }),
    "Y4S-off": ("Y4S", {  # m(Y4S) - 60 MeV = 10.519 GeV
        "energyHER": 6.964,
        "energyLER": 3.979,
    }),
    "Y5S-off": ("Y5S", {  # m(Y5S) - 60 MeV = 10.816 GeV
        "energyHER": 7.160,
        "energyLER": 4.092,
    }),
    "Y1D1": ("SuperKEKB", {  # m(Y1D) = 10.152 GeV
        "energyHER": 6.720,
        "energyLER": 3.840,
    }),
    "Y2D1": ("SuperKEKB", {  # m(Y2D) = 10.425 GeV
        "energyHER": 6.901,
        "energyLER": 3.944,
    }),
    "Y6S": ("SuperKEKB", {  # m(Y6S) = 11.020 GeV
        "energyHER": 7.295,
        "energyLER": 4.169,
    }),
    "Yb10775": ("SuperKEKB", {  # m(Yb10775) = 10.775 GeV
        "energyHER": 7.133,
        "energyLER": 4.076,
    }),
    "KEKB-Belle": (None, {
        "energyHER": 7.998213,
        "energyLER": 3.499218,
        "angleXHER": 0.022,
    }),
    "LEP1": (None, {
        "energyHER": 45.6,
        "energyLER": 45.6,
    }),
    "DAPHNE": (None, {
        "energyHER": 1.02 / 2.,
        "energyLER": 1.02 / 2.,
    }),
}


def __get_4vector(energy, angle):
    """Calculate an 4vector for electron/positron from energy and angle"""
    import ROOT
    m = 0.511e-3
    pz = (energy ** 2 - m ** 2) ** .5
    v = ROOT.Math.PxPyPzEVector(0, 0, pz, energy)
    if angle < 0:
        angle = math.pi + angle
    rotationY = ROOT.Math.RotationY(angle)
    v = rotationY(v)
    return v


def rot_matrix_y(angle):
    """Return rotation matrix for a rotation of angle around the y-axis"""
    c = math.cos(angle)
    s = math.sin(angle)
    return np.matrix([(c, 0, s), (0, 1, 0), (-s, 0, c)])


def cov_matrix(beamsize, angle):
    """Return the covariance matrix for the given bunch size and angle with
    respect to the z axis"""
    if angle < 0:
        angle = math.pi + angle
    rot = rot_matrix_y(angle)
    cov = np.matrix(np.diag(beamsize ** 2))
    return rot * cov * rot.T


def calculate_beamspot(pos_her, pos_ler, size_her, size_ler, angle_her, angle_ler):
    """
    Function to calculate position and covariance matrix of the beamspot.

    This function calculates the mean position and the covariance matrix for the
    beamspot from the bunch positions, bunch sizes and the angles of the beam
    with respect to the z-axis. It assumes normal distributed bunch densities and
    multiplies the PDFs for the two bunches to get a resulting beamspot

    negative angles are interpreted as "math.pi - abs(angle)"

    Args:
        pos_her (list): mean position of the HER bunch (in cm)
        pos_ler (list): mean position of the LER bunch (in cm)
        size_her (list): HER bunch size in cm
        size_ler (list): LER bunch size in cm
        angle_her (float): angle between HER direction and z-axis
        angle_her (float): angle between LER direction and z-axis

    Returns:
        pos (array): array (x, y, z) with the spot center in cm
        cov (matrix): covariance matrix of the beam spot
    """

    cov_her = cov_matrix(size_her, angle_her)
    cov_ler = cov_matrix(size_ler, angle_ler)
    # calculate the inverse of the sum of the two covariance matrices
    cov_sum_i = (cov_her + cov_ler).I
    # and use it to calculate the product of the two gaussian distributions,
    # covariance and mean
    cov_spot = cov_her * cov_sum_i * cov_ler
    beampos_spot = np.array(
        cov_ler * cov_sum_i * np.matrix(pos_her).T +
        cov_her * cov_sum_i * np.matrix(pos_ler).T
    ).T[0]
    return beampos_spot, cov_spot


def add_beamparameters(path, name, E_cms=None, **argk):
    """Add BeamParameter module to a given path

    Args:
        path (basf2.Path instance): path to add the module to
        name (str): name of the beamparameter settings to use
        E_cms (float): center of mass energy. If not None the beamenergies will
            be scaled accordingly to achieve E_cms

    Additional keyword arguments will be passed directly to the module as parameters.
    """
    # calculate all the parameter values from the preset
    values = calculate_beamparameters(name, E_cms)
    # add a BeamParametes module to the path
    module = path.add_module("BeamParameters")
    module.set_name("BeamParameters:%s" % name)
    # finally, set all parameters and return the module
    module.param(values)
    # and override parameters with any additional keyword arguments
    module.param(argk)
    return module


def calculate_beamparameters(name, E_cms=None):
    """Get/calculate the necessary beamparameter values from the given preset name,
    optionally scale it to achieve the given cms energy

    Args:
        name (str): name of the beamparameter settings to use
        E_cms (float): center of mass energy. If not None the beamenergies will
            be scaled accordingly to achieve E_cms
    """
    if name not in beamparameter_presets:
        B2FATAL("Unknown beamparameter preset: '%s', use one of %s" %
                (name, ", ".join(sorted(beamparameter_presets.keys()))))
        return None

    # copy the defaults
    values = beamparameter_defaults.copy()

    # collect all the values from the preset by also looping over the
    preset = {}

    def collect_values(preset, name):
        parent, values = beamparameter_presets[name]
        if parent is not None:
            collect_values(preset, parent)
        preset.update(values)

    collect_values(preset, name)

    # and update with the preset
    for k in values.keys():
        if k in preset:
            if isinstance(preset[k], np.ndarray):
                values[k] = list(preset[k].flat)
            else:
                values[k] = preset[k]

    # if we have bunch sizes we want to compute the vertex covariance from them
    if "bunchHER" in preset and "bunchLER" in preset:
        her_size = preset["bunchHER"]
        ler_size = preset["bunchLER"]
        her_angle = values["angleXHER"]
        ler_angle = values["angleXLER"]
        _, cov = calculate_beamspot(
            [0, 0, 0], [0, 0, 0], her_size, ler_size, her_angle, ler_angle
        )
        values["covVertex"] = list(cov.flat)

    if E_cms is not None:
        ler = __get_4vector(values["energyLER"], values["angleXLER"])
        her = __get_4vector(values["energyHER"], values["angleXHER"])
        mass = (her + ler).M()
        scale = E_cms / mass
        B2INFO("Scaling beam energies by %g to obtain E_cms = %g GeV" % (scale, E_cms))
        values["energyHER"] *= scale
        values["energyLER"] *= scale

    return values


if __name__ == "__main__":
    # if called directly we will
    # 1. calculate the beamspot for the SuperKEKB preset and display it if possible
    # 2. calculate beam energies for Y1S - Y5S and offresonance
    np.set_printoptions(precision=3)

    # calculate beamspot for SuperKEKB for testing
    #: beamparameter defaults for SuperKEKB
    values = beamparameter_presets["SuperKEKB"][1]
    beampos_spot, cov_spot = calculate_beamspot(
        [0, 0, 0], [0, 0, 0],
        values["bunchHER"], values["bunchLER"],
        values["angleXHER"], values["angleXLER"],
    )
    print("Beamspot position:")
    print(beampos_spot)
    print("Beamspot covariance:")
    print(cov_spot)
    print("Beamspot dimensions (in mm, axes in arbitary order):")
    print(np.linalg.eig(cov_spot)[0] ** .5 * 10)

    # see if we can plot it
    try:
        from matplotlib import pyplot as pl
        #: covariance matrix for her bunch
        cov_her = cov_matrix(values["bunchHER"], values["angleXHER"])
        #: covariance matrix for ler bunch
        cov_ler = cov_matrix(values["bunchLER"], values["angleXLER"])
        #: random points according to her bunch covariance
        points_her = np.random.multivariate_normal([0, 0, 0], cov_her, 1000)
        #: random points according to ler bunch covariance
        points_ler = np.random.multivariate_normal([0, 0, 0], cov_ler, 1000)
        #: random points according to calculated beamspot
        points_spot = np.random.multivariate_normal(beampos_spot, cov_spot, 1000)
        pl.scatter(points_her[:, 2], points_her[:, 0], s=5, marker=".", c="b", edgecolors="None", label="HER")
        pl.scatter(points_ler[:, 2], points_ler[:, 0], s=5, marker=".", c="r", edgecolors="None", label="LER")
        pl.scatter(points_spot[:, 2], points_spot[:, 0], s=5, marker=".", c="g", edgecolors="None", label="spot")
        pl.legend()
        pl.xlabel("z / cm")
        pl.ylabel("x / cm")
        pl.tight_layout()
        pl.show()
    except ImportError:
        pass

    #: nominal SuperKEKB HER energy
    eher = values["energyHER"]
    #: nominal SuperKEKB LER energy
    eler = values["energyLER"]
    #: nominal SuperKEKB HER angle
    aher = values["angleXHER"]
    #: nominal SuperKEKB LER angle
    aler = values["angleXLER"]

    # calculate beam energies for Y1S - Y4S by scaling them from the nominal
    # beam energies for the SuperKEKB preset
    #: nominal SuperKEKB HER 4vector
    ler = __get_4vector(eher, aher)
    #: nominal SuperKEKB LER 4vector
    her = __get_4vector(eler, aler)
    #: nominal SuperKEKB CMS energy
    mass = (her + ler).M()
    print("Nominal CMS Energy: ", mass)

    #: target CMS energies from PDG
    targets = {
        "Y1S": 9460.30e-3,
        "Y2S": 10023.26e-3,
        "Y3S": 10355.2e-3,
        "Y4S": 10579.4e-3,
        "Y5S": 10876e-3
    }
    for name, energy in sorted(targets.items()):
        #: scaling between nominal and target CMS energy
        scale = energy / mass
        print("""\
            "%s": ("SuperKEKB", {  # m(%s) = %.3f GeV
                "energyHER": %.3f,
                "energyLER": %.3f,
            }),""" % (name, name, energy, eher * scale, eler * scale))

    for name, energy in sorted(targets.items()):
        #: scaling between nominal and target CMS energy for off resoncane, i.e. 60MeV lower
        scale = (energy - 60e-3) / mass
        print("""\
            "%s-off": ("%s", {  # m(%s) - 60 MeV = %.3f GeV
                "energyHER": %.3f,
                "energyLER": %.3f,
            }),""" % (name, name, name, energy - 60e-3, eher * scale, eler * scale))
