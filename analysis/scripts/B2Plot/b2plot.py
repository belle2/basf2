import numpy
import matplotlib
import matplotlib.artist
import matplotlib.figure
import matplotlib.gridspec
import matplotlib.colors
import pandas
import scipy
import scipy.stats
import copy


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


class Histograms(object):
    def __init__(self, data, column, masks=dict(), weight_column=None):
        self.hist, self.bins = numpy.histogram(data[column], bins=100,
                                               weights=None if weight_column is None else data[weight_column])
        self.bin_centers = (numpy.roll(self.bins, 1) + self.bins)[1:] / 2.0
        self.bin_widths = (numpy.roll(self.bins, 1) - self.bins)[1:] / 2.0
        self.hists = dict()
        for name, mask in masks.iteritems():
            self.hists[name] = numpy.histogram(data.loc[mask, column], bins=self.bins,
                                               weights=None if weight_column is None else data.loc[mask, weight_column])[0]

    def get_hist(self, value=None):
        if value is None:
            return self.hist, poisson_error(self.hist)
        return self.get_summed_hist([value])

    def get_summed_hist(self, values):
        default = numpy.zeros(len(self.bin_centers))
        hist = numpy.sum(self.hists.get(v, default) for v in values)
        hist_error = poisson_error(hist)
        return hist, hist_error

    def get_efficiency(self, signal_values):
        signal, _ = self.get_summed_hist(signal_values)
        cumsignal = (signal.sum() - signal.cumsum()).astype('float')

        efficiency = cumsignal / signal.sum()
        efficiency_error = binom_error(cumsignal, signal.sum())
        return efficiency, efficiency_error

    def get_purity(self, signal_values, bckgrd_values):
        signal, _ = self.get_summed_hist(signal_values)
        bckgrd, _ = self.get_summed_hist(bckgrd_values)
        cumsignal = (signal.sum() - signal.cumsum()).astype('float')
        cumbckgrd = (bckgrd.sum() - bckgrd.cumsum()).astype('float')

        purity = cumsignal / (cumsignal + cumbckgrd)
        purity_error = binom_error(cumsignal, cumsignal + cumbckgrd)
        return purity, purity_error

    def get_purity_per_bin(self, signal_values, bckgrd_values):
        signal, _ = self.get_summed_hist(signal_values)
        bckgrd, _ = self.get_summed_hist(bckgrd_values)
        signal = signal.astype('float')
        bckgrd = bckgrd.astype('float')

        purity = signal / (signal + bckgrd)
        purity_error = binom_error(signal, signal + bckgrd)
        return purity, purity_error


class Plotter(object):
    def __init__(self, figure=None, axis=None):
        if figure is None:
            self.figure = matplotlib.figure.Figure(figsize=(20, 20))
            self.figure.set_tight_layout(True)
        else:
            self.figure = figure

        if axis is None:
            self.axis = self.figure.add_subplot(1, 1, 1)
        else:
            self.axis = axis

        self.plots = []
        self.labels = []
        self.xmin, self.xmax = float(0), float(1)
        self.ymin, self.ymax = float(0), float(0)

        self.set_plot_options()
        self.set_errorbar_options()
        self.set_errorband_options()

    def save(self, filename):
        from matplotlib.backends.backend_agg import FigureCanvasAgg as FigureCanvas
        canvas = FigureCanvas(self.figure)
        canvas.print_figure(filename, dpi=100)
        return self

    def set_plot_options(self, plot_kwargs={}):
        self.plot_kwargs = plot_kwargs
        return self

    def set_errorbar_options(self, errorbar_kwargs={'fmt': '.', 'elinewidth': 0.5, 'alpha': 0.5}):
        self.errorbar_kwargs = errorbar_kwargs
        return self

    def set_errorband_options(self, errorband_kwargs={'alpha': 0.5}):
        self.errorband_kwargs = errorband_kwargs
        return self

    def _plot_datapoints(self, axis, x, y, xerr=None, yerr=None):
        p = e = f = color = None
        plot_kwargs = copy.copy(self.plot_kwargs)
        errorbar_kwargs = copy.copy(self.errorbar_kwargs)
        errorband_kwargs = copy.copy(self.errorband_kwargs)

        if plot_kwargs is not None:
            p, = axis.plot(x, y, **plot_kwargs)
            color = matplotlib.colors.ColorConverter().to_rgb(p.get_color())

        if errorbar_kwargs is not None and (xerr is not None or yerr is not None):
            if 'color' not in errorbar_kwargs and color is not None:
                errorbar_kwargs['color'] = color
            if 'ecolor' not in errorbar_kwargs and color is not None:
                errorbar_kwargs['ecolor'] = map(lambda x: 0.5*x, color)
            e = axis.errorbar(x, y, xerr=xerr, yerr=yerr, **errorbar_kwargs)

        if errorband_kwargs is not None and yerr is not None:
            if 'color' not in errorband_kwargs and color is not None:
                errorband_kwargs['color'] = color
            f = axis.fill_between(x, y-yerr, y+yerr, **errorband_kwargs)

        return (p, e, f)


