##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


from torch import nn


class MultiTrainLoss(nn.Module):
    """
    Sum of cross-entropies for training against LCAS and mass hypotheses.

    Args:
        alpha_mass (float): Weight of mass cross-entropy term in the loss.
        ignore_index (int): Index to ignore in the computation (e.g. padding).
        reduction (str): Type of reduction to be applied on the batch (``sum`` or ``mean``).
    """

    def __init__(
        self,
        alpha_mass=0,
        ignore_index=-1,
        reduction="mean",
    ):
        """
        Initialization
        """
        super().__init__()

        #: Parameter controlling the importance of mass term in loss
        self.alpha_mass = alpha_mass

        #: LCA cross-entropy
        self.LCA_CE = nn.CrossEntropyLoss(
            ignore_index=ignore_index, reduction=reduction
        )
        #: Mass cross-entropy
        self.mass_CE = nn.CrossEntropyLoss(
            ignore_index=ignore_index, reduction=reduction
        )

        assert alpha_mass >= 0, "Alpha should be positive"

    def forward(self, x_input, x_target, edge_input, edge_target, u_input, u_target):
        """
        Called internally by PyTorch to propagate the input.
        """

        LCA_loss = self.LCA_CE(
            edge_input,
            edge_target,
        )

        mass_loss = (
            self.mass_CE(
                x_input,
                x_target,
            )
            if self.alpha_mass > 0
            else 0
        )

        return LCA_loss + self.alpha_mass * mass_loss
