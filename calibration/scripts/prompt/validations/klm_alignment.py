##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# KLM alignment validation

from prompt import ValidationSettings
import sys
import os
import basf2
from ROOT.Belle2 import KLMCalibrationChecker
import uproot
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages

#: Tells the automated system some details of this script
settings = ValidationSettings(name='KLM alignment',
                              description=__doc__,
                              download_files=['stdout'],
                              expert_config=None)


def get_result(job_path, tmp_dir):
    database_file = f'{job_path}/outputdb/database.txt'
    exp_run_list = []
    with open(database_file) as f:
        for line in f:
            fields = line.split(' ')
            if (fields[0] == 'dbstore/BKLMAlignment'):
                iov = fields[2].split(',')
                exp_run_list.append([int(iov[0]), int(iov[1])])

    for exp_run in exp_run_list:
        exp = exp_run[0]
        run = exp_run[1]
        checker = KLMCalibrationChecker()
        checker.setExperimentRun(exp, run)
        checker.setTestingPayload(database_file)
        basf2.B2INFO(f'Creating alignment results tree for experiment {exp}, run {run}.')
        checker.setAlignmentResultsFile(tmp_dir+f'/alignment_{exp}_{run}.root')
        checker.checkAlignment()

    return exp_run_list


def get_residuals(data_path, data_path_prev):
    # convert the alignment results to numpy.ndarray and calculate the residuals
    data_prev = uproot.open(data_path_prev)
    data = uproot.open(data_path)
    BKLMModule_prev = data_prev[data_prev.keys()[0]]
    EKLMModule_prev = data_prev[data_prev.keys()[1]]
    BKLMModule = data[data.keys()[0]]
    EKLMModule = data[data.keys()[1]]
    EKLM = [EKLMModule_prev.arrays(library='pd'), EKLMModule.arrays(library='pd')]
    BKLM = [BKLMModule_prev.arrays(library='pd'), BKLMModule.arrays(library='pd')]

    # Fill numpy.ndarray with the alignment results
    EKLM_values = np.zeros((2, len(EKLM[0]['section'].unique()),
                            len(EKLM[0]['sector'].unique()),
                            3, len(EKLM[0]['layer'].unique())))
    EKLM_errors = np.zeros((2, len(EKLM[0]['section'].unique()),
                            len(EKLM[0]['sector'].unique()),
                            3, len(EKLM[0]['layer'].unique())))

    BKLM_values = np.zeros((2, len(BKLM[0]['section'].unique()),
                            len(BKLM[0]['sector'].unique()),
                            3, len(BKLM[0]['layer'].unique())))
    BKLM_errors = np.zeros((2, len(BKLM[0]['section'].unique()),
                            len(BKLM[0]['sector'].unique()),
                            3, len(BKLM[0]['layer'].unique())))

    pars = {1: 1, 2: 2, 3: 6}

    for i in [0, 1]:
        for section in EKLM[i]['section'].unique():
            for sector in EKLM[i]['sector'].unique():
                for param in range(0, len(pars)):
                    req = (
                        (EKLM[i]['section'] == section) & (
                            EKLM[i]['sector'] == sector) & (
                            EKLM[i]['param'] == list(
                                pars.values())[param]))
                    if (section == 1):
                        EKLM_values[i][section-1][sector-1][list(pars.keys())[param] -
                                                            1] = np.append(np.array(EKLM[i][req]['value']), [0, 0])
                        EKLM_errors[i][section-1][sector-1][list(pars.keys())[param] -
                                                            1] = np.append(np.array(EKLM[i][req]['error']), [0, 0])
                    else:
                        EKLM_values[i][section-1][sector-1][list(pars.keys())[param]-1] = np.array(EKLM[i][req]['value'])
                        EKLM_errors[i][section-1][sector-1][list(pars.keys())[param]-1] = np.array(EKLM[i][req]['error'])

    for i in [0, 1]:
        for section in BKLM[i]['section'].unique():
            for sector in BKLM[i]['sector'].unique():
                for param in range(0, len(pars)):
                    req = (
                        (BKLM[i]['section'] == section) & (
                            BKLM[i]['sector'] == sector) & (
                            BKLM[i]['param'] == list(
                                pars.values())[param]))
                    BKLM_values[i][section][sector-1][list(pars.keys())[param]-1] = np.array(BKLM[i][req]['value'])
                    BKLM_errors[i][section][sector-1][list(pars.keys())[param]-1] = np.array(BKLM[i][req]['error'])

    # Calculate the residuals
    EKLM_res_values = np.zeros((len(EKLM[0]['section'].unique()),
                                len(EKLM[0]['sector'].unique()),
                                3, len(EKLM[0]['layer'].unique())))
    EKLM_res_errors = np.zeros((len(EKLM[0]['section'].unique()),
                                len(EKLM[0]['sector'].unique()),
                                3, len(EKLM[0]['layer'].unique())))

    BKLM_res_values = np.zeros((len(BKLM[0]['section'].unique()),
                                len(BKLM[0]['sector'].unique()),
                                3, len(BKLM[0]['layer'].unique())))
    BKLM_res_errors = np.zeros((len(BKLM[0]['section'].unique()),
                                len(BKLM[0]['sector'].unique()),
                                3, len(BKLM[0]['layer'].unique())))

    for section in range(0, EKLM_values[0].shape[0]):
        for sector in range(0, EKLM_values[0].shape[1]):
            for param in range(0, EKLM_values[0].shape[2]):
                EKLM_res_values[section][sector][param] = EKLM_values[1][section][sector][param] - \
                    EKLM_values[0][section][sector][param]
                EKLM_res_errors[section][sector][param] = np.sqrt(
                    EKLM_errors[1][section][sector][param]**2 +
                    EKLM_errors[0][section][sector][param]**2)

    for section in range(0, BKLM_values[0].shape[0]):
        for sector in range(0, BKLM_values[0].shape[1]):
            for param in range(0, BKLM_values[0].shape[2]):
                BKLM_res_values[section][sector][param] = BKLM_values[1][section][sector][param] - \
                    BKLM_values[0][section][sector][param]
                BKLM_res_errors[section][sector][param] = np.sqrt(
                    BKLM_errors[1][section][sector][param]**2 +
                    BKLM_errors[0][section][sector][param]**2)

    EKLM_chi2 = np.zeros((len(EKLM[0]['section'].unique()),
                         len(EKLM[0]['sector'].unique()),
                          3, len(EKLM[0]['layer'].unique())))

    BKLM_chi2 = np.zeros((len(BKLM[0]['section'].unique()),
                         len(BKLM[0]['sector'].unique()),
                         3, len(BKLM[0]['layer'].unique())))

    for section in range(0, EKLM_res_values.shape[0]):
        for sector in range(0, EKLM_res_values.shape[1]):
            for param in range(0, EKLM_res_values.shape[2]):
                for layer in range(0, EKLM_res_values.shape[3]):
                    if ((EKLM_res_values[section][sector][param][layer] == 0) |
                            (EKLM_res_errors[section][sector][param][layer] == 0)):
                        EKLM_chi2[section][sector][param][layer] = 0
                    else:
                        EKLM_chi2[section][sector][param][layer] = (
                            EKLM_res_values[section][sector][param][layer]**2)/(EKLM_res_errors[section][sector][param][layer]**2)

    for section in range(0, BKLM_res_values.shape[0]):
        for sector in range(0, BKLM_res_values.shape[1]):
            for param in range(0, BKLM_res_values.shape[2]):
                for layer in range(0, BKLM_res_values.shape[3]):
                    if ((BKLM_res_values[section][sector][param][layer] == 0) |
                            (BKLM_res_errors[section][sector][param][layer] == 0)):
                        BKLM_chi2[section][sector][param][layer] = 0
                    else:
                        BKLM_chi2[section][sector][param][layer] = (
                            BKLM_res_values[section][sector][param][layer]**2)/(BKLM_res_errors[section][sector][param][layer]**2)

    return [EKLM_res_values, EKLM_res_errors, EKLM_chi2, BKLM_res_values, BKLM_res_errors, BKLM_chi2]