class PurityOverEfficiency(Plotter):

    def add(self, data, column, signal_mask, bckgrd_mask, weight_column=None):
        hists = Histograms(data, column, {'Signal': signal_mask, 'Background': bckgrd_mask}, weight_column=weight_column)
        efficiency, efficiency_error = hists.get_efficiency(['Signal'])
        purity, purity_error = hists.get_purity(['Signal'], ['Background'])

        self.xmin, self.xmax = numpy.nanmin([efficiency.min(), self.xmin]), numpy.nanmax([efficiency.max(), self.xmax])
        self.ymin, self.ymax = numpy.nanmin([numpy.nanmin(purity), self.ymin]), numpy.nanmax([numpy.nanmax(purity), self.ymax])

        p = self._plot_datapoints(self.axis, efficiency, purity, xerr=efficiency_error, yerr=purity_error)
        self.plots.append(p)
        self.labels.append(column)
        return self

    def finish(self):
        self.axis.set_xlim((self.xmin, self.xmax))
        self.axis.set_ylim((self.ymin, self.ymax))
        self.axis.set_title("ROC Purity Plot")
        self.axis.get_xaxis().set_label_text('Efficiency')
        self.axis.get_yaxis().set_label_text('Purity')
        self.axis.legend(map(lambda x: x[0], self.plots), self.labels, loc=3)
        return self


class RejectionOverEfficiency(Plotter):

    def add(self, data, column, signal_mask, bckgrd_mask, weight_column=None):
        hists = Histograms(data, column, {'Signal': signal_mask, 'Background': bckgrd_mask}, weight_column=weight_column)
        efficiency, efficiency_error = hists.get_efficiency(['Signal'])
        rejection, rejection_error = hists.get_efficiency(['Background'])
        rejection = 1 - rejection

        self.xmin, self.xmax = numpy.nanmin([efficiency.min(), self.xmin]), numpy.nanmax([efficiency.max(), self.xmax])
        self.ymin, self.ymax = numpy.nanmin([rejection.min(), self.ymin]), numpy.nanmax([rejection.max(), self.ymax])

        p = self._plot_datapoints(self.axis, efficiency, rejection, xerr=efficiency_error, yerr=rejection_error)
        self.plots.append(p)
        self.labels.append(column)
        return self

    def finish(self):
        self.axis.set_xlim((self.xmin, self.xmax))
        self.axis.set_ylim((self.ymin, self.ymax))
        self.axis.set_title("ROC Rejection Plot")
        self.axis.get_xaxis().set_label_text('Signal Efficiency')
        self.axis.get_yaxis().set_label_text('Background Rejection')
        self.axis.legend(map(lambda x: x[0], self.plots), self.labels, loc=3)
        return self


class Diagonal(Plotter):

    def add(self, data, column, signal_mask, bckgrd_mask, weight_column=None):
        hists = Histograms(data, column, {'Signal': signal_mask, 'Background': bckgrd_mask}, weight_column=weight_column)
        purity, purity_error = hists.get_purity_per_bin(['Signal'], ['Background'])

        self.xmin, self.xmax = min(hists.bin_centers.min(), self.xmin), max(hists.bin_centers.max(), self.xmax)
        self.ymin, self.ymax = numpy.nanmin([numpy.nanmin(purity), self.ymin]), numpy.nanmax([numpy.nanmax(purity), self.ymax])

        p = self._plot_datapoints(self.axis, hists.bin_centers, purity, xerr=hists.bin_widths, yerr=purity_error)
        self.plots.append(p)
        self.labels.append(column)
        return self

    def finish(self):
        self.axis.plot((self.xmin, self.xmax), (0, 1), color='black')
        self.axis.set_xlim((self.xmin, self.xmax))
        self.axis.set_ylim((self.ymin, self.ymax))
        self.axis.set_title("Diagonal Plot")
        self.axis.get_xaxis().set_label_text('Classifier Output')
        self.axis.get_yaxis().set_label_text('Purity Per Bin')
        self.axis.legend(map(lambda x: x[0], self.plots), self.labels, loc=4)
        return self


