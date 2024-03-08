##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


import torch
from torch_scatter import scatter
from ignite.metrics import Metric
from ignite.exceptions import NotComputableError
from ignite.metrics.metric import sync_all_reduce, reinit__is_reduced


class PerfectLCA(Metric, object):
    """
    Computes the rate of perfectly predicted LCAS matrices over a batch.

    ``output_transform`` should return the following items: ``(edge_pred, edge_y, edge_index, u_y, batch, num_graphs)``.

    * ``edge_pred`` must contain edge prediction logits and have shape (num_edges_in_batch, edge_classes);
    * ``edge_y`` must contain edge ground-truth class indices and have shape (num_edges_in_batch, 1);
    * ``edge index`` maps edges to its nodes;
    * ``u_y`` is the signal/background class (always 1 in the current setting);
    * ``batch`` maps nodes to their graph;
    * ``num_graphs`` is the number of graph in a batch (could be derived from ``batch`` also).

    .. seealso::
        `Ignite metrics <https://pytorch.org/ignite/metrics.html>`_

    :param ignore_index: Class or list of classes to ignore during the computation (e.g. padding).
    :type ignore_index: list[int]
    :param output_transform: Function to transform engine's output to desired output.
    :type output_transform: `function <https://docs.python.org/3/glossary.html#term-function>`_
    :param device: ``cpu`` or ``gpu``.
    :type device: str
    """

    def __init__(self, ignore_index, output_transform, device='cpu'):
        """
        Initialization.
        """
        #: Ignore index
        self.ignore_index = ignore_index if isinstance(ignore_index, list) else [ignore_index]
        #: CPU or GPU
        self.device = device
        #: Good samples
        self._per_corrects = None
        #: Total samples
        self._num_examples = None

        super(PerfectLCA, self).__init__(output_transform=output_transform, device=device)

    @reinit__is_reduced
    def reset(self):
        """
        Resets counters.
        """
        self._per_corrects = 0
        self._num_examples = 0

        super(PerfectLCA, self).reset()

    @reinit__is_reduced
    def update(self, output):
        """
        Updates counts.
        """
        edge_pred, edge_y, edge_index, u_y, batch, num_graphs = output

        num_graphs = num_graphs.item()

        probs = torch.softmax(edge_pred, dim=1)
        winners = probs.argmax(dim=1)

        assert winners.shape == edge_y.shape, 'Edge predictions shape does not match target shape'

        # Create a mask for the zeroth elements (padded entries)
        mask = torch.ones(edge_y.size(), dtype=torch.long, device=self.device)
        for ig_class in self.ignore_index:
            mask &= (edge_y != ig_class)

        # Zero the respective entries in the predictions
        y_pred_mask = winners * mask
        y_mask = edge_y * mask

        # (N) compare the masked predictions with the target. The padded will be equal due to masking
        truth = y_pred_mask.eq(y_mask) + 0  # +0 so it's not bool but 0 and 1
        truth = scatter(truth, edge_index[0], reduce="min")
        truth = scatter(truth, batch, reduce="min")

        # Count the number of zero wrong predictions across the batch
        batch_perfect = truth.sum().item()

        self._per_corrects += batch_perfect
        self._num_examples += num_graphs

    @sync_all_reduce("_perfectLCA")
    def compute(self):
        """
        Final result.
        """
        if self._num_examples == 0:
            raise NotComputableError(
                "CustomAccuracy must have at least one example before it can be computed."
            )
        return self._per_corrects / self._num_examples


class PerfectMasses(Metric, object):
    """
    Computes the rate of events with perfectly predicted mass hypotheses over a batch.

    ``output_transform`` should return the following items: ``(x_pred, x_y, u_y, batch, num_graphs)``.

    * ``x_pred`` must contain node prediction logits and have shape (num_nodes_in_batch, node_classes);
    * ``x_y`` must contain node ground-truth class indices and have shape (num_nodes_in_batch, 1);
    * ``u_y`` is the signal/background class (always 1 in the current setting);
    * ``batch`` maps nodes to their graph;
    * ``num_graphs`` is the number of graph in a batch (could be derived from ``batch`` also).

    .. seealso::
        `Ignite metrics <https://pytorch.org/ignite/metrics.html>`_

    :param ignore_index: Class or list of classes to ignore during the computation (e.g. padding).
    :type ignore_index: list[int]
    :param output_transform: Function to transform engine's output to desired output.
    :type output_transform: `function <https://docs.python.org/3/glossary.html#term-function>`_
    :param device: ``cpu`` or ``gpu``.
    :type device: str
    """

    def __init__(self, ignore_index, output_transform, device='cpu'):
        """
        Initialization.
        """
        #: Ignore index
        self.ignore_index = ignore_index if isinstance(ignore_index, list) else [ignore_index]
        #: CPU or GPU
        self.device = device
        #: Good samples
        self._per_corrects = None
        #: Total samples
        self._num_examples = None

        super(PerfectMasses, self).__init__(output_transform=output_transform, device=device)

    @reinit__is_reduced
    def reset(self):
        """
        Resets counts.
        """
        self._per_corrects = 0
        self._num_examples = 0

        super(PerfectMasses, self).reset()

    @reinit__is_reduced
    def update(self, output):
        """
        Updates counts.
        """
        x_pred, x_y, u_y, batch, num_graphs = output

        num_graphs = num_graphs.item()

        probs = torch.softmax(x_pred, dim=1)
        winners = probs.argmax(dim=1)

        assert winners.shape == x_y.shape, 'Mass predictions shape does not match target shape'

        # Create a mask for the zeroth elements (padded entries)
        mask = torch.ones(x_y.size(), dtype=torch.long, device=self.device)
        for ig_class in self.ignore_index:
            mask &= (x_y != ig_class)

        # Zero the respective entries in the predictions
        y_pred_mask = winners * mask
        y_mask = x_y * mask

        # (N) compare the masked predictions with the target. The padded will be equal due to masking
        truth = y_pred_mask.eq(y_mask) + 0  # +0 so it's not bool but 0 and 1
        truth = scatter(truth, batch, reduce="min")

        # Count the number of zero wrong predictions across the batch
        batch_perfect = truth.sum().item()

        self._per_corrects += batch_perfect
        self._num_examples += num_graphs

    @sync_all_reduce("_perfectMasses")
    def compute(self):
        """
        Final computation.
        """
        if self._num_examples == 0:
            raise NotComputableError(
                "CustomAccuracy must have at least one example before it can be computed."
            )
        return self._per_corrects / self._num_examples


