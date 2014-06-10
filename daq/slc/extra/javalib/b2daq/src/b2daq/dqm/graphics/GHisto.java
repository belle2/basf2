package b2daq.dqm.graphics;

import b2daq.dqm.core.Histo;
import b2daq.dqm.core.MonObject;

public abstract class GHisto extends GMonObject {

    protected Histo histo = null;
    protected GAxis axisY = null;
    protected double[] pointX = new double[0];
    protected double[] pointY = new double[0];
    private boolean ajustableAxis = false;

    public GHisto() {
        super(null);
    }

    public GHisto(HistogramCanvas canvas) {
        super(canvas);
        if (canvas != null) {
            axisY = canvas.getAxisY();
        }
    }

    public GHisto(HistogramCanvas canvas, Histo histo) {
        this(canvas);
        this.histo = histo;
    }

    @Override
    public Histo getObject() {
        return histo;
    }

    public Histo getHisto() {
        return histo;
    }

    @Override
    public void setObject(MonObject histo) {
        this.histo = (Histo) histo;
    }

    public void setAxisY(GAxis axis) {
        axisY = axis;
    }

    @Override
    public void update() {
        if (ajustableAxis) {
            axisY.get().copyRange(histo.getAxisY());
        }
        histo.setMaxAndMin();
    }

    public boolean isAxisAjustable() {
        return ajustableAxis;
    }

    public void setAxisAjustable(boolean axisAjustable) {
        this.ajustableAxis = axisAjustable;
    }

    void setCanvas(HistogramCanvas canvas) {
        this.canvas = canvas;
        if (canvas != null) {
            axisY = canvas.getAxisY();
        }
    }

}
