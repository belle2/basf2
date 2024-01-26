##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import re
import random
import string
import awkward as ak
import numpy as np
import pandas as pd
from collections import defaultdict

from smartBKG import PREPROC_CONFIG, TOKENIZE_DICT, LIST_FIELDS


def check_status_bit(status_bit):
    """
    Check whether the corresponding particle is usable according to its status_bit,
    which means not virtual, not initial, not ISR or FSR photon.

    Arguments:
        status_bit (short int): 1-based index of particle showing its status.
        More details in `mdst/dataobjects/include/MCParticle.h`

    Returns:
        bool: Whether conditions are satisfied (not an unusable particle).
    """
    return (
        (status_bit & 1 << 4 == 0) &  # IsVirtual
        (status_bit & 1 << 5 == 0) &  # Initial
        (status_bit & 1 << 6 == 0) &  # ISRPhoton
        (status_bit & 1 << 7 == 0)  # FSRPhoton
    )


def load_particle_list(mcplist, **meta_kwargs):
    """
    Collect variables from MC particle list.

    Arguments:
        mcplist (Belle2.PyStoreArray("MCParticles")): MC particle list in belle2 data store.
        meta_kwargs: extra event level variables that will be copied through the particle list.

    Returns:
        pandas dataframe: particle list containing all the necessary information.
    """
    particle_dict = defaultdict(list)
    root_prodTime = defaultdict(list)
    # Create particle features
    for mcp in mcplist:
        prodTime = mcp.getProductionTime()
        # Collect indices for graph building
        arrayIndex = mcp.getArrayIndex()
        mother = mcp.getMother()
        if mother:
            motherArrayIndex = mother.getArrayIndex()
            # pass the production time of root particle for the correction of jitter
            root_prodTime[arrayIndex] = root_prodTime[motherArrayIndex]
            if mother.isVirtual():
                motherArrayIndex = arrayIndex
        else:
            motherArrayIndex = arrayIndex
            # record the production time of root particle for the correction of jitter
            root_prodTime[arrayIndex] = prodTime

        if mcp.isPrimaryParticle() and check_status_bit(mcp.getStatus()):
            four_vec = mcp.get4Vector()
            prod_vec = mcp.getProductionVertex()
            # indices
            particle_dict['arrayIndex'].append(arrayIndex)
            # features
            particle_dict['PDG'].append(mcp.getPDG())
            particle_dict['mass'].append(mcp.getMass())
            particle_dict['charge'].append(mcp.getCharge())
            particle_dict['energy'].append(mcp.getEnergy())
            particle_dict['prodTime'].append(prodTime-root_prodTime[arrayIndex])
            particle_dict['x'].append(prod_vec.x())
            particle_dict['y'].append(prod_vec.y())
            particle_dict['z'].append(prod_vec.z())
            particle_dict['px'].append(four_vec.Px())
            particle_dict['py'].append(four_vec.Py())
            particle_dict['pz'].append(four_vec.Pz())
            particle_dict['motherIndex'].append(motherArrayIndex)
            particle_dict.update(meta_kwargs)
    return pd.DataFrame(particle_dict)


