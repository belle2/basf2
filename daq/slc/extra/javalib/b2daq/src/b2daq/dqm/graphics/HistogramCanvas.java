package b2daq.dqm.graphics;

import b2daq.dqm.core.Histo;
import b2daq.dqm.core.Histo2;
import b2daq.dqm.core.MonObject;
import b2daq.dqm.core.TimedGraph1;
import b2daq.dqm.ui.CanvasPanel;
import b2daq.graphics.FontProperty;
import b2daq.graphics.GraphicsDrawer;
import b2daq.graphics.GRect;
import b2daq.graphics.GShape;
import b2daq.graphics.GText;
import java.util.ArrayList;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.scene.paint.Color;
import javafx.scene.text.FontWeight;

public class HistogramCanvas extends GRect {

    private Pad pad = new Pad();
    private GAxis axisx = new GAxisX();
    private GAxis axisy = new GAxisY();
    private GAxis axisy2 = null;
    private GAxis colorAxis = null;
    private String colorPattern = "";
    private final ObservableList<Histo> histograms = FXCollections.observableArrayList();
    private final ObservableList<GShape> shapes = FXCollections.observableArrayList();
    private final ArrayList<GShape> cshapes = new ArrayList<>();
    private Legend legend = null;
    private double _title_offset = 0.0;
    private GText _title;
    private String _title_position = "center";
    private boolean _use_pad = false;
    private long _update_time = -1;
    private GRect _stat_rect;
    private GText _stat_name;
    private GText _stat_update;
    private GText _stat_entries;
    private Histo _stat_histo = null;
    private boolean stat;
    private CanvasPanel panel = null;

    public HistogramCanvas(String name, String title, double width, double height, boolean use_stat) {
        super(0, 0, 1, 1);
        stat = use_stat;
        _title = new GText("", 0.5, pad.getY() / 3 - _title_offset, "center top");
        setName(name);
        setFont(new FontProperty(Color.BLACK, "Arial", 1.1, FontWeight.BOLD));
        setTitle(title);
        resetPadding();
        setFillColor(Color.WHITE);
    }

    public HistogramCanvas(String name, String title, double width, double height) {
        this(name, title, width, height, true);
    }

    public HistogramCanvas(String name, String title) {
        this(name, title, 100, 100);
    }

    public HistogramCanvas() {
        this("", "", 100, 100);
    }

    public HistogramCanvas(ObservableList<Histo> histos) {
        this("", "", 100, 100);
        setHistograms(histos);
    }

    public Pad getPad() {
        return pad;
    }

    public void resetPadding() {
        if (!_use_pad) {
            return;
        }
        axisx.set(pad.getX(), pad.getY() + pad.getHeight(),
                pad.getWidth(), 0, 1 - (pad.getY() + pad.getHeight()));
        axisy.set(pad.getX(), pad.getY() + pad.getHeight(),
                pad.getHeight(), pad.getX(), 0);
        if (colorAxis != null) {
            colorAxis.set(pad.getX() + pad.getWidth() + (1 - pad.getX() - pad.getWidth()) * 0.28, pad.getY() + pad.getHeight(),
                    pad.getHeight(), (1 - pad.getX() - pad.getWidth()) * 0.25, 0);
        }
        if (axisy2 != null) {
            axisy2.set(pad.getX() + pad.getWidth(), pad.getY() + pad.getHeight(),
                    pad.getHeight(), 1 - pad.getX() - pad.getWidth(), 0);
        }
    }

    public int getNHistos() {
        return histograms.size();
    }

    public Histo getHisto(int index) {
        return (Histo) histograms.get(index);
    }

    public MonObject getMonObject(int index) {
        return (MonObject) histograms.get(index);
    }

    public ObservableList<Histo> getHistograms() {
        return histograms;
    }

    public void setHistograms(ObservableList<Histo> histograms) {
        this.histograms.clear();
        for (Histo histo : histograms) {
            addHisto(histo);
        }
    }

    public ObservableList<GShape> getShapes() {
        return shapes;
    }
    
    public void setShapes(ObservableList<GShape> shapes) {
        this.shapes.clear();
        for (GShape shape : shapes) {
           this.shapes.add(shape);
        }
    }
    
    public Legend getLegend() {
        return legend;
    }

    public void setColorPattern(String pattern) {
        colorPattern = pattern;
        if (colorAxis == null) {
            colorAxis = new GColorAxis();
        }
        ((GColorAxis)colorAxis).setColorPattern(pattern);
        resetPadding();
    }