def draw_EKLM_pics(EKLM_values, EKLM_errors, EKLM_chi2, pdfPages):
    # Draw the EKLM alignment residuals and add them to a .pdf file
    plt.rcParams.update({
        'font.size': 20,
        'figure.figsize': (11, 10),
        'axes.grid': True,
        'grid.linestyle': '-',
        'grid.alpha': 0.2,
        'lines.markersize': 5.0,
        'xtick.minor.visible': True,
        'xtick.direction': 'in',
        'xtick.major.size': 20.0,
        'xtick.minor.size': 10.0,
        'xtick.top': True,
        'ytick.minor.visible': True,
        'ytick.direction': 'in',
        'ytick.major.size': 20.0,
        'ytick.minor.size': 10.0,
        'ytick.right': True,
        'errorbar.capsize': 0.0,
    })
    param_meaning = {0: 'x', 1: 'y', 2: r'$\alpha$'}
    section_meaning = {0: 'b', 1: 'f'}
    layers = {'EKLM': np.arange(1, 15, 1), 'BKLM': np.arange(1, 16, 1)}
    layers_err = {'EKLM': np.full(14, 0.5), 'BKLM': np.full(15, 0.5)}
    for section in [0, 1]:
        fig, axs = plt.subplots(4, 3, figsize=(20, 20))
        for i in range(0, 12):
            sector = i//3
            param = i % 3
            plt.sca(axs[sector][param])
            plt.errorbar(
                x=layers['EKLM'],
                xerr=layers_err['EKLM'],
                y=EKLM_values[section][sector][param],
                yerr=EKLM_errors[section][sector][param],
                ls='',
                fmt='o',
                ds='steps-mid',
                color='black',
                label='EKLM ' +
                section_meaning[section] +
                ' ' +
                str(sector) +
                r' $\chi^{2}$=' +
                str(
                    np.around(
                        np.sum(
                            EKLM_chi2,
                            axis=3)[section][sector][param],
                        1)))
            plt.hlines(0, 0, 14, color='red')
            if (param == 2):
                plt.ylim(-0.02, 0.02)
                plt.ylabel(r'$\Delta$'+param_meaning[param]+' rad')
            else:
                plt.ylim(-2, 2)
                plt.ylabel(r'$\Delta$'+param_meaning[param]+' cm')
            plt.xlabel('Layer')
            axs[sector][param].yaxis.set_label_coords(-0.1, 0.5)
            plt.legend()
        fig.tight_layout()
        plt.savefig(pdfPages, format='pdf')
    plt.close('all')


