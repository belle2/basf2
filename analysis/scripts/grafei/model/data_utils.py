import torch


def calculate_class_weights(dataloader, num_classes, edges, num_batches=100):
    """
    Calculates class weights based on num_batches of the dataloader.

    This assumes there exists a -1 padding value that is not part of the class weights.
    Any classes not found will have a weight of one.

    :param dataloader: PyTorch Geometric Dataloader.
    :type dataloader: `DataLoader
        <https://pytorch-geometric.readthedocs.io/en/latest/modules/loader.html#torch_geometric.loader.DataLoader>`_
    :param num_classes: Number of classes.
    :type num_classes: int
    :param edges: True for edges False for nodes.
    :type edges: bool
    :param num_batches: Number of batches from dataloader to use to approximate class weights.
    :type num_batches: int

    :return: Tensor of class weights, normalised to 1.
    :rtype: `Tensor <https://pytorch.org/docs/stable/tensors.html#torch.Tensor>`_

    .. note:: Class weights do not improve the performances, left for experimenting.
    """
    assert isinstance(edges, bool), "class_type should be True for edges and False for nodes"

    weights = torch.zeros((num_classes,))
    for _, batch in zip(range(num_batches), dataloader):
        index, count = torch.unique(
            batch.edge_y if edges else batch.x_y, sorted=True, return_counts=True
        )
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
