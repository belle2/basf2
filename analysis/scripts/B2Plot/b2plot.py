import numpy
import matplotlib
import matplotlib.figure
import matplotlib.gridspec
import matplotlib.colors
import pandas
import scipy
import scipy.stats


def binom_error(n_sig, n_tot):
    """
    for an efficiency = nSig/nTrueSig or purity = nSig / (nSig + nBckgrd), this function calculates the
    standard deviation according to http://arxiv.org/abs/physics/0701199 .
    """
    variance = numpy.where(n_tot > 0, (n_sig + 1) * (n_sig + 2) / ((n_tot + 2) * (n_tot + 3))
                           - (n_sig + 1) ** 2 / ((n_tot + 2) ** 2), 0)
    return numpy.sqrt(variance)


def poisson_error(n_tot):
    """
    use poisson error, except for 0 we use an 68% CL upper limit
    """
    return numpy.where(n_tot > 0, numpy.sqrt(n_tot), numpy.log(1.0/(1-0.6827)))


class Plotter(object):

    def __init__(self, data, columns, target, weight=None):
        self.set_plot_options()

        self.columns = columns
        self.hists = {}
        self.weight = weight

        self.bin_patches = {}
        self.bin_centers = {}
        self.bin_widths = {}
        self.xmax = float('-inf')
        self.xmin = float('inf')
        self.ymax = float('-inf')
        self.ymin = float('inf')

        self.target_values = data[target].unique()

        for column in columns:
            _, patches = numpy.histogram(data[column], bins=100, weights=None if weight is None else data[weight])
            self.hists[column] = {}
            for t in self.target_values:
                t_data = data[data[target] == t]
                hist, _ = numpy.histogram(t_data[column], bins=patches,
                                          weights=None if weight is None else t_data[weight])
                self.ymax = numpy.nanmax([hist.max(), hist.max(), self.ymax])
                self.ymin = numpy.nanmin([hist.min(), hist.min(), self.ymin])
                self.hists[column][t] = hist.astype('float')

            # Save binning
            self.bin_patches[column] = patches
            self.bin_centers[column] = (numpy.roll(patches, 1) + patches)[1:] / 2.0
            self.bin_widths[column] = (numpy.roll(patches, 1) - patches)[1:] / 2.0
            # Save maximum and minimum for x and y dimension
            self.xmax = numpy.nanmax([data[column].max(), self.xmax])
            self.xmin = numpy.nanmin([data[column].min(), self.xmin])

    def add_data(self, data):
        for column in self.columns:
            self.target_values = numpy.unique(numpy.r_[self.target_values, data[target].unique()])
            for t in self.target_value:
                t_data = data[data[target] == t]
                hist, _ = numpy.histogram(t_data[column], bins=self.patches[column],
                                          weights=None if self.weight is None else t_data[self.weight])
                self.ymax = numpy.nanmax([hist.max(), hist.max(), self.ymax])
                self.ymin = numpy.nanmin([hist.min(), hist.min(), self.ymin])
                if t in self.hists[column]:
                    self.hists[column][t] += hist
                else:
                    self.hists[column][t] = hist
            self.xmax = numpy.nanmax([data[column].max(), self.xmax])
            self.xmin = numpy.nanmin([data[column].min(), self.xmin])

    def set_plot_options(self,
                         plot_kwargs={},
                         errorbar_kwargs={'fmt': '.', 'elinewidth': 0.5, 'alpha': 0.5},
                         errorband_kwargs={'alpha': 0.5}):
        self.plot_kwargs = plot_kwargs
        self.errorbar_kwargs = errorbar_kwargs
        self.errorband_kwargs = errorband_kwargs

    def datapoint_plot(self, axis, x, y, xerr=None, yerr=None, label=None):
        p = e = f = color = None
        if self.plot_kwargs is not None:
            p, = axis.plot(x, y, label=label, **self.plot_kwargs)
            color = matplotlib.colors.ColorConverter().to_rgb(p.get_color())
        if self.errorbar_kwargs is not None:
            darker_color = map(lambda x: 0.5*x, color)
            e = axis.errorbar(x, y, xerr=xerr, yerr=yerr, color=color, ecolor=darker_color, **self.errorbar_kwargs)
        if self.errorband_kwargs is not None:
            f = axis.fill_between(x, y-yerr, y+yerr, color=color, **self.errorband_kwargs)
        return (p, e, f)

    def roc_purity_plot(self, axis, signal_values, bckgrd_values):
        plots = []
        ymax = float(1)
        ymin = float(0)
        xmax = float(1)
        xmin = float(0)
        for column in self.columns:
            bin_centers = self.bin_centers[column]
            default = numpy.zeros(len(bin_centers))
            signal = numpy.sum(self.hists[column].get(s, default) for s in signal_values)
            bckgrd = numpy.sum(self.hists[column].get(b, default) for b in bckgrd_values)

            cumsignal = (signal.sum() - signal.cumsum()).astype('float')
            cumbckgrd = (bckgrd.sum() - bckgrd.cumsum()).astype('float')

            efficiency = cumsignal / signal.sum()
            efficiency_error = binom_error(cumsignal, signal.sum())
            purity = cumsignal / (cumsignal + cumbckgrd)
            purity_error = binom_error(cumsignal, cumsignal + cumbckgrd)

            ymax = numpy.nanmax([purity.max(), ymax])
            ymin = numpy.nanmin([purity.min(), ymin])
            xmax = numpy.nanmax([efficiency.max(), xmax])
            xmin = numpy.nanmin([efficiency.min(), xmin])

            p = self.datapoint_plot(axis, efficiency, purity, xerr=efficiency_error, yerr=purity_error, label=column)
            plots.append(p)

        axis.set_xlim((xmin, xmax))
        axis.set_ylim((ymin, ymax))
        return plots

    def roc_rejection_plot(self, axis, signal_values, bckgrd_values):
        plots = []
        ymax = float(1)
        ymin = float(0)
        xmax = float(1)
        xmin = float(0)
        for column in self.columns:
            bin_centers = self.bin_centers[column]
            default = numpy.zeros(len(bin_centers))
            signal = numpy.sum(self.hists[column].get(s, default) for s in signal_values)
            bckgrd = numpy.sum(self.hists[column].get(b, default) for b in bckgrd_values)

            cumsignal = (signal.sum() - signal.cumsum()).astype('float')
            cumbckgrd = (bckgrd.cumsum()).astype('float')

            efficiency = cumsignal / signal.sum()
            efficiency_error = binom_error(cumsignal, signal.sum())

            rejection = cumbckgrd / bckgrd.sum()
            rejection_error = binom_error(cumbckgrd, bckgrd.sum())

            ymax = numpy.nanmax([rejection.max(), ymax])
            ymin = numpy.nanmin([rejection.min(), ymin])
            xmax = numpy.nanmax([efficiency.max(), xmax])
            xmin = numpy.nanmin([efficiency.min(), xmin])

            p = self.datapoint_plot(axis, efficiency, rejection, xerr=efficiency_error, yerr=rejection_error, label=column)
            plots.append(p)

        axis.set_xlim((xmin, xmax))
        axis.set_ylim((ymin, ymax))
        return plots

    def diagonal_plot(self, axis, signal_values, bckgrd_values):
        plots = []
        ymax = float(1)
        ymin = float(0)
        for column in self.columns:
            bin_centers = self.bin_centers[column]
            bin_widths = self.bin_widths[column]

            default = numpy.zeros(len(bin_centers))
            signal = numpy.sum(self.hists[column].get(s, default) for s in signal_values)
            bckgrd = numpy.sum(self.hists[column].get(b, default) for b in bckgrd_values)

            purity = signal / (signal + bckgrd)
            purity_error = binom_error(signal, signal + bckgrd)

            ymax = numpy.nanmax([purity.max(), ymax])
            ymin = numpy.nanmin([purity.min(), ymin])

            p = self.datapoint_plot(axis, bin_centers, purity, xerr=bin_widths, yerr=purity_error, label=column)
            plots.append(p)

        axis.plot((self.xmin, self.xmax), (0, 1), color='black')
        axis.set_xlim((self.xmin, self.xmax))
        axis.set_ylim((ymin, ymax))
        return plots

    def distribution_plot(self, axis, values):
        plots = []
        ymax = float('-inf')
        ymin = float(0)
        for column in self.columns:
            bin_centers = self.bin_centers[column]
            bin_widths = self.bin_widths[column]

            default = numpy.zeros(len(bin_centers))
            for v in values:
                hist = numpy.sum(self.hists[column].get(s, default) for s in v)
                hist_error = poisson_error(hist)
                ymax = numpy.nanmax([hist.max() + hist_error.max(), ymax])
                ymin = numpy.nanmin([hist.min(), ymin])

                p = self.datapoint_plot(axis, bin_centers, hist, xerr=bin_widths, yerr=hist_error, label=column + ' ' + str(v))
                plots.append(p)

        axis.set_xlim((self.xmin, self.xmax))
        axis.set_ylim((ymin, ymax))
        return plots

    def difference_plot(self, axis, signal_values, bckgrd_values):
        plots = []
        ymax = float('-inf')
        ymin = float('inf')
        for column in self.columns:
            bin_centers = self.bin_centers[column]
            bin_widths = self.bin_widths[column]

            default = numpy.zeros(len(bin_centers))
            signal = numpy.sum(self.hists[column].get(s, default) for s in signal_values)
            bckgrd = numpy.sum(self.hists[column].get(b, default) for b in bckgrd_values)

            hist = signal - bckgrd
            hist_error = poisson_error(signal + bckgrd)

            ymax = numpy.nanmax([hist.max() + hist_error.max(), ymax])
            ymin = numpy.nanmin([hist.min() - hist_error.max(), ymin])
            p = self.datapoint_plot(axis, bin_centers, hist, xerr=bin_widths, yerr=hist_error,
                                    label=column + '(' + str(signal_values) + ' - ' + str(bckgrd_values) + ')')
            plots.append(p)

        axis.set_xlim((self.xmin, self.xmax))
        axis.set_ylim((ymin, ymax))
        return plots