def draw_BKLM_pics(BKLM_values, BKLM_errors, BKLM_chi2, pdfPages):
    # Draw the BKLM alignment residuals and add them to a .pdf file
    plt.rcParams.update({
        'font.size': 20,
        'figure.figsize': (11, 10),
        'axes.grid': True,
        'grid.linestyle': '-',
        'grid.alpha': 0.2,
        'lines.markersize': 5.0,
        'xtick.minor.visible': True,
        'xtick.direction': 'in',
        'xtick.major.size': 20.0,
        'xtick.minor.size': 10.0,
        'xtick.top': True,
        'ytick.minor.visible': True,
        'ytick.direction': 'in',
        'ytick.major.size': 20.0,
        'ytick.minor.size': 10.0,
        'ytick.right': True,
        'errorbar.capsize': 0.0,
    })
    param_meaning = {0: 'x', 1: 'y', 2: r'$\alpha$'}
    section_meaning = {0: 'b', 1: 'f'}
    layers = {'EKLM': np.arange(1, 15, 1), 'BKLM': np.arange(1, 16, 1)}
    layers_err = {'EKLM': np.full(14, 0.5), 'BKLM': np.full(15, 0.5)}
    for section in [0, 1]:
        for sector_shift in [0, 4]:
            fig, axs = plt.subplots(4, 3, figsize=(20, 20))
            for i in range(0, 12):
                sector = i//3+sector_shift
                param = i % 3
                plt.sca(axs[sector-sector_shift][param])
                plt.errorbar(
                    x=layers['BKLM'],
                    xerr=layers_err['BKLM'],
                    y=BKLM_values[section][sector][param],
                    yerr=BKLM_errors[section][sector][param],
                    ls='',
                    fmt='o',
                    ds='steps-mid',
                    color='black',
                    label='BKLM ' +
                    section_meaning[section] +
                    ' ' +
                    str(sector) +
                    r' $\chi^{2}$=' +
                    str(
                        np.around(
                            np.sum(
                                BKLM_chi2,
                                axis=3)[section][sector][param],
                            1)))
                plt.hlines(0, 0, 15, color='red')
                if (param == 2):
                    plt.ylim(-0.02, 0.02)
                    plt.ylabel(r'$\Delta$'+param_meaning[param]+' rad')
                else:
                    plt.ylim(-2, 2)
                    plt.ylabel(r'$\Delta$'+param_meaning[param]+' cm')
                plt.xlabel('Layer')
                axs[sector-sector_shift][param].yaxis.set_label_coords(-0.1, 0.5)
                plt.legend()
            fig.tight_layout()
            plt.savefig(pdfPages, format='pdf')
    plt.close('all')


