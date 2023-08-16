import torch
from torch_scatter import scatter
from ignite.metrics import Metric
from ignite.exceptions import NotComputableError
from ignite.metrics.metric import sync_all_reduce, reinit__is_reduced
from .lca2adjacency import lca2adjacency, InvalidLCAMatrix


class Efficiency(Metric, object):
    ''' Calculates the efficiency, calculated as (valid trees/total trees)

    - `update` must receive output of the form `(y_pred, y)` or `{'y_pred': y_pred, 'y': y}`.
    - `y_pred` must contain logits and has the following shape (batch_size, num_categories, ...)
    - `y` should have the following shape (batch_size, ...) and contains ground-truth class indices
        with or without the background class. During the computation, argmax of `y_pred` is taken to determine
        predicted classes.

    First the average percentage per batch is computed.
    Then the average of all the batches is returned.

    Args:
        ignore_index (int or list[int]): Class index(es) to ignore when calculating accuracy
        ignore_disconnected_leaves (bool): Whether to ignore disconnected leaves in the LCA when determining if valid
    '''

    def __init__(self, ignore_index=-1., ignore_disconnected_leaves=False, output_transform=lambda x: x, device='cpu'):

        self.ignore_index = ignore_index if isinstance(ignore_index, list) else [ignore_index]
        self.ignore_disconnected = ignore_disconnected_leaves
        self.device = device

        self._num_valid = None
        self._num_examples = None

        super(Efficiency, self).__init__(
            output_transform=output_transform, device=device
        )

    @reinit__is_reduced
    def reset(self):

        self._num_valid = 0
        self._num_examples = 0

        super(Efficiency, self).reset()

    # @reinit__is_reduced
    def _is_valid_lca(self, y_pred, y):
        ''' Remove padding from y_pred and build adjacency

        Args:
            y_pred (Tensor): (d1, d2) square matrix of predicted LCA (no batch dim)
            y (Tensor): (d1, d2) square matrix of true LCA (no batch dim)

        Returns:
            bool: True if adjacency can be built and is valid, else false
        '''
        # Check we weren't passed an empty LCA
        if y.nelement() == 0:
            return False

        # First remove padding rows/cols
        # True represents keep, False is ignore
        ignore_mask = torch.ones(y.size(), dtype=torch.bool, device=self.device)
        # Only interested in those that are not the ignore indices
        for ig_class in self.ignore_index:
            ignore_mask &= (y != ig_class)

        # Now that we have the ignore_index mask,
        # check that predicted LCA isn't trvial (i.e. all zeros)
        if not (y_pred * ignore_mask).any():
            return False

        if self.ignore_disconnected:
            # Create mask to ignore rows of disconnected leaves
            # This is done on predicted LCA since we're only concerned with ignoring
            # what it predicts are disconnected, and calculating if what's left is valid.
            # PerfectLCA will take care of which are actually correct and purity will tell
            # us the correct/valid ratio
            ignore_mask &= (y_pred != 0)

        # Ignore diagonal to be safe
        ignore_mask = ignore_mask.fill_diagonal_(False)

        get_rows = ignore_mask.any(dim=0)  # (d1,) boolean mask of non-empty rows

        bare_y_pred = y_pred[get_rows][:, get_rows]  # (d1, d2) padding rows/cols removed

        # Finally, set diagonal to zero to match expected leaf values in lca2adjacency
        bare_y_pred = bare_y_pred.fill_diagonal_(0)

        # If empty then we've probably predicted all zeros
        if bare_y_pred.numel() == 0:
            return False

        try:
            lca2adjacency(bare_y_pred)
            return True
        except InvalidLCAMatrix:
            return False
        except Exception as e:
            # Something has gone badly wrong
            raise(e)

    @reinit__is_reduced
    def update(self, output):
        ''' Computes the number of valid LCAs PER BATCH!. '''

        y_pred, y = output  # (N, C, d1, d2), (N, d1, d2), where d1 = L =leaves

        # n_leaves = int(y_pred.shape[-1]**0.5)

        # First extract most predicted LCA
        probs = torch.softmax(y_pred, dim=1)  # (N, C, d1, d2)
        winners = probs.argmax(dim=1)  # (N, d1, d2)

        # y = y.flatten(start_dim=1)  # (N, d1)
        assert winners.shape == y.shape, print(f' winners: {winners.shape}, y: {y.shape}')  #

        # mask = (y != 0).float()  # create a mask for the zeroth elements (padded entries and diagonal)

        # y_pred_mask = winners * mask  # zero the respective entries in the predictions

        # Need to loop through LCAs and check they can be built
        n_valid = sum(map(
            self._is_valid_lca,
            torch.unbind(winners),
            torch.unbind(y),
            # torch.unbind(y_pred_mask.view(y_pred_mask.shape[0], n_leaves, n_leaves)),
            # torch.unbind(y.view(y.shape[0], n_leaves, n_leaves)),
        ))

        self._num_valid += n_valid
        self._num_examples += y.shape[0]

    @sync_all_reduce("_efficiency")
    def compute(self):
        ''' Computes the average fraction of valid LCAs across all batches '''

        if self._num_examples == 0:
            raise NotComputableError(
                "CustomAccuracy must have at least one example before it can be computed."
            )
        return self._num_valid / self._num_examples