class PerfectEvent(Metric, object):
    """
    Computes the rate of events with perfectly predicted mass hypotheses and LCAS matrices over a batch.

    ``output_transform`` should return the following items:
    ``(x_pred, x_y, edge_pred, edge_y, edge_index, u_y, batch, num_graphs)``.

    * ``x_pred`` must contain node prediction logits and have shape (num_nodes_in_batch, node_classes);
    * ``x_y`` must contain node ground-truth class indices and have shape (num_nodes_in_batch, 1);
    * ``edge_pred`` must contain edge prediction logits and have shape (num_edges_in_batch, edge_classes);
    * ``edge_y`` must contain edge ground-truth class indices and have shape (num_edges_in_batch, 1);
    * ``edge index`` maps edges to its nodes;
    * ``u_y`` is the signal/background class (always 1 in the current setting);
    * ``batch`` maps nodes to their graph;
    * ``num_graphs`` is the number of graph in a batch (could be derived from ``batch`` also).

    .. seealso::
        `Ignite metrics <https://pytorch.org/ignite/metrics.html>`_

    :param ignore_index: Class or list of classes to ignore during the computation (e.g. padding).
    :type ignore_index: list[int]
    :param output_transform: Function to transform engine's output to desired output.
    :type output_transform: `function <https://docs.python.org/3/glossary.html#term-function>`_
    :param device: ``cpu`` or ``gpu``.
    :type device: str
    """

    def __init__(self, ignore_index, output_transform, device='cpu'):
        """
        Initialization.
        """
        #: Ignore index
        self.ignore_index = ignore_index if isinstance(ignore_index, list) else [ignore_index]
        #: CPU or GPU
        self.device = device
        #: Good samples
        self._per_corrects = None
        #: Total samples
        self._num_examples = None

        super(PerfectEvent, self).__init__(output_transform=output_transform, device=device)

    @reinit__is_reduced
    def reset(self):
        """
        Resets counts.
        """
        self._per_corrects = 0
        self._num_examples = 0

        super(PerfectEvent, self).reset()

    @reinit__is_reduced
    def update(self, output):
        """
        Updates counts.
        """
        x_pred, x_y, edge_pred, edge_y, edge_index, u_y, batch, num_graphs = output

        num_graphs = num_graphs.item()

        x_probs = torch.softmax(x_pred, dim=1)
        x_winners = x_probs.argmax(dim=1)
        edge_probs = torch.softmax(edge_pred, dim=1)
        edge_winners = edge_probs.argmax(dim=1)

        assert x_winners.shape == x_y.shape, 'Mass predictions shape does not match target shape'
        assert edge_winners.shape == edge_y.shape, 'Edge predictions shape does not match target shape'

        # Create a mask for the zeroth elements (padded entries)
        x_mask = torch.ones(x_y.size(), dtype=torch.long, device=self.device)
        edge_mask = torch.ones(edge_y.size(), dtype=torch.long, device=self.device)
        for ig_class in self.ignore_index:
            x_mask &= (x_y != ig_class)
            edge_mask &= (edge_y != ig_class)

        # Zero the respective entries in the predictions
        x_pred_mask = x_winners * x_mask
        x_mask = x_y * x_mask
        edge_pred_mask = edge_winners * edge_mask
        edge_mask = edge_y * edge_mask

        # (N) compare the masked predictions with the target. The padded will be equal due to masking
        # Masses
        x_truth = x_pred_mask.eq(x_mask) + 0  # +0 so it's not bool but 0 and 1
        x_truth = scatter(x_truth, batch, reduce="min")
        # Edges
        edge_truth = edge_pred_mask.eq(edge_mask) + 0  # +0 so it's not bool but 0 and 1
        edge_truth = scatter(edge_truth, edge_index[0], reduce="min")
        edge_truth = scatter(edge_truth, batch, reduce="min")

        # Count the number of zero wrong predictions across the batch
        truth = x_truth.bool() & edge_truth.bool()
        batch_perfect = (truth + 0).sum().item()

        self._per_corrects += batch_perfect
        self._num_examples += num_graphs

    @sync_all_reduce("_perfectEvent")
    def compute(self):
        """
        Final computation.
        """
        if self._num_examples == 0:
            raise NotComputableError(
                "CustomAccuracy must have at least one example before it can be computed."
            )
        return self._per_corrects / self._num_examples
