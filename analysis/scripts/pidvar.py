import pandas as pd
import numpy as np


"""
A module that adds corrections to analysis dataframe.
It adds weight variations according to the total uncertainty for easier error propagation.
"""


def get_bin_columns(weight_df) -> list:
    """
    Returns the kinematic bin columns of the dataframe.
    """
    return [col for col in weight_df.columns if col.endswith('_min') or col.endswith('_max')]


def get_binning(weight_df) -> dict:
    """
    Returns the kinematic binning of the dataframe.
    """
    columns = get_bin_columns(weight_df)
    var_names = {'_'.join(col.split('_')[:-1]) for col in columns}
    bin_dict = {}
    for var_name in var_names:
        bin_dict[var_name] = []
        for col in columns:
            if col.startswith(var_name):
                bin_dict[var_name] += list(weight_df[col].values)
        bin_dict[var_name] = np.array(sorted(set(bin_dict[var_name])))
    return bin_dict


def get_ntuple_variables(ntuple_df: pd.DataFrame,
                         bin_variables: list,
                         pid_variable_name: str,
                         mc_pdg_name: str,
                         prefix: str) -> None:
    """
    Checks if the variables are in the ntuple and returns them.
    Parameters:
        ntuple_df (pd.DataFrame): Dataframe containing the analysis ntuple.
        bin_variables (list): List of kinematic bin variables.
        pid_variable_name (str): Name of the PID variable.
        prefix (str): Prefix for the new columns.
    """
    ntuple_variables = [f'{prefix}_{var}' for var in bin_variables]
    if pid_variable_name:
        ntuple_variables += [f'{prefix}_{pid_variable_name}']
    if mc_pdg_name:
        ntuple_variables += [f'{prefix}_{mc_pdg_name}']
    for var in ntuple_variables:
        if var not in ntuple_df.columns:
            raise ValueError(f'Variable {var} not in ntuple! Required variables are {ntuple_variables}')
    return ntuple_variables


def merge_weight_tables(eff_weights_dict: dict,
                        fake_weights_dict: dict,
                        pid_variable_name: str,
                        threshold: float) -> pd.DataFrame:
    """
    Merges the efficiency and fake rate weight tables.
    Parameters:
        eff_weights_df (pd.DataFrame): Dataframe containing the efficiency weights.
        fake_weights_dict (pd.DataFrame): Dataframe containing the fake rate weights.
        mc_pdg_name (str): Name of the MC particle PDG ID.
    """
    weight_dfs = []
    for pdg in fake_weights_dict:
        fake_weights_dict[pdg]['mcPDG'] = pdg
        weight_dfs.append(fake_weights_dict[pdg].query(f'variable == "{pid_variable_name}" and threshold == {threshold}'))
    for pdg in eff_weights_dict:
        eff_weights_dict[pdg]['mcPDG'] = pdg
        weight_dfs.append(eff_weights_dict[pdg].query(f'variable == "{pid_variable_name}" and threshold == {threshold}'))
    return pd.concat(weight_dfs, ignore_index=True)


def add_weight_column(ntuple_df: pd.DataFrame,
                      weight_df: pd.DataFrame,
                      binning: dict,
                      prefix: str,
                      weight_name: str = "Weight") -> None:
    """
    Adds a weight and uncertainty columns to the dataframe.
    Parameters:
        ntuple_df (pd.DataFrame): Dataframe containing the analysis ntuple.
        weight_df (pd.DataFrame): Dataframe containing the weights.
        binning (dict): Dictionary containing the binning of the weights.
        prefix (str): Prefix for the new columns.
    """
    weight_cols = ['data_MC_ratio',
                   'data_MC_uncertainty_stat_dn',
                   'data_MC_uncertainty_stat_up',
                   'data_MC_uncertainty_sys_dn',
                   'data_MC_uncertainty_sys_up',
                   # 'data_MC_uncertainty_statsys_dn',
                   # 'data_MC_uncertainty_statsys_up',
                   # 'rel_data_MC_uncertainty_stat_dn',
                   # 'rel_data_MC_uncertainty_stat_up','rel_data_MC_uncertainty_sys_dn',
                   # 'rel_data_MC_uncertainty_sys_up','rel_data_MC_uncertainty_statsys_dn',
                   # 'rel_data_MC_uncertainty_statsys_up'
                   ]
    # Apply a weight value from the weight table to the ntuple, based on the binning
    binning_df = pd.DataFrame(index=ntuple_df.index)
    binning_df['mcPDG'] = ntuple_df[f'{prefix}_mcPDG'].abs()
    print(binning_df)
    for pdg in binning:
        for var in binning[pdg]:
            print(pdg, var)
            labels = [(binning[pdg][var][i-1], binning[pdg][var][i]) for i in range(1, len(binning[pdg][var]))]
            binning_df.loc[binning_df['mcPDG'] == pdg, var] = pd.cut(ntuple_df.query(f'abs({prefix}_mcPDG) == {pdg}')[
                                                                     f'{prefix}_{var}'], binning[pdg][var], labels=labels)
            print(binning_df.query(f'mcPDG == {pdg}')[var])
            print(ntuple_df.query(f'{prefix}_mcPDG == {pdg}')[f'{prefix}_{var}'])
            binning_df.loc[binning_df['mcPDG'] == pdg,
                           f'{var}_min'] = binning_df.loc[binning_df['mcPDG'] == pdg,
                                                          var].str[0]  # Uh oh
            binning_df.loc[binning_df['mcPDG'] == pdg,
                           f'{var}_max'] = binning_df.loc[binning_df['mcPDG'] == pdg,
                                                          var].str[1]  # Oh uff
            binning_df.drop(var, axis=1, inplace=True)
    # merge the weight table with the ntuple on binning columns
    binning_df = binning_df.merge(weight_df[weight_cols + binning_df.columns.tolist()], on=binning_df.columns.tolist(), how='left')
    for col in weight_cols:
        ntuple_df[f'{prefix}_{col}'] = binning_df[col]