class Pad_Accuracy(Metric, object):

    """ Computes the average classification accuracy ignoring the given class (e.g. 0 for padding)

    This is almost identical to the CustomAccuracy example in https://pytorch.org/ignite/metrics.html
    except we don't ignore y_pred occurances of the ignored class.

    - `update` must receive output of the form `(y_pred, y)` or `{'y_pred': y_pred, 'y': y}`.
    - `y_pred` must contain logits and has the following shape (batch_size, num_categories, ...)
    - `y` should have the following shape (batch_size, ...) and contains ground-truth class indices
        with or without the background class. During the computation, argmax of `y_pred` is taken to determine
        predicted classes.

    First the average percentage per batch is computed.
    Then the average of all the batches is returned.

    Args:
        ignore_index (int or [int]): Class index(es) to ignore when calculating accuracy

    """

    def __init__(self, ignore_index, output_transform=lambda x: x, device='cpu'):
        self.ignore_index = ignore_index if isinstance(ignore_index, list) else [ignore_index]
        self.device = device
        self._num_correct = None
        self._num_examples = None
        super(Pad_Accuracy, self).__init__(output_transform=output_transform, device=device)

    @reinit__is_reduced
    def reset(self):
        self._num_correct = 0
        self._num_examples = 0
        super(Pad_Accuracy, self).reset()

    @reinit__is_reduced
    def update(self, output):
        y_pred, y = output  # (N, C, d1, d2), (N, d1, d2)

        indices = torch.argmax(y_pred, dim=1)

        mask = torch.ones(y.size(), dtype=torch.bool, device=self.device)  # (N, d1, d2)
        for ig_class in self.ignore_index:
            mask &= (y != ig_class)

        y = y[mask]
        indices = indices[mask]
        correct = torch.eq(indices, y).view(-1)

        self._num_correct += torch.sum(correct).item()
        self._num_examples += correct.shape[0]

    # @sync_all_reduce("_num_examples", "_num_correct")
    @sync_all_reduce("_pad_accuracy")
    def compute(self):
        if self._num_examples == 0:
            raise NotComputableError('Pad_Accuracy must have at least one example before it can be computed.')
        return self._num_correct / self._num_examples


class PerfectLCA(Metric, object):

    """ Computes the percentage of the Perfectly predicted LCAs

    - `update` must receive output of the form `(y_pred, y)` or `{'y_pred': y_pred, 'y': y}`.
    - `y_pred` must contain logits and has the following shape (batch_size, num_categories, ...)
    - `y` should have the following shape (batch_size, ...) and contains ground-truth class indices
        with or without the background class. During the computation, argmax of `y_pred` is taken to determine
        predicted classes.

    First the average percentage per batch is computed.
    Then the average of all the batches is returned.
    """

    def __init__(self, ignore_index=-1., output_transform=lambda x: x, device='cpu'):

        self.ignore_index = ignore_index if isinstance(ignore_index, list) else [ignore_index]
        self.device = device
        self._per_corrects = None
        self._num_examples = None

        super(PerfectLCA, self).__init__(
            output_transform=output_transform, device=device
        )

    @reinit__is_reduced
    def reset(self):

        self._per_corrects = 0
        self._num_examples = 0

        super(PerfectLCA, self).reset()

    ''' Computes the percentage of Perfect LCAs PER BATCH!.
    the tensors y_pred and y contain multiple LCAs that belong in a batch.
    '''

    @reinit__is_reduced
    def update(self, output):

        y_pred, y = output  # (N, C, d1, d2), (N, d1, d2)

        probs = torch.softmax(y_pred, dim=1)  # (N, C, d1, d2)
        winners = probs.argmax(dim=1)  # (N, d1, d2)

        # print(y.shape)
        assert winners.shape == y.shape

        # Create a mask for the ignored elements (padded entries and diagonal)
        mask = torch.ones(y.size(), dtype=torch.bool, device=self.device)
        for ig_class in self.ignore_index:
            mask &= (y != ig_class)
        # mask = (y != self.ignore_index).float()

        # Zero the respective entries in the predictions
        y_pred_mask = winners * mask
        y_mask = y * mask

        # Do this to make comparison across d1 and d2
        y_mask = y_mask.flatten(start_dim=1)  # (N, d1)
        y_pred_mask = y_pred_mask.flatten(start_dim=1)  # (N, d1)

        # (N) compare the masked predictions with the target. The padded and the diagonal will be equal due to masking
        truth = y_pred_mask.eq(y_mask).all(dim=1)

        # Count the number of zero wrong predictions across the batch.
        batch_perfect = truth.sum().item()

        self._per_corrects += batch_perfect
        self._num_examples += y.shape[0]

    @sync_all_reduce("_perfect")
    def compute(self):

        if self._num_examples == 0:
            raise NotComputableError(
                "CustomAccuracy must have at least one example before it can be computed."
            )
        return self._per_corrects / self._num_examples