class Distribution(Plotter):

    def add(self, data, column, mask=None, weight_column=None):
        if mask is None:
            mask = numpy.ones(len(data)).astype('bool')
        hists = Histograms(data, column, {'Total': mask}, weight_column=weight_column)
        hist, hist_error = hists.get_hist('Total')

        self.xmin, self.xmax = min(hists.bin_centers.min(), self.xmin), max(hists.bin_centers.max(), self.xmax)
        self.ymin, self.ymax = numpy.nanmin([hist.min(), self.ymin]), numpy.nanmax([(hist + hist_error).max(), self.ymax])

        p = self._plot_datapoints(self.axis, hists.bin_centers, hist, xerr=hists.bin_widths, yerr=hist_error)
        self.plots.append(p)
        self.labels.append(column)
        return self

    def finish(self):
        self.axis.set_xlim((self.xmin, self.xmax))
        self.axis.set_ylim((self.ymin, self.ymax))
        self.axis.set_title("Distribution Plot")
        self.axis.get_xaxis().set_label_text('Classifier Output')
        self.axis.get_yaxis().set_label_text('# Entries')
        self.axis.legend(map(lambda x: x[0], self.plots), self.labels, loc=4)
        return self


class Difference(Plotter):

    def add(self, data, column, minuend_mask, subtrahend_mask, weight_column=None):
        hists = Histograms(data, column, {'Minuend': minuend_mask, 'Subtrahend': subtrahend_mask}, weight_column=weight_column)
        minuend, minuend_error = hists.get_hist('Minuend')
        subtrahend, subtrahend_error = hists.get_hist('Subtrahend')
        difference, difference_error = minuend - subtrahend, poisson_error(minuend + subtrahend)

        self.xmin, self.xmax = min(hists.bin_centers.min(), self.xmin), max(hists.bin_centers.max(), self.xmax)
        self.ymin = min((difference - difference_error).min(), self.ymin)
        self.ymax = max((difference + difference_error).max(), self.ymax)

        p = self._plot_datapoints(self.axis, hists.bin_centers, difference, xerr=hists.bin_widths, yerr=difference_error)
        self.plots.append(p)
        self.labels.append(column)
        return self

    def finish(self):
        self.axis.set_xlim((self.xmin, self.xmax))
        self.axis.set_ylim((self.ymin, self.ymax))
        self.axis.set_title("Difference Plot")
        self.axis.get_xaxis().set_label_text('Classifier Output')
        self.axis.get_yaxis().set_label_text('Difference')
        self.axis.legend(map(lambda x: x[0], self.plots), self.labels, loc=4)
        return self


class Overtraining(Plotter):
    def __init__(self, figure=None):
        if figure is None:
            self.figure = matplotlib.figure.Figure(figsize=(20, 20))
            self.figure.set_tight_layout(True)
        else:
            self.figure = figure

        gs = matplotlib.gridspec.GridSpec(5, 1)
        self.axis = self.figure.add_subplot(gs[:3, :])
        self.axis_d1 = self.figure.add_subplot(gs[3, :], sharex=self.axis)
        self.axis_d2 = self.figure.add_subplot(gs[4, :], sharex=self.axis)

        super(Overtraining, self).__init__(self.figure, self.axis)

    def add(self, data, column, train_mask, test_mask, signal_mask, bckgrd_mask, weight_column=None):
        distribution = Distribution(self.figure, self.axis)
        distribution.add(data, column, train_mask & signal_mask, weight_column)
        distribution.add(data, column, train_mask & bckgrd_mask, weight_column)
        distribution.add(data, column, test_mask & signal_mask, weight_column)
        distribution.add(data, column, test_mask & bckgrd_mask, weight_column)
        distribution.labels = ['Train-Signal', 'Train-Background', 'Test-Signal', 'Test-Background']
        distribution.finish()

        difference_signal = Difference(self.figure, self.axis_d1)
        difference_signal.add(data, column, train_mask & signal_mask, test_mask & signal_mask, weight_column)
        self.axis_d1.set_xlim((difference_signal.xmin, difference_signal.xmax))
        self.axis_d1.set_ylim((difference_signal.ymin, difference_signal.ymax))

        difference_bckgrd = Difference(self.figure, self.axis_d2)
        difference_bckgrd.add(data, column, train_mask & signal_mask, test_mask & signal_mask, weight_column)
        self.axis_d2.set_xlim((difference_bckgrd.xmin, difference_bckgrd.xmax))
        self.axis_d2.set_ylim((difference_bckgrd.ymin, difference_bckgrd.ymax))

        # Kolmogorov smirnov test
        ks = scipy.stats.ks_2samp(data.loc[train_mask & signal_mask, column], data.loc[test_mask & signal_mask, column])
        self.axis_d1.text(0.95, 0.01, r'signal (train - test) difference $p={:.2f}$'.format(ks[1]), fontsize=28,
                          verticalalignment='bottom', horizontalalignment='right', transform=self.axis_d1.transAxes)
        ks = scipy.stats.ks_2samp(data.loc[train_mask & bckgrd_mask, column], data.loc[test_mask & bckgrd_mask, column])
        self.axis_d2.text(0.95, 0.01, r'background (train - test) difference $p={:.2f}$'.format(ks[1]), fontsize=28,
                          verticalalignment='bottom', horizontalalignment='right', transform=self.axis_d2.transAxes)
        return self

    def finish(self):
        matplotlib.artist.setp(self.axis.get_xaxis(), visible=False)
        matplotlib.artist.setp(self.axis_d1.get_xaxis(), visible=False)
        self.axis.set_title("Overtraining Plot")
        self.axis_d2.get_xaxis().set_label_text('Classifier Output')
        return self