def run_validation(job_path, job_path_prev, arg3, arg4, arg5):
    '''
    Run the validation.
    The script compares the most recent alignment result with the previous results by calculating the residuals.
    '''
    tmp_work_dir = os.path.join(os.getcwd(), 'tmp_work')
    tmp_plot_dir = os.path.join(os.getcwd(), 'tmp_plot')
    if not os.path.exists(tmp_work_dir):
        os.makedirs(tmp_work_dir)
    if not os.path.exists(tmp_plot_dir):
        os.makedirs(tmp_plot_dir)

    # Create alignment results tree the recent and previous calibration and get IoVs
    exp_run_list = get_result(job_path, tmp_work_dir)
    exp_run_list_prev = get_result(job_path_prev, tmp_work_dir)
    # Sort IoV from earliest to latest
    sorted_exp_run_list = sorted(exp_run_list + exp_run_list_prev)
    # Calculate the residuals for each adjacent pair of IoVs and saves the results of the comparison in a .pdf file
    for i in range(0, len(sorted_exp_run_list)-1):
        exp_prev = sorted_exp_run_list[i][0]
        run_prev = sorted_exp_run_list[i][1]
        exp = sorted_exp_run_list[i+1][0]
        run = sorted_exp_run_list[i+1][1]
        data_path = tmp_work_dir+f'/alignment_{exp_prev}_{run_prev}.root'
        data_path_prev = tmp_work_dir+f'/alignment_{exp}_{run}.root'
        EKLM_values, EKLM_errors, EKLM_chi2, BKLM_values, BKLM_errors, BKLM_chi2 = get_residuals(data_path, data_path_prev)
        pdfPages = PdfPages(tmp_plot_dir+'/e'+str(exp_prev)+'r'+str(run_prev)+'_e'+str(exp)+'r'+str(run)+'.pdf')
        draw_EKLM_pics(EKLM_values, EKLM_errors, EKLM_chi2, pdfPages)
        draw_BKLM_pics(BKLM_values, BKLM_errors, BKLM_chi2, pdfPages)
        pdfPages.close()


if __name__ == "__main__":
    run_validation(*sys.argv[1:])