class PerfectLCAGeometric(Metric, object):
    '''
    Computes the percentage of the Perfectly predicted LCAs
    - `update` must receive output of the form `(y_pred, y, edge_index, u_y, batch, num_graph)` or `{'y_pred': y_pred, ...}`.
    - `edge_pred` must contain logits and has the following shape (num_edges_in_batch, classes)
    - `edge_y` should have the following shape (num_edges_in_batch, 1) and contains ground-truth class indices
       with or without the background class. During the computation, argmax of `y_pred` is taken to determine predicted classes.
    - edge index is the array that maps edges to its nodes
    - 'u_y' is the signal/background class
    - batch is a vector that comes with a batch of graph and maps nodes to their graph
    - num graph is the number of graph, it could be computed here using batch
    '''

    def __init__(self, ignore_index, output_transform=lambda x: x, device='cpu', ignore_background=False):

        self.ignore_index = ignore_index if isinstance(ignore_index, list) else [ignore_index]
        self.device = device
        self.ignore_background = ignore_background
        self._per_corrects = None
        self._num_examples = None

        super(PerfectLCAGeometric, self).__init__(output_transform=output_transform, device=device)

    @reinit__is_reduced
    def reset(self):

        self._per_corrects = 0
        self._num_examples = 0

        super(PerfectLCAGeometric, self).reset()

    @reinit__is_reduced
    def update(self, output):
        edge_pred, edge_y, edge_index, u_y, batch, num_graphs = output

        num_graphs = num_graphs.item()

        probs = torch.softmax(edge_pred, dim=1)
        winners = probs.argmax(dim=1)

        # print(y.shape)
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

        # Ignore background events
        ignored_num = torch.logical_and((u_y == 0), (truth == 1)).sum().item() if self.ignore_background else 0
        ignored_den = (u_y == 0).sum().item() if self.ignore_background else 0

        self._per_corrects += (batch_perfect - ignored_num)
        self._num_examples += (num_graphs - ignored_den)

    @sync_all_reduce("_perfectLCA")
    def compute(self):

        if self._num_examples == 0:
            raise NotComputableError(
                "CustomAccuracy must have at least one example before it can be computed."
            )
        return self._per_corrects / self._num_examples


class PerfectMasses(Metric, object):
    '''
    Computes the rate of events with perfectly predicted masses of FSP
    - 'update' must receive output of the form `(x_pred, x, u_y, batch, num_graph)` or `{'x_pred': x_pred, 'x': x, ...}`.
    - 'x_pred' must contain logits and has the following shape (num_nodes_in_batch, classes)
    - 'x_y' should have the following shape (num_nodes_in_batch, 1) and contains ground-truth class indices
       with or without the background class. During the computation, argmax of `x_pred` is taken to determine predicted classes.
    - 'u_y' is the signal/background class
    - batch is a vector that comes with a batch of graph and maps nodes to their graph
    - num graph is the number of graph, it could be computed here using batch
    '''

    def __init__(self, ignore_index, output_transform=lambda x: x, device='cpu', ignore_background=False):

        self.ignore_index = ignore_index if isinstance(ignore_index, list) else [ignore_index]
        self.device = device
        self.ignore_background = ignore_background
        self._per_corrects = None
        self._num_examples = None

        super(PerfectMasses, self).__init__(output_transform=output_transform, device=device)

    @reinit__is_reduced
    def reset(self):

        self._per_corrects = 0
        self._num_examples = 0

        super(PerfectMasses, self).reset()

    @reinit__is_reduced
    def update(self, output):
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

        # Ignore background events
        ignored_num = torch.logical_and((u_y == 0), (truth == 1)).sum().item() if self.ignore_background else 0
        ignored_den = (u_y == 0).sum().item() if self.ignore_background else 0

        self._per_corrects += (batch_perfect - ignored_num)
        self._num_examples += (num_graphs - ignored_den)

    @sync_all_reduce("_perfectMasses")
    def compute(self):

        if self._num_examples == 0:
            raise NotComputableError(
                "CustomAccuracy must have at least one example before it can be computed."
            )
        return self._per_corrects / self._num_examples