class Correlation(Plotter):
    def add(self, data, column, cut_column, quantiles, mask=None, weight_column=None):

        percentiles = numpy.percentile(data[cut_column], q=quantiles)
        distribution = Distribution(self.figure, self.axis)
        for p in percentiles:
            distribution.add(data, column, data[cut_column] > p, weight_column)
        if mask is not None:
            self.axis.set_color_cycle(None)
            distribution.set_errorbar_options(None)
            distribution.set_errorband_options(None)
            for p in percentiles:
                distribution.add(data, column, (data[cut_column] > p) & mask, weight_column)
        distribution.plots = distribution.plots[:len(quantiles)]
        distribution.labels = [str(q) + '% Quantiles' for q in quantiles]
        distribution.finish()
        return self

    def finish(self):
        return self


def get_data(N, columns):
    N /= 2
    n = len(columns) - 1
    xs = numpy.random.normal(0, size=(N, n))
    xb = numpy.random.normal(1, size=(N, n))
    ys = numpy.zeros(N)
    yb = numpy.ones(N)
    data = pandas.DataFrame(numpy.c_[numpy.r_[xs, xb], numpy.r_[ys, yb]], columns=columns)
    return data.reindex(numpy.random.permutation(data.index))


if __name__ == '__main__':

    import seaborn
    # Set nice searborn settings
    seaborn.set(font_scale=3)
    seaborn.set_style('whitegrid')

    # Standard plots
    data = get_data(10000, columns=['FastBDT', 'NeuroBayes', 'isSignal'])
    data['type'] = ''
    data.type.iloc[:5000] = 'Train'
    data.type.iloc[5000:] = 'Test'

    p = PurityOverEfficiency()
    p.add(data, 'FastBDT', data['isSignal'] == 1, data['isSignal'] == 0)
    p.add(data, 'NeuroBayes', data['isSignal'] == 1, data['isSignal'] == 0)
    p.finish()
    p.save('roc_purity_plot.png')

    p = RejectionOverEfficiency()
    p.add(data, 'FastBDT', data['isSignal'] == 1, data['isSignal'] == 0)
    p.add(data, 'NeuroBayes', data['isSignal'] == 1, data['isSignal'] == 0)
    p.finish()
    p.save('roc_rejection_plot.png')

    p = Diagonal()
    p.add(data, 'FastBDT', data['isSignal'] == 1, data['isSignal'] == 0)
    p.add(data, 'NeuroBayes', data['isSignal'] == 1, data['isSignal'] == 0)
    p.finish()
    p.save('diagonal_plot.png')

    p = Distribution()
    p.add(data, 'FastBDT')
    p.add(data, 'NeuroBayes')
    p.finish()
    p.save('distribution_plot.png')

    p = Difference()
    p.add(data, 'FastBDT', data['type'] == 'Train', data['type'] == 'Test')
    p.add(data, 'NeuroBayes', data['type'] == 'Train', data['type'] == 'Test')
    p.finish()
    p.save('difference_plot.png')

    p = Overtraining()
    p.add(data, 'FastBDT', data['type'] == 'Train', data['type'] == 'Test', data['isSignal'] == 1, data['isSignal'] == 0)
    p.finish()
    p.save('overtraining_plot.png')

    p = Correlation()
    p.add(data, 'FastBDT', 'NeuroBayes', [0, 20, 40, 60, 80, 100], data['isSignal'] == 0)
    p.finish()
    p.save('correlation_plot.png')