def roc_purity_plot(plotter, axis, signal='Signal', background='Background'):
    plots = plotter.roc_purity_plot(axis, signal_values=[signal], bckgrd_values=[background])
    axis.set_title("ROC Purity Plot")
    axis.get_xaxis().set_label_text('Efficiency')
    axis.get_yaxis().set_label_text('Purity')
    axis.legend(handles=map(lambda x: x[0], plots), loc=3)


def roc_rejection_plot(plotter, axis, signal='Signal', background='Background'):
    plots = plotter.roc_rejection_plot(axis, signal_values=[signal], bckgrd_values=[background])
    axis.set_title("ROC Rejection Plot")
    axis.get_xaxis().set_label_text('Signal Efficiency')
    axis.get_yaxis().set_label_text('Background Rejection')
    axis.legend(handles=map(lambda x: x[0], plots), loc=3)


def diagonal_plot(plotter, axis, signal='Signal', background='Background'):
    plots = plotter.diagonal_plot(axis, signal_values=[signal], bckgrd_values=[background])
    axis.set_title("Diagonal Plot")
    axis.get_xaxis().set_label_text('Classifier Output')
    axis.get_yaxis().set_label_text('Purity')
    axis.legend(handles=map(lambda x: x[0], plots), loc=4)


def distribution_plot(plotter, axis):
    plots = plotter.distribution_plot(axis, [['Signal'], ['Background']])
    axis.set_title("Distribution Plot")
    axis.get_xaxis().set_label_text('Classifier Output')
    axis.get_yaxis().set_label_text('N')
    axis.legend(map(lambda x: x[0], plots), sum(([x + '(Signal)', x + '(Background)'] for x in plotter.columns), []), loc=4)