def ak_from_df(
    df,
    decorr_df=None,
    columns=None,
    missing_values=False,
    convert_to_32=True,
):
    """
    Load pandas data frame stored in parquet into an awkward array

    Particle-level quantities will be lists of variable length, the other
    variables are assumed to be event-level. Grouping will be done based on the
    `evtNum` column.

    Arguments:
        df (pandas dataframe): particle-level information.
        decorr_df (pandas dataframe): event-level information.
        columns (list): read only the listed columns (None for all) - passed to `ak.from_parquet`
        missing_values (bool): if False, assume there are no missing values in the particle
            lists and drop the masks. For the event-level quantities, replace missing
            values with nan. Avoiding option types might speedup the subsequent processing.
        convert_to_32 (bool): convert int64 to int32 and float64 to float32

    Returns:
        Awkward array with particle quantities as Lists and event-level quantities as flat arrays

    Note:
        Example:
        >>> import io
        >>> import pandas as pd
        >>> df = pd.DataFrame({
        ... "x": [1, 2, 3, 4],
        ... "y": [5, 6, 7, 8],
        ... "label": [True, True, False, False],
        ... "evtNum": [0, 0, 1, 1],
        ... })
        >>> f = io.BytesIO()
        >>> df.to_parquet(f)
        >>> ak_array = ak_from_parquet_df(f)
        >>> ak_array.particles.x
        <Array [[1, 2], [3, 4]] type='2 * var * int32'>
        >>> ak_array.label
        <Array [True, False] type='2 * bool'>
    """
    # to group by events (unflatten), we need to find the particle count for each event
    evt_nums = df.evtNum.values
    df.set_index('evtNum')
    if decorr_df is not None:
        decorr_df = decorr_df.set_index('evtNum')
        df = df.join(other=decorr_df, on='evtNum', how='inner')
    df = df.reset_index().set_index(['label', 'evtNum', 'arrayIndex'])
    unique, indices, counts = np.unique(evt_nums, return_index=True, return_counts=True)
    # reverse the sorting that np.unique did
    counts = counts[np.argsort(indices)]
    ak_array = ak.unflatten(ak.Array(df.to_records()), counts)

    out = {"particles": {}}
    for field in ak_array.fields:
        if field not in LIST_FIELDS:
            # for event-level quantities we assume that the first entry is equal to all entries
            out[field] = ak_array[field][:, 0]
            if convert_to_32:
                out[field] = values_as_32(out[field])
        else:
            # particle-level quantities stay lists
            out["particles"][field] = ak_array[field]
            if convert_to_32:
                out["particles"][field] = values_as_32(out["particles"][field])
    out["particles"] = ak.zip(out["particles"])
    out = ak.zip(out, depth_limit=1)

    if not missing_values:
        out = remove_masks(out)

    return out


def values_as_32(array):
    """
    Convert int64 to int32 and float64 to float32 in the given array for the processing in Pytorch.

    Arguments:
        array (awkward array): any.

    Returns:
        awkward array: the converted array.
    """
    ak_type = ak.type(array.layout)
    while not isinstance(ak_type, ak.types.PrimitiveType):
        ak_type = ak_type.type
    dtype = ak_type.dtype
    if dtype == "int64":
        return ak.values_astype(array, np.int32)
    if dtype == "float64":
        return ak.values_astype(array, np.float32)
    return array


def remove_masks(array):
    """
    Drop masks for particle-level quantities and replace missing values by nan for event-level quantities

    Arguments:
        array (awkward array): any.

    Returns:
        awkward array: the processed array.
    """
    out = array[:]
    for field in out.fields:
        if field == "particles":
            continue
        out[field] = ak.fill_none(out[field], np.nan)
    for field in out.particles.fields:
        masked = out.particles[field].layout
        if not isinstance(masked, ak.layout.ListOffsetArray64):
            raise TypeError(
                "Wrong type - this method only works with ListOffsetArray for the particle fields"
            )
        out["particles", field] = ak.Array(
            # explicitly construct a ListOffsetArray (without the mask)
            ak.layout.ListOffsetArray64(masked.offsets, masked.content)
        )
    return out


def mapped_mother_index_flat(array_indices_flat, mother_indices_flat, total, sizes, dict_size):
    """
    Map mother indices for particle arrays to handle removed mothers.

    Arguments:
        array_indices_flat (array): flat array indices of the retained particles from MC particle list.
        mother_indices_flat (array): flat array indices of the mother particles of the retained particles.
        total (int): total number of particles in all the events.
        sizes (array): numbers of particles in each event.
        dict_size (int): maximum number of different indices.

    Returns:
        array: flat mother indices after correction.
    """
    out = np.empty(total, dtype=np.int32)
    i = 0
    idx_dict = np.empty(dict_size, dtype=np.int32)
    start = 0
    for size in sizes:
        stop = start + size
        array_indices = array_indices_flat[start:stop]
        mother_indices = mother_indices_flat[start:stop]
        # fill idx_dict
        for original_index in mother_indices:
            # default -1 (will represent mothers that have been removed)
            idx_dict[original_index] = -1
        for mapped_index, original_index in enumerate(array_indices):
            # indices of still existing mothers
            idx_dict[original_index] = mapped_index
        # remap
        for mother_index in mother_indices:
            out[i] = idx_dict[mother_index]
            i += 1
        start = stop
    return out