    public String getColorPattern() {
        return colorPattern;
    }
    
    public String colorPatternProperty() {
        return colorPattern;
    }
    
    public void setMonObject(int index, Histo h) {
        histograms.set(index, h);
    }

    public void addHisto(Histo h) {
        if (!_use_pad) {
            _use_pad = true;
            try {
                getAxisX().get().copy(h.getAxisX());
                double xmax = getAxisX().get().getMax();
                double xmin = getAxisX().get().getMin();
                if (xmax < 10 && xmax > 5
                        && xmin > -10 && xmin < -5) {
                    getAxisX().setNdivisions(5);
                }
                getAxisY().get().copy(h.getAxisY());
                xmax = getAxisY().get().getMax();
                xmin = getAxisY().get().getMin();
                if (xmax < 10 && xmax > 5
                        && xmin > -10 && xmin < -5) {
                    getAxisY().setNdivisions(5);
                }
                if (getTitle().length() == 0) {
                    setTitle(h.getTitle());
                }
                if (stat) {
                    _stat_histo = h;
                }
            } catch (Exception e) {
            }
        }
        if (h.getDim() == 2) {
            if (colorAxis == null) {
                colorAxis = new GColorAxis(10, 0, 1, "", false, false, false);
            }
            colorAxis.setHand(GAxis.HAND_L);
            //resetPadding();
        } else if (h.getDataType().matches("TG.")) {
            TimedGraph1 graph = (TimedGraph1) h;
            axisx.setTimeScale(GAxis.TIME_SCALE_SEC);
            axisx.get().copy(graph.getAxisX());
            axisx.setTitle("Time");
            axisx.setLabelFontSize(0.7);
            axisx.setLabelAngle(45);
            axisx.setLabelAlignment("top");
        }
        histograms.add(h);
        resetPadding();
    }

    public void addShape(GShape shape) {
        shapes.add(shape);
    }

    public void addCShape(GShape shape) {
        cshapes.add(shape);
    }

    public void setLegend(Legend legend) {
        this.legend = legend;
    }

    public void update() {
        histograms.stream().forEach((histo) -> {
            histo.getDrawer().setCanvas(this);
            histo.getDrawer().update();
        });
        if (stat && _stat_histo != null) {
            if (_stat_rect == null) {
                _stat_rect = new GRect(0.71, 0.02, 0.28, 0.12, Color.WHITE, Color.BLACK);
                _stat_name = new GText(_stat_histo.getName(), 0.85, 0.055, "center");
                _stat_name.setFontSize(0.52);
                if (_stat_name.getText().length() > 20) {
                    _stat_name.setFontSize(0.47);
                }
                _stat_update = new GText("mean     : ", 0.72, 0.088, "left");
                _stat_update.setFontSize(0.52);
                _stat_entries = new GText("entries : " + _stat_histo.getEntries(), 0.72, 0.128, "left");
                _stat_entries.setFontSize(0.52);
                addShape(_stat_rect);
                addShape(_stat_name);
                addShape(_stat_update);
                addShape(_stat_entries);
            }
            if (_stat_histo.getDim() == 1) {
                String label = String.format("%1$.3f", _stat_histo.getMean());
                _stat_update.setText("mean     : " + label);
            } else if (_stat_histo.getDim() == 2) {
                Histo2 h = (Histo2) _stat_histo;
                String label = String.format("(%1$.3f, ", h.getMeanX());
                label += String.format("%1$.3f)", h.getMeanY());
                _stat_update.setText("mean     : " + label);
            }
            _stat_entries.setText("integral : " + ((int) _stat_histo.getEntries()));
        }
        panel.repaint();
    }