def difference_plot(plotter, axis, signal='Signal', background='Background'):
    plots = plotter.difference_plot(axis, signal_values=[signal], bckgrd_values=[background])
    axis.set_title("Difference Plot")
    axis.get_xaxis().set_label_text('Classifier Output')
    axis.get_yaxis().set_label_text('Difference')
    axis.legend(handles=map(lambda x: x[0], plots), loc=4)


def overtraining_plot(plotter, axes,
                      train_signal='Train-Signal', test_signal='Test-Signal',
                      train_background='Train-Background', test_background='Test-Background'):
    if len(plotter.columns) != 1:
        raise RuntimeError('Overtraining plot with more than one column is not supported')
    c = plotter.columns[0]

    plots = plotter.distribution_plot(axes[0], [[train_signal], [test_signal], [train_background], [test_background]])
    axes[0].set_title("Distribution Plot")
    axes[0].get_xaxis().set_label_text('Classifier Output')
    axes[0].get_yaxis().set_label_text('N')
    axes[0].legend(map(lambda x: x[0], plots), [train_signal, test_signal, train_background, test_background], loc=4)
    plotter.difference_plot(axes[1], [train_signal], [test_signal])
    ks = scipy.stats.ks_2samp(plotter.hists[c][train_signal], plotter.hists[c][test_signal])
    axes[1].set_title("Signal Difference Plot (Train - Test) KS = {ks:.3f}".format(ks=ks[1]))
    axes[1].get_xaxis().set_label_text('Classifier Output')
    axes[1].get_yaxis().set_label_text('Difference')
    plotter.difference_plot(axes[2], [train_background], [test_background])
    ks = scipy.stats.ks_2samp(plotter.hists[c][train_background], plotter.hists[c][test_background])
    axes[2].set_title("Background Difference Plot (Train - Test) KS = {ks:.3f}".format(ks=ks[1]))
    axes[2].get_xaxis().set_label_text('Classifier Output')
    axes[2].get_yaxis().set_label_text('Difference')
    plotter.difference_plot(axes[3], [train_background, train_signal], [test_background, test_signal])
    axes[3].set_title("Total Difference Plot (Train - Test)")
    axes[3].get_xaxis().set_label_text('Classifier Output')
    axes[3].get_yaxis().set_label_text('Difference')