def mapped_mother_index(array_indices, mother_indices):
    """
    Map mother indices for particle arrays to handle removed mothers for awkward arrays.

    Arguments:
        array_indices (awkward array): array indices of the retained particles from MC particle list.
        mother_indices (awkward array): array indices of the mother particles of the retained particles.

    Returns:
        awkward array: mother indices after correction.
    """
    max_dict_index = max(ak.max(array_indices), ak.max(mother_indices))
    dict_size = max_dict_index + 1
    flat = mapped_mother_index_flat(
        ak.to_numpy(ak.flatten(ak.fill_none(array_indices, -1))),
        ak.to_numpy(ak.flatten(ak.fill_none(mother_indices, -1))),
        sizes=ak.num(array_indices),
        total=ak.sum(ak.num(array_indices)),
        dict_size=dict_size,
    )
    return ak.unflatten(flat, ak.num(array_indices))


def map_np(array, mapping):
    """
    Map PDG IDs to tokens.

    Arguments:
        pdg (array): PDG IDs.

    Returns:
        array: array after PDG ID mapping.
    """
    unique, inv = np.unique(array, return_inverse=True)
    np_mapping = np.array([mapping[x] for x in unique])
    return np_mapping[inv]


def mapped_pdg_id(pdg):
    """
    Map PDG IDs to tokens for awkward arrays.

    Arguments:
        pdg (awkward array): PDG IDs.

    Returns:
        awkward array: awkward array after PDG ID mapping.
    """
    return ak.unflatten(
        map_np(ak.to_numpy(ak.flatten(pdg)), TOKENIZE_DICT), ak.num(pdg)
    )


def evaluate_query(array, query):
    """
    Evaluate a query on the awkward array, pd.DataFrame.evaluate - style
    Can also pass a callable that takes an awkward array and returns an awkward array mask

    Arguments:
        array (awkward array or dataframe): any.
        query (str): queries for particle selection.

    Returns:
        awkward array: awkward array after particle selection.
    """
    if callable(query):
        return query(array)

    # merge event-level and particle-level quantities
    # such that queries can directly access both
    # e.g. "x > 5" instead of "particles.x > 5"
    array_dict = {
        **dict(zip(array.fields, ak.unzip(array))),
        **dict(zip(array.particles.fields, ak.unzip(array.particles)))
    }

    # replace quoted fieldnames - e.g `nParticlesInList(B0:feiHadronic)` by random strings
    # such that ak.numexpr.evaluate can handle them
    joined_query = " & ".join(f"({q})" for q in query)
    quoted_fieldnames = set(re.findall("`[^`]*`", joined_query))
    name_mapping = {k: "".join(random.choices(string.ascii_lowercase, k=20)) for k in quoted_fieldnames}
    quoted_fields = {name_mapping[field]: array_dict[field.replace("`", "")] for field in quoted_fieldnames}
    for field, rnd_name in name_mapping.items():
        joined_query = joined_query.replace(field, rnd_name)

    return ak.numexpr.evaluate(joined_query, local_dict={**array_dict, **quoted_fields})


def preprocessed(df, decorr_df=None, particle_selection=PREPROC_CONFIG['cuts']):
    """
    Preprocess the input dataframe and return an awkward array that is ready for graph building.

    Arguments:
        df (pandas dataframe): containing particle-level information.
        decorr_df (pandas dataframe): containing event-level information.
        particle_selection (str): queries for particle selection.

    Returns:
        awkward array: awkward array after preprocessing.
    """
    array = ak_from_df(df, decorr_df)[:]
    array["particles"] = array.particles[evaluate_query(array, particle_selection)]
    array["particles", "PDG"] = mapped_pdg_id(array.particles.PDG)
    array["particles", "motherIndex"] = mapped_mother_index(
        array.particles.arrayIndex,
        array.particles.motherIndex,
    )
    return array
