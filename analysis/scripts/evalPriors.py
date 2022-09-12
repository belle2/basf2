#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# torch imports
import torch
from torch.nn.functional import one_hot

# other imports
import numpy as np
import uproot3 as ur
from sklearn.preprocessing import PolynomialFeatures

# Model
from priorDataLoaderAndModel import PriorModel

# dataloader


def data_load(data: np.array) -> torch.FloatTensor:
    """
    Creates a dataset in the format that is required by the model for processing.

    Parameters:
        data(np.array): A 2D numpy array containing cos(theta) as the first column and momentum as the second column.

    Returns:
        A torch tensor containing second order polynomial feature transformation of the provided data along with the
        additional transverse momentum.
    """
    x = np.sin(np.arccos(data[:, 0])) * data[:, 1]
    x = np.hstack((data, x.reshape(-1, 1)))
    pf = PolynomialFeatures(2, include_bias=False)
    x = pf.fit_transform(x)
    return torch.FloatTensor(x)


# scaling for calibration
def scaling(dat: str):
    """
    Creates the temperature scaling object for calibration.

    Parameters:
        dat(str): The path to the scaling file generated during training.

    Returns:
        The scaling class to transform the output predictions from the model.
    """
    # TemperatureScaling imported within the scaling function as it is not
    # included with basf2 and calibration is not mandatory so it can be
    # avoided if calibration is not required.
    from netcal.scaling import TemperatureScaling

    data = ur.open(dat)["scale"].pandas.df()
    pdg_list = list(data.columns)
    pdg_list.remove("truth")
    sorted_list = np.sort([e[:-4] for e in pdg_list])
    y = data[sorted_list[0] + "_pdg"].values.reshape(-1, 1)
    for i in range(len(sorted_list) - 1):
        y = np.hstack((y, data[sorted_list[i + 1] + "_pdg"].values.reshape(-1, 1)))
    temp = TemperatureScaling()
    temp.fit(y, one_hot(torch.LongTensor(data["truth"].values)).numpy())
    return temp


class Priors:
    """
    Class to calculate PID prior probabilities and posteriors.

    Attributes:
        model(PriorModel): The trained model to be used for evaluation.
        plist(np.array): List of particle PDGs for which the model was trained.
        require_scale(bool): True if a scaling file is provided or else False.
        scale(TemperatureScaling) (optional): Calibration object constructed for temperature scaling.
    """

    def __init__(self, particlelist: list, Model: str, prescaling: str = None):
        """
        Initialize the Priors class.

        Parameters:
            particlelist(list(int)): List of PDG values for which the model was trained.
            Model(str): Path to a previously trained model which will be used to calculate priors.
            prescaling(str) (optional): Path to the scaling file created while training the model.
        """
        model = PriorModel(len(particlelist))
        model.load_state_dict(torch.load(Model))
        model.eval()
        if torch.cuda.is_available():
            model = model.to("cuda")
        #: The torch model for prior calculation.
        self.model = model
        if prescaling is not None:
            scale = scaling(prescaling)
            #: Temperature scaling object for calibration.
            self.scale = scale
            #: True if the scaling object exist.
            self.require_scale = True
        else:
            #: False if the scaling object doesn't exist.
            self.require_scale = False
        #: Sorted particle PDG list.
        self.plist = np.sort(particlelist)

    def calculate_priors(self, momentum: np.array, cosTheta: np.array):
        """
        Calculates priors for given momentum and cos(theta).

        Parameters:
            momentum(np.array): A numpy array containing the momentum of particles.
            cosTheta(np.array): A numpy array containing the cosTheta information of particles.

        Returns:
            None.
        """
        y = data_load(np.hstack((cosTheta.reshape(-1, 1), momentum.reshape(-1, 1))))
        if torch.cuda.is_available():
            y = y.to("cuda")
        out = self.model(y)
        if torch.cuda.is_available():
            out = out.to("cpu")
        out = out.detach().numpy()

        if self.require_scale:
            out = self.scale.transform(out)
        #: Numpy array containing PID prior probability data.
        self.prior = out

    def get_priors(self, pdg: int = None) -> np.array:
        """
        Gives the calculated PID priors.

        Parameters:
            pdg(int) (optional): The PDG value of the particles for which prior probabilities are needed.

        Returns:
            A 1D array conatining prior probabilities for required particle in case PDG value is specified;
            else it will return a 2D array for all particles that were used during training.
        """
        if pdg is not None:
            index = np.where(self.plist == pdg)[0][0]
            return self.prior[:, index]
        else:
            return self.prior

    def get_posterior(self, pid: int, pdg: int = None) -> np.array:
        """
        Get PID posterior probabilities.

        Parameters:
            pid(np.array): The PID values for the particles used during training process arranged in ascending order of PDG values.
            pdg(int) (optional): PDG value of particle for which posterior is required.

        Returns:
            A 1D array of posterior probabilities in case PDG value is provided else returns a 2D array containing
            the posteriors for all particles.
        """
        priorpid = np.multiply(self.prior, pid)
        sumprpid = np.sum(priorpid, axis=1)
        posterior = np.divide(priorpid, sumprpid.reshape(-1, 1))
        if pdg is None:
            return posterior
        else:
            index = np.where(self.plist == pdg)[0][0]
            return posterior[:, index]