def correlation_plot(data, background_mask, axis, x='FastBDT', y='NeuroBayes'):
    quantiles = [30, 60, 90]
    data[x + '_quantile'] = 0
    for i, p in enumerate(numpy.percentile(data[x], q=quantiles)):
        data.loc[data[x] > p, x + '_quantile'] = quantiles[i]

    plotter = Plotter(data[background_mask], columns=[y], target=x + '_quantile')
    plotter.set_plot_options(errorbar_kwargs=None, errorband_kwargs=None)
    plotter.distribution_plot(axis, [[0] + quantiles, quantiles, quantiles[1:], quantiles[2:]])

    axis.set_color_cycle(None)
    plotter = Plotter(data, columns=[y], target=x + '_quantile')
    plots = plotter.distribution_plot(axis, [[0] + quantiles, quantiles, quantiles[1:], quantiles[2:]])
    axis.set_title(y + ' with different cuts on ' + x)
    axis.get_xaxis().set_label_text(y)
    axis.get_yaxis().set_label_text('N')
    axis.legend(map(lambda x: x[0], plots), ['NoCut', 'Cut on 30% Quantile', 'Cut on 60% Quantile', 'Cut on 90% Quantile'], loc=4)


def get_data(N, columns):
    N /= 2
    n = len(columns) - 1
    xs = numpy.random.normal(0, size=(N, n))
    xb = numpy.random.normal(1, size=(N, n))
    ys = numpy.zeros(N)
    yb = numpy.ones(N)
    data = pandas.DataFrame(numpy.c_[numpy.r_[xs, xb], numpy.r_[ys, yb]], columns=columns)
    return data.reindex(numpy.random.permutation(data.index))


def write(figure, filename):
    from matplotlib.backends.backend_agg import FigureCanvasAgg as FigureCanvas
    canvas = FigureCanvas(figure)
    canvas.print_figure(filename, dpi=100)


if __name__ == '__main__':

    import seaborn
    # Set nice searborn settings
    seaborn.set(font_scale=3)
    seaborn.set_style('whitegrid')

    # Standard plots
    data = get_data(10000, columns=['FastBDT', 'NeuroBayes', 'isSignal'])
    data['class'] = ''
    data.loc[data['isSignal'] == 1, 'class'] = 'Signal'
    data.loc[data['isSignal'] == 0, 'class'] = 'Background'
    plotter = Plotter(data, columns=['FastBDT', 'NeuroBayes'], target='class')

    figure = matplotlib.figure.Figure(figsize=(20, 20))
    figure.set_tight_layout(True)
    gs = matplotlib.gridspec.GridSpec(4, 1)
    roc_purity_plot(plotter, figure.add_subplot(gs[0, :]))
    roc_rejection_plot(plotter, figure.add_subplot(gs[1, :]))
    diagonal_plot(plotter, figure.add_subplot(gs[2, :]))
    distribution_plot(plotter, figure.add_subplot(gs[3, :]))
    write(figure, "comparison.png")

    # Correlation plot
    figure = matplotlib.figure.Figure(figsize=(20, 20))
    figure.set_tight_layout(True)
    gs = matplotlib.gridspec.GridSpec(1, 1)
    correlation_plot(data, data['class'] == 'Background', figure.add_subplot(gs[:, :]))
    write(figure, "correlation.png")

    # Overtraining plot
    data['type'] = ''
    data.type.iloc[:5000] = 'Train'
    data.type.iloc[5000:] = 'Test'
    data.loc[(data['isSignal'] == 1) & (data['type'] == 'Train'), 'class'] = 'Train-Signal'
    data.loc[(data['isSignal'] == 1) & (data['type'] == 'Test'), 'class'] = 'Test-Signal'
    data.loc[(data['isSignal'] == 0) & (data['type'] == 'Train'), 'class'] = 'Train-Background'
    data.loc[(data['isSignal'] == 0) & (data['type'] == 'Test'), 'class'] = 'Test-Background'
    plotter = Plotter(data, columns=['FastBDT'], target='class')

    figure = matplotlib.figure.Figure(figsize=(20, 20))
    figure.set_tight_layout(True)
    gs = matplotlib.gridspec.GridSpec(5, 1)
    axes = [figure.add_subplot(gs[:2, :]), figure.add_subplot(gs[2, :]), figure.add_subplot(gs[3, :]), figure.add_subplot(gs[4, :])]
    overtraining_plot(plotter, axes)
    write(figure, "overtraining.png")
