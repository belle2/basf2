import torch
from particle import Particle, ParticleNotFound


class GParticle:
    """
    Generated particle.
    By Giulio Dujany and Lucas Martel.
    """

    def __init__(self, mcPDG):
        self.mcPDG = mcPDG
        try:
            self.particle = Particle.from_pdgid(mcPDG)
            self.name = self.particle.name
            self.latex_name = self.particle.latex_name
        except ParticleNotFound:
            if mcPDG == -20423:
                self.name = "D1(2430)~0"
                self.latex_name = "\\overline{D}_1(2430)^0"
            elif mcPDG == 39:
                self.name = ""
                self.latex_name = ""
            else:
                self.name = str(int(mcPDG))
                self.latex_name = str(int(mcPDG))
        self.daughters = []

    @property
    def has_daughters(self):
        return len(self.daughters) > 0

    def __repr__(self):
        return self.name

    def __str__(self):
        return rf"${self.latex_name}$"

    def __getitem__(self, key):
        try:
            return self.daughters[key]
        except TypeError:
            if len(key) == 1:
                return self[key[0]]
            else:
                return self[key[0]][key[1:]]

    def add_daughter(self, mcPDG):
        try:
            self.daughters.append(GParticle(mcPDG))
        except ValueError:
            pass

    def add_descendent(self, index, mcPDG):
        try:
            self[index].add_daughter(mcPDG)
        except IndexError:
            pass

    def decay(self):
        if self.has_daughters:
            return "({0} --> {1})".format(
                self.latex_name, " ".join(i.decay() for i in self.daughters)
            )
        else:
            return self.latex_name


def default_collate_fn(batch):
    input, target = list(zip(*batch))
    input = torch.stack(input)
    target = torch.stack(target)
    input = input.transpose(0, 1)
    return input, target


def pad_collate_fn(batch):
    """Collate function for batches with varying sized inputs

    This pads the batch with zeros to the size of the large sample in the batch

    Args:
        batch(tuple):  batch contains a list of tuples of structure (sequence, target)
    Return:
        (tuple): Input, labels, mask, all padded
    """
    # First pad the input data
    data = [item[0] for item in batch]
    # Here we pad with 0 as it's the input, so need to indicate that the network ignores it
    data = torch.nn.utils.rnn.pad_sequence(
        data, batch_first=True, padding_value=0.0
    )  # (N, L, F)
    data = data.transpose(0, 1)  # (L, N, F)
    # Then the labels
    labels = [item[1] for item in batch]

    # Note the -1 padding, this is where we tell the loss to ignore the outputs in those cells
    target = (
        torch.zeros(data.shape[1], data.shape[0], data.shape[0], dtype=torch.long) - 1
    )  # (N, L, L)
    # mask = torch.zeros(data.shape[0], data.shape[1], data.shape[1])  # (N, L, L)

    # I don't know a cleaner way to do this, just copying data into the fixed-sized tensor
    for i, tensor in enumerate(labels):
        length = tensor.size(0)
        target[i, :length, :length] = tensor
        # mask[i, :length, :length] = 1

    return data, target  # mask


def rel_pad_collate_fn(batch, self_interaction=False):
    """Collate function for batches with varying sized inputs

    This pads the batch with zeros to the size of the large sample in the batch

    Args:
        batch(tuple):  batch contains a list of tuples of structure (sequence, target)
    Return:
        (tuple): Input, labels, rel_rec, rel_send, all padded
    """
    lens = [sample[0].size(0) for sample in batch]

    data, target = pad_collate_fn(batch)

    rel_recvs = construct_rel_recvs(lens, self_interaction=self_interaction)
    rel_sends = construct_rel_sends(lens, self_interaction=self_interaction)

    return (data, rel_recvs, rel_sends), target


def construct_rel_recvs(ln_leaves, self_interaction=False, device=None):
    """
    ln_leaves: list of ints, number of leaves for each sample in the batch
    """
    pad_len = max(ln_leaves)
    rel_recvs = []
    for ln_leaf in ln_leaves:
        rel_recv = torch.eye(pad_len, device=device)  # (l, l)
        rel_recv[:, ln_leaf:] = 0
        rel_recv = rel_recv.repeat_interleave(pad_len, dim=1).T  # (l*l, l)
        for j in range(ln_leaf, pad_len):  # remove padding vertex edges TODO optimize
            rel_recv[j::pad_len] = 0

        if not self_interaction:
            rel_recv[0:: pad_len + 1] = 0

        rel_recvs.append(rel_recv)

    return torch.stack(rel_recvs)


def construct_rel_sends(ln_leaves, self_interaction=False, device=None):
    """
    ln_leaves: list of ints, number of leaves for each sample in the batch
    """
    pad_len = max(ln_leaves)
    rel_sends = []
    for ln_leave in ln_leaves:
        rel_send = torch.eye(pad_len, device=device).repeat(pad_len, 1)
        if not self_interaction:
            rel_send[torch.arange(0, pad_len * pad_len, pad_len + 1)] = 0
            # rel_send = rel_send[rel_send.sum(dim=1) > 0]  # (l*l, l)

        # padding
        rel_send[:, ln_leave:] = 0
        rel_send[ln_leave * (pad_len):] = 0
        rel_sends.append(rel_send)
    return torch.stack(rel_sends)


def calculate_class_weights(dataloader, num_classes, edges, num_batches=100):
    """Calculates class weights based on num_batches of the dataloader

    This assumes there exists a -1 padding value that is not part of the class weights.
    Any classes not found will have a weight of one set

    Args:
        dataloader(torch.Dataloader): Dataloader to iterate through when collecting batches
        edge_classes(int): Number of classes
        num_batches(int, optional): Number of batches from dataloader to use to approximate class weights
        amp_enabled(bool, optional): Enabled mixed precision. Creates weights tensor as half precision
    Return:
        (torch.tensor): Tensor of class weights, normalised to 1
    """
    assert isinstance(
        edges, bool
    ), "class_type should be True for edges and False for nodes"
    weights = torch.zeros((num_classes,))
    for _, batch in zip(range(num_batches), dataloader):
        index, count = torch.unique(
            batch.edge_y if edges else batch.x_y, sorted=True, return_counts=True
        )
        # TODO: add padding value as input to specifically ignore
        if -1 in index:
            # This line here assumes that the lowest class found is -1 (padding) which should be ignored
            weights[index[1:]] += count[1:]
        else:
            weights[index] += count

    # The weights need to be the invers, since we scale down the most common classes
    weights = 1 / weights
    # Set inf to 1
    weights = torch.nan_to_num(weights, posinf=float("nan"))
    # And normalise to sum to 1
    weights = weights / weights.nansum()
    # Finally, assign default value to any that were missing during calculation time
    weights = torch.nan_to_num(weights, nan=1)

    return weights