    public void draw(GraphicsDrawer canvas) {
        super.draw(canvas);
        //update();
        if (histograms.size() > 0) {
            if (_use_pad) {
                try {
                    Histo histo = histograms.get(0);
                    canvas.drawRect(pad.getX(), pad.getY(), pad.getWidth(), pad.getHeight());
                    getAxisX().get().copyRange(histo.getAxisX());
                    getAxisY().get().copyRange(histo.getAxisY());
                    _update_time = histo.getUpdateTime();
                    getAxisX().setUpdateTime(_update_time);
                    getAxisY().setUpdateTime(_update_time);
                    pad.draw(canvas);
                    if (colorAxis != null) {
                        colorAxis.get().copyRange(histo.getAxisZ());
                    }
                    for (Histo obj : histograms) {
                        obj.getDrawer().setCanvas(this);
                        obj.draw(canvas);
                    }
                    for (GShape shape : cshapes) {
                        shape.draw(canvas);
                    }
                    canvas.setFill(getFill());
                    canvas.setLine(null);
                    canvas.drawRect(0, 0, pad.getX(), 1);
                    canvas.drawRect(pad.getX() + pad.getWidth(), 0, 1 - (pad.getX() + pad.getWidth()), 1);
                    canvas.drawRect(0, 0, 1, pad.getY());
                    canvas.drawRect(0, pad.getY() + pad.getHeight(), 1, 1 - (pad.getY() + pad.getHeight()));
//                    canvas.drawRect(0, 0, pad.getX(), 1);
//                    canvas.drawRect(pad.getX() + pad.getWidth(), 0, 1 - (pad.getX() + pad.getWidth()), 1);
//                    canvas.drawRect(0, 0, 1, pad.getY());
//                    canvas.drawRect(0, pad.getY() + pad.getHeight(), pad.getX() + pad.getWidth(), 1 - (pad.getY() + pad.getHeight()));
                    canvas.setFill(null);
                    canvas.setLine(pad.getLine());
                    canvas.drawRect(pad.getX(), pad.getY(), pad.getWidth(), pad.getHeight());
                    _title.setFont(getFont());
                    _title.draw(canvas);
                    axisy.draw(canvas);
                    axisx.draw(canvas);
                    if (axisy2 != null) {
                        axisy2.draw(canvas);
                    }
                    if (colorAxis != null) {
                        colorAxis.draw(canvas);
                    }
                    if (legend != null) {
                        legend.draw(canvas);
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
            } else {
                try {
                    Histo histo = histograms.get(0);
                    if (colorAxis != null) {
                        colorAxis.get().copyRange(histo.getAxisY());
                        colorAxis.draw(canvas);
                    }
                } catch (Exception e) {
                }
                for (Histo obj : histograms) {
                    obj.getDrawer().setCanvas(this);
                    obj.draw(canvas);
                }
            }
        }
        for (GShape shape : shapes) {
            shape.draw(canvas);
        }
    }

    public void setPad(Pad pad) {
        this.pad = pad;
    }

    public void setAxisX(GAxis axis) {
        axisx = axis;
    }

    public void setAxisY(GAxis axis) {
        axisy = axis;
    }

    public GAxis getAxisX() {
        return axisx;
    }

    public GAxis getAxisY() {
        return axisy;
    }

    public GAxis getAxisY2() {
        return axisy2;
    }

    public String getTitle() {
        return _title.getText();
    }

    public void setTitlePosition(String pos) {
        if (pos.matches("center")) {
            _title_position = pos;
            _title.setX(0.5);
            _title.setAlignment("center top");
        } else if (pos.matches("left")) {
            _title_position = pos;
            _title.setX(0.04);
            _title.setAlignment("left top");
        } else if (pos.matches("right")) {
            _title_position = pos;
            _title.setX(0.96);
            _title.setAlignment("right top");
        }
    }

    public String getTitlePosition() {
        return _title_position;
    }

    public void setTitle(String title) {
        _title.setText(title);
        if (title.length() > 20) {
            setTitlePosition("left");
        }
        if (title.length() > 25) {
            setFontSize(1.0);
        }
        if (title.length() > 30) {
            setFontSize(0.9);
        }
        if (title.length() > 35) {
            setFontSize(0.8);
        }
        if (title.length() > 40) {
            setFontSize(0.7);
        }
        if (title.length() > 45) {
            setFontSize(0.6);
        }
        if (title.length() > 50) {
            setFontSize(0.55);
        }
    }

    public void setAxisY2(GAxis axis) {
        axisy2 = axis;
        axisy2.setHand(GAxis.HAND_L);
        resetPadding();
    }

    public GAxis getColorAxis() {
        return colorAxis;
    }

    public void setColorAxis(GAxis axis) {
        colorAxis = axis;
    }

    public boolean usePad() {
        return _use_pad;
    }

    public void setUpdateTime(long update_time) {
        _update_time = update_time;
    }

    public long getUpdateTime() {
        return _update_time;
    }

    public void setTitleOffset(double offset) {
        _title_offset = offset;
        _title.setY(pad.getY() / 3 - _title_offset);
    }

    public double getTitleOffset() {
        return _title_offset;
    }

    public boolean isStat() {
        return stat;
    }

    public void setStat(boolean use_stat) {
        stat = use_stat;
    }

    public void setxPanel(CanvasPanel panel) {
        this.panel = panel;
    }
}