class PerfectEvent(Metric, object):
    '''
    Computes the rate of events with masses and LCA correctly predicted
    - 'update' must receive output of the form `(x_pred, x_y, y_pred, y, edge_index, u_y, batch, num_graph)`
      or `{'x_pred': x_pred, ...}`
    - 'x_pred' must contain logits and has the following shape (num_nodes_in_batch, classes)
    - 'x_y' should have the following shape (num_nodes_in_batch, 1) and contains ground-truth class indices
    - 'edge_pred' must contain logits and has the following shape (num_edges_in_batch, classes)
    - 'edge_y' should have the following shape (num_edges_in_batch, 1) and contains ground-truth class indices
       with or without the background class. During the computation, argmax of `y_pred` is taken to determine predicted classes.
    - edge index is the array that maps edges to its nodes
    - 'u_y' is the signal/background class
    - batch is a vector that comes with a batch of graph and maps nodes to their graph
    - num graph is the number of graph, it could be computed here using batch
    '''

    def __init__(self, ignore_index, output_transform=lambda x: x, device='cpu', ignore_background=False):

        self.ignore_index = ignore_index if isinstance(ignore_index, list) else [ignore_index]
        self.device = device
        self.ignore_background = ignore_background
        self._per_corrects = None
        self._num_examples = None

        super(PerfectEvent, self).__init__(output_transform=output_transform, device=device)

    @reinit__is_reduced
    def reset(self):

        self._per_corrects = 0
        self._num_examples = 0

        super(PerfectEvent, self).reset()

    @reinit__is_reduced
    def update(self, output):
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

        # Ignore background events
        ignored_num = torch.logical_and((u_y == 0), (truth == 1)).sum().item() if self.ignore_background else 0
        ignored_den = (u_y == 0).sum().item() if self.ignore_background else 0

        self._per_corrects += (batch_perfect - ignored_num)
        self._num_examples += (num_graphs - ignored_den)

    @sync_all_reduce("_perfectEvent")
    def compute(self):

        if self._num_examples == 0:
            raise NotComputableError(
                "CustomAccuracy must have at least one example before it can be computed."
            )
        return self._per_corrects / self._num_examples


class IsTrueB(Metric, object):
    '''
    Computes the percentage of correctly identified B's
    - `update` must receive output of the form `(u_pred, u_y, batch, num_graph)` or `{'u_pred': u_pred, 'u_y': u_y , ...}`.
    - `u_pred` must contain logits and has shape (num_graph, 1)
    - `u` contains ground-truth class indices and has same shape as u_pred
    - num graph is the number of graph, it could be computed here using batch
    '''

    def __init__(self, ignore_index, output_transform=lambda x: x, device='cpu'):

        self.ignore_index = ignore_index if isinstance(ignore_index, list) else [ignore_index]
        self.device = device
        self._per_corrects = None
        self._num_examples = None

        super(IsTrueB, self).__init__(output_transform=output_transform, device=device)

    @reinit__is_reduced
    def reset(self):

        self._per_corrects = 0
        self._num_examples = 0

        super(IsTrueB, self).reset()

    @reinit__is_reduced
    def update(self, output):
        u_pred, u_y, num_graphs = output

        num_graphs = num_graphs.item()

        u_pred = (torch.sigmoid(u_pred) > 0.5).double()  # If element has probability > 0.5 it's signal, else background

        assert u_y.shape == u_pred.shape, 'Predictions shape does not match target shape'

        # Create a mask for the padded entries
        mask = torch.ones(u_y.size(), dtype=torch.long, device=self.device)
        for ig_class in self.ignore_index:
            mask &= (u_y != ig_class)

        # Zero the respective entries in the predictions
        u_pred_mask = u_pred * mask
        u_mask = u_y * mask

        # Count the number of zero wrong predictions across the batch
        good_predictions = (u_pred_mask == u_mask).sum().item()

        self._per_corrects += good_predictions
        self._num_examples += num_graphs

    @sync_all_reduce("_isTrueB")
    def compute(self):

        if self._num_examples == 0:
            raise NotComputableError(
                "CustomAccuracy must have at least one example before it can be computed."
            )
        return self._per_corrects / self._num_examples
