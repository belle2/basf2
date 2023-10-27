from torch import nn


# class B_MomentumLoss(nn.Module):
#     """Loss function to train the model against the B momentum.
#        Actually not used at the moment.
#     """

#     def __init__(
#         self,
#         reduction="mean",
#     ):
#         super().__init__()
#         self.rho_L1 = nn.L1Loss(reduction=reduction)
#         self.theta_L1 = nn.L1Loss(reduction=reduction)
#         self.phi_L1 = nn.L1Loss(reduction=reduction)

#     def forward(self, u_input, u_target):
#         x, y, z = u_input[:, 0], u_input[:, 1], u_input[:, 2]
#         x_y, y_y, z_y = u_target[:, 0], u_target[:, 1], u_target[:, 2]

#         rho = torch.sqrt(x**2 + y**2 + z**2)
#         rho_y = torch.sqrt(x_y**2 + y_y**2 + z_y**2)

#         cosTheta = z / rho
#         cosTheta_y = z_y / rho_y

#         cosPhi = x / torch.sqrt(x**2 + y**2)
#         cosPhi_y = x_y / torch.sqrt(x_y**2 + y_y**2)

#         rho_loss = self.rho_L1(rho, rho_y)
#         cosTheta_loss = self.theta_L1(cosTheta, cosTheta_y)
#         cosPhi_loss = self.phi_L1(cosPhi, cosPhi_y)

#         return rho_loss + cosTheta_loss + cosPhi_loss


class MultiTrainLoss(nn.Module):
    """
    Loss function to train the model against a variety of targets: LCA, B momentum, B probability and masses of FSP.

    Args:
        alpha_mass (float): weight of masses term in the loss
        ignore_index (int): index to ignore while padding
        reduction (string): type of reduction to be applied on the batch
        edge_weights (array): weights applied to edge classes
        node_weights (array): weights applied to node classes
    Returns:
        Total loss
    """

    def __init__(
        self,
        alpha_mass=0,
        # alpha_momentum=0,
        # alpha_prob=0,
        ignore_index=-1,
        reduction="mean",
        # global_layer=False,
        edge_weights=None,
        node_weights=None,
    ):
        super().__init__()
        self.alpha_mass = alpha_mass
        # self.alpha_momentum = alpha_momentum
        # self.alpha_prob = alpha_prob
        # self.global_layer = global_layer

        self.LCA_CE = nn.CrossEntropyLoss(
            weight=edge_weights, ignore_index=ignore_index, reduction=reduction
        )
        self.mass_CE = nn.CrossEntropyLoss(
            weight=node_weights, ignore_index=ignore_index, reduction=reduction
        )
        # self.prob_CE = nn.BCEWithLogitsLoss(reduction=reduction)
        # self.momentum_L1 = B_MomentumLoss(reduction=reduction)

        self.scaling = 1 if reduction == "mean" else 100

        assert (
            alpha_mass >= 0  # and alpha_momentum >= 0 and alpha_prob >= 0
        ), "Alpha should be positive"

    def forward(self, x_input, x_target, edge_input, edge_target, u_input, u_target):
        # prob_input = u_input if self.global_layer else None
        # prob_target = u_target
        # p_input = u_input[:, :3] if self.global_layer else None
        # p_target = u_target[:, :3]

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

        # prob_loss = (
        #     self.prob_CE(
        #         prob_input,
        #         prob_target,
        #     )
        #     if self.alpha_prob > 0
        #     else 0
        # )

        # momentum_loss = (
        #     self.momentum_L1(
        #         p_target,
        #         p_input,
        #     )
        #     if self.alpha_momentum > 0
        #     else 0
        # )

        return (
            LCA_loss
            + self.alpha_mass * mass_loss
            # + self.alpha_prob * prob_loss
            # + self.alpha_momentum * momentum_loss
        ) / self.scaling