def add_pid_weights(ntuple_df: pd.DataFrame,
                    eff_weights_dict: dict,
                    fake_weights_dict: dict,
                    pid_variable_name: str,
                    threshold: float,
                    prefix: str) -> None:
    """
    Adds weight variations according to the total uncertainty for easier error propagation.
    Parameters:
        ntuple_df (pd.DataFrame): Dataframe containing the analysis ntuple.
        eff_weights_dict (pd.DataFrame): Dataframe containing the efficiency weights.
        fake_weights_dict (pd.DataFrame): Dataframe containing the fake rate weights.
        pid_variable_name (str): Name of the PID variable.
        threshold (float): Threshold for the PID variable.
        prefix (str): Prefix for the new columns.
    """
    prefix = prefix.strip('_')
    merged_weight_df = merge_weight_tables(eff_weights_dict, fake_weights_dict, pid_variable_name, threshold)
    charge_dict = {'+': [0, 2], '-': [-2, 0]}
    pdg_binning = {pdg: get_binning(merged_weight_df.query(f'mcPDG == {pdg}')) for pdg in merged_weight_df['mcPDG'].unique()}
    # Check if these are legacy tables:
    if 'charge' in merged_weight_df.columns:
        merged_weight_df[['charge_min', 'charge_max']] = [charge_dict[val] for val in merged_weight_df['charge'].values]
        merged_weight_df = merged_weight_df.drop(columns=['charge'])
        for pdg in pdg_binning:
            pdg_binning[pdg]['charge'] = np.array([-2, 0, 2])
            if 'iso_score' in pdg_binning[pdg] and len(pdg_binning[pdg]['iso_score']) == 2:
                del pdg_binning[pdg]['iso_score']
    add_weight_column(ntuple_df, merged_weight_df, pdg_binning, prefix)


if __name__ == "__main__":
    import uproot
    with uproot.open("~/belle2/sutclw_b_sll_bonn/AnalysisSelections/mc15v16B0eeB0/Upsilon4SB0candee.root") as f:
        df = f["variables"].arrays(library="pd")
    pv_path = '~/belle2/pidvar/examples/'
    efftable_e = pd.read_csv(f'{pv_path}/v0b/efficiency/e_efficiency_table.csv')
    efftable_mu = pd.read_csv(f'{pv_path}/v0b/efficiency/mu_efficiency_table.csv')
    faketable_pi_e = pd.read_csv(f'{pv_path}/v0b/fakeRate/pi_e_fakeRate_table.csv')
    faketable_pi_mu = pd.read_csv(f'{pv_path}/v0b/fakeRate/pi_mu_fakeRate_table.csv')
    faketable_K_e = pd.read_csv(f'{pv_path}/v0b/fakeRate/K_e_fakeRate_table.csv')
    faketable_K_mu = pd.read_csv(f'{pv_path}/v0b/fakeRate/K_mu_fakeRate_table.csv')

    best_available_query = "is_best_available == True"
    exclude_e_bins_query = "not (theta_min == 0.56 and theta_max == 2.23) "
    exclude_e_bins_query += "and not (theta_min == 0.22 and theta_max == 2.71)"
    exclude_e_bins_query += "and not (p_min == 0.2 and p_max == 7.0)"
    exclude_mu_bins_query = "not (theta_min == 0.82 and theta_max == 2.22) "
    exclude_mu_bins_query += "and not (theta_min == 0.4 and theta_max == 0.82) "
    exclude_mu_bins_query += "and not (theta_min == 0.4 and theta_max == 2.6)"
    e_query = f"({best_available_query}) and ({exclude_e_bins_query})"
    mu_query = f"({best_available_query}) and ({exclude_mu_bins_query})"
    fake_query = f"({best_available_query}) and not (theta_min == 0.22 and theta_max == 2.71) "
    fake_query += " and not (theta_min == 0.56 and theta_max == 2.23)"
    fake_query += " and not (theta_min == 0.82 and theta_max == 2.22)"
    fake_query += " and not (theta_min == 0.4 and theta_max == 2.6)"
    fake_query += " and not (theta_min == 0.4 and theta_max == 0.82)"
    fake_query += " and not (p_min == 0.2 and p_max == 5.)"

    efftables = {11: efftable_e.query(e_query), 13: efftable_mu.query(mu_query)}
    faketables = {
        211: faketable_pi_e.query(fake_query),
        # (211,13) :faketable_pi_mu.query(fake_query),
        321: faketable_K_e.query(fake_query),
        # (321,13) :faketable_K_mu.query(fake_query)
    }
    df['lepminus_charge'] = -1
    df['lepplus_charge'] = 1
    df['lepplus_mcPDG'] = df['lepplus_MC_PDG'].fillna(0).astype(int)
    df['lepminus_mcPDG'] = df['lepminus_MC_PDG'].fillna(0).astype(int)
    add_pid_weights(df, {11: efftable_e.query(e_query)}, faketables, 'pidChargedBDTScore_e', 0.9, 'lepminus_')
