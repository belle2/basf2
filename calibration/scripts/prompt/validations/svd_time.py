#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from pathlib import Path
import sys

import pandas as pd
import seaborn as sns
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

from prompt import ValidationSettings
import svd.validation_utils as vu

import ROOT as r
r.PyConfig.IgnoreCommandLineOptions = True

matplotlib.use('Agg')
plt.style.use("belle2")

#: Tells the automated system some details of this script
settings = ValidationSettings(name="caf_svd_time",
                              description=__doc__,
                              download_files=[],
                              expert_config=None)


def progress(count, total):
    bar_len = 60
    filled_len = int(round(bar_len * count / total))
    percents = round(100 * count / total, 1)
    bar = '=' * filled_len + '-' * (bar_len - filled_len)
    sys.stdout.write(f'[{bar}] {percents}%\r')
    sys.stdout.flush()


def run_validation(job_path, input_data_path=None, **kwargs):
    '''job_path will be replaced with path/to/calibration_results
    input_data_path will be replaced with path/to/data_path used for calibration
    e.g. /group/belle2/dataprod/Data/PromptSkim/'''

    collector_output_dir = Path(job_path) / 'SVDTimeValidation/0/collector_output/default/'
    output_dir = Path(kwargs.get('output_dir', 'SVDTimeValidation_output'))
    plots_per_run = output_dir / 'runs'

    plots_per_run.mkdir(parents=True, exist_ok=True)

    files = list(collector_output_dir.glob('**/CollectorOutput.root'))

    agreements = {algo: {} for algo in vu.time_algorithms}
    precisions = {algo: {} for algo in vu.time_algorithms}
    discriminations = {algo: {} for algo in vu.time_algorithms}
    entries_onTracks = {algo: {} for algo in vu.time_algorithms}
    entries_eventT0 = {algo: {} for algo in vu.time_algorithms}

    roc_U = {algo: {} for algo in vu.time_algorithms}
    roc_V = {algo: {} for algo in vu.time_algorithms}

    num_files = len(files)
    print(f'Looping over {num_files} files')
    progress(0, num_files)
    for count, in_file_name in enumerate(files):

        in_file = r.TFile(str(in_file_name))

        for algo in vu.time_algorithms:

            histos, exp, run = vu.get_histos(in_file, algo)

            if histos is None:
                print(f'Skipping file {in_file_name} for {algo}')
                continue

            # if some histogram is empty (too little stat) do not crash but skip that file for that calibration
            try:
                entries_eventT0_ = histos['eventT0'].GetEntries()
                if run not in entries_eventT0[algo] or entries_eventT0_ > entries_eventT0[algo][run]:
                    agreements[algo][run] = {key: vu.get_agreament(histos['eventT0'], h_diff)
                                             for key, h_diff in histos['diff'].items()}
                    precisions[algo][run] = {key: vu.get_precision(h_diff)
                                             for key, h_diff in histos['diff'].items()}
                    discriminations[algo][run] = {key: vu.get_roc_auc(histos['onTracks'][key], histos['offTracks'][key])
                                                  for key in histos['onTracks']}
                    entries_onTracks[algo][run] = {key: val.GetEntries() for key, val in histos['onTracks'].items()}
                    entries_eventT0[algo][run] = entries_eventT0_

                    vu.make_combined_plot('*U', histos,
                                          title=f'exp {exp} run {run} U {algo}')
                    plt.savefig(plots_per_run / f'{exp}_{run}_U_{algo}.pdf')
                    plt.close()

                    vu.make_combined_plot('*V', histos,
                                          title=f'exp {exp} run {run} V {algo}')
                    plt.savefig(plots_per_run / f'{exp}_{run}_V_{algo}.pdf')
                    plt.close()

                    roc_U[algo][run] = vu.make_roc(vu.get_combined(histos['onTracks'], '*U'),
                                                   vu.get_combined(histos['offTracks'], '*U'))
                    roc_V[algo][run] = vu.make_roc(vu.get_combined(histos['onTracks'], '*V'),
                                                   vu.get_combined(histos['offTracks'], '*V'))
            except AttributeError:
                print(f'Skipping file {in_file_name} for {algo}')
                continue

        in_file.Close()

        # Show the progress
        progress(count+1, num_files)

    print()

    dd = {}
    runs = sorted(agreements[vu.time_algorithms[0]])
    dd['run'] = sum([[i]*len(vu.names_sides) for i in runs], [])
    dd['name'] = vu.names_sides*len(runs)
    dd['side'] = [i[-1] for i in dd['name']]

    for algo in vu.time_algorithms:
        dd[f'agreement_{algo}'] = [agreements[algo][run][side] for run, side in zip(dd['run'], dd['name'])]
        dd[f'precision_{algo}'] = [precisions[algo][run][side] for run, side in zip(dd['run'], dd['name'])]
        dd[f'discrimination_{algo}'] = [discriminations[algo][run][side] for run, side in zip(dd['run'], dd['name'])]
        dd[f'entries_onTracks_{algo}'] = [entries_onTracks[algo][run][side] for run, side in zip(dd['run'], dd['name'])]
        dd[f'entries_eventT0_{algo}'] = [entries_eventT0[algo][run] for run, side in zip(dd['run'], dd['name'])]

    # Make ROC plots
    for run in runs:
        plt.figure()
        plt.plot(*roc_U['CoG6'][run], 'k-', label='CoG6 U')
        plt.plot(*roc_V['CoG6'][run], 'k:', label='CoG6 V')
        plt.plot(*roc_U['CoG3'][run], 'b-', label='CoG3 U')
        plt.plot(*roc_V['CoG3'][run], 'b:', label='CoG3 V')
        plt.plot(*roc_U['ELS3'][run], 'r-', label='ELS3 U')
        plt.plot(*roc_V['ELS3'][run], 'r:', label='ELS3 V')
        plt.legend(loc='lower left')
        plt.xlabel('sgn efficiency')
        plt.ylabel('bkg rejection')
        plt.title(f'ROC run {run}')
        plt.xlim((0, 1))
        plt.ylim((0, 1))
        plt.tight_layout()
        plt.savefig(plots_per_run / f'ROC_{run}.pdf')
        plt.close()

    df = pd.DataFrame(dd)
    df.to_pickle(output_dir / 'df.pkl')

    # df = pd.read_pickle('df.pkl')

    print('Making combined plots')

    for algo in vu.time_algorithms:
        plt.figure(figsize=(6.4*max(2, num_files/30), 4.8*2))
        ax = sns.violinplot(x='run', y=f'agreement_{algo}', hue='side', data=df, split=True)
        ax.set_ylim([-2, 2])
        ax.xaxis.set_minor_locator(ticker.NullLocator())
        plt.axhline(0, color='black', linestyle='--')
        plt.axhline(0.5, color='black', linestyle=':')
        plt.axhline(-0.5, color='black', linestyle=':')
        plt.setp(ax.get_xticklabels(), rotation=90)
        plt.tight_layout()
        plt.savefig(output_dir / f'agreement_{algo}.pdf')
        plt.close()

        plt.figure(figsize=(6.4*max(2, num_files/30), 4.8*2))
        ax = sns.violinplot(x='run', y=f'precision_{algo}', hue='side', data=df, split=True)
        ax.set_ylim([0, 50])
        ax.xaxis.set_minor_locator(ticker.NullLocator())
        plt.axhline(10, color='black', linestyle=':')
        plt.axhline(20, color='black', linestyle=':')
        plt.setp(ax.get_xticklabels(), rotation=90)
        plt.tight_layout()
        plt.savefig(output_dir / f'precision_{algo}.pdf')
        plt.close()

        plt.figure(figsize=(6.4*max(2, num_files/30), 4.8*2))
        ax = sns.violinplot(x='run', y=f'discrimination_{algo}', hue='side', data=df, split=True)
        ax.set_ylim([0.5, 1])
        ax.xaxis.set_minor_locator(ticker.NullLocator())
        plt.axhline(0.8, color='black', linestyle=':')
        plt.axhline(0.9, color='black', linestyle=':')
        plt.setp(ax.get_xticklabels(), rotation=90)
        plt.tight_layout()
        plt.savefig(output_dir / f'discrimination_{algo}.pdf')
        plt.close()

        plt.figure(figsize=(6.4*max(2, num_files/30), 4.8*2))
        ax = sns.violinplot(x='run', y=f'entries_onTracks_{algo}', hue='side', data=df, split=True, cut=0)
        ax.xaxis.set_minor_locator(ticker.NullLocator())
        plt.setp(ax.get_xticklabels(), rotation=90)
        plt.tight_layout()
        plt.savefig(output_dir / f'entries_onTracks_{algo}.pdf')
        plt.close()

        plt.figure(figsize=(6.4*max(2, num_files/30), 4.8*2))
        ax = sns.violinplot(x='run', y=f'entries_eventT0_{algo}', hue='side', data=df, split=True)
        ax.xaxis.set_minor_locator(ticker.NullLocator())
        plt.setp(ax.get_xticklabels(), rotation=90)
        plt.tight_layout()
        plt.savefig(output_dir / f'entries_eventT0_{algo}.pdf')
        plt.close()


if __name__ == '__main__':

    import argparse
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawTextHelpFormatter)

    # b2val-prompt-run wants to pass to the script also input_data_path
    # and requested_iov. As they are not required by this validation I just accept
    # them together with calibration_results_dir and then ignore them
    parser.add_argument('calibration_results_dir',
                        help='The directory that contains the collector outputs',
                        nargs='+')

    parser.add_argument('-o', '--output_dir',
                        help='The directory where all the output will be saved',
                        default='SVDTimeValidation_output')
    args = parser.parse_args()

    run_validation(args.calibration_results_dir[0], output_dir=args.output_dir)
