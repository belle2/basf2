package b2daq.dqm.ui;

import b2daq.core.Updatable;
import b2daq.dqm.core.Histo;
import b2daq.dqm.graphics.HistogramCanvas;
import b2daq.dqm.graphics.Legend;
import b2daq.dqm.ui.event.CanvasPopupHandler;
import b2daq.dqm.ui.event.CanvasToolTipHandler;
import b2daq.graphics.FXGraphicsDrawer;
import b2daq.graphics.GraphicsDrawer;
import b2daq.graphics.GShape;
import javafx.beans.InvalidationListener;
import javafx.beans.Observable;
import javafx.beans.property.ObjectProperty;
import javafx.beans.property.SimpleObjectProperty;
import javafx.event.Event;
import javafx.event.EventHandler;
import javafx.scene.Cursor;
import javafx.scene.canvas.Canvas;
import javafx.scene.input.MouseButton;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.AnchorPane;

public class CanvasPanel extends AnchorPane implements Updatable {

    private final ObjectProperty<HistogramCanvas> canvas = new SimpleObjectProperty<>(new HistogramCanvas());
    private FXGraphicsDrawer _graphics = null;
    private final Canvas _raw_canvas = new Canvas();
    private final CanvasToolTipHandler _tip = new CanvasToolTipHandler();
    private final CanvasPopupHandler _popup = new CanvasPopupHandler();

    public CanvasPanel(HistogramCanvas canvas, double width, double height) {
        _graphics = new FXGraphicsDrawer();
        _graphics.setGraphics(_raw_canvas.getGraphicsContext2D());
        this.canvas.set(canvas);
        canvas.setxPanel(this);
        getChildren().add(_raw_canvas);
        setId(this.canvas.get().getName());
        setMinSize(width, height);
        setPrefSize(width, height);
        initEventHanlers();
        repaint();
    }

    public CanvasPanel(HistogramCanvas canvas) {
        this(canvas, 100,100);
    }

    public CanvasPanel(String name, String title, double width, double height) {
        this(new HistogramCanvas(name, title, width, height), width, height);
    }

    public CanvasPanel() {
        this("", "", 100, 100);
    }

    public CanvasPanel(String name, String title) {
        this(name, title, 100, 100);
    }

    public HistogramCanvas getCanvas() {
        return this.canvas.get();
    }

    public Histo getHisto(int index) {
        return getCanvas().getHisto(index);
    }

    public int getNHistos() {
        return getCanvas().getNHistos();
    }

    public void addHisto(Histo obj) {
        getCanvas().addHisto(obj);
    }
    
    public void addShape(GShape shape) {
        getCanvas().addShape(shape);
    }

    public void addCShape(GShape shape) {
        getCanvas().addShape(shape);
    }

    public void setLegend(Legend legend) {
        getCanvas().setLegend(legend);
    }
    
    @Override
    public void setPrefSize(double width, double height) {
        super.setPrefSize(width, height);
        repaint();
    }

    public void repaint() {
        _graphics.setGraphics(_raw_canvas.getGraphicsContext2D());
        _graphics.setWidth(getWidth());
        _graphics.setHeight(getHeight());
        _graphics.setFontRealSize(Math.max(getHeight(), getWidth())/20);
        getCanvas().draw(_graphics);
    }

    @Override
    public void update() {
        canvas.get().resetPadding();
        repaint();
    }

    public GraphicsDrawer getGCanvas() {
        return _graphics;
    }

    private void initEventHanlers() {
        InvalidationListener listener = new InvalidationListener() {
            @Override
            public void invalidated(Observable o) {
                _raw_canvas.setWidth(getWidth());
                _raw_canvas.setHeight(getHeight());
                repaint();
            }
        };
        widthProperty().addListener(listener);
        heightProperty().addListener(listener);
        setOnMouseEntered(new EventHandler() {
            @Override
            public void handle(Event event) {
            }
        });
        setOnMouseExited(new EventHandler() {
            @Override
            public void handle(Event event) {
                getCanvas().getAxisY().mouseReleased();
                getCanvas().getAxisX().mouseReleased();
                if (getCanvas().getAxisY2() != null) {
                    getCanvas().getAxisY2().mouseReleased();
                }
                setCursor(Cursor.DEFAULT);
                repaint();
            }
        });
        setOnMousePressed(new EventHandler() {
            @Override
            public void handle(Event event) {
                MouseEvent mouse = (MouseEvent) event;
                if (mouse.isPrimaryButtonDown()) {
                    CanvasPanel panel = (CanvasPanel) event.getSource();
                    double x = mouse.getX() / panel.getWidth();
                    double y = mouse.getY() / panel.getHeight();
                    double x1 = getCanvas().getPad().getX();
                    double y1 = getCanvas().getPad().getY();
                    double x2 = getCanvas().getPad().getX() + getCanvas().getPad().getWidth();
                    double y2 = getCanvas().getPad().getY() + getCanvas().getPad().getHeight();
                    if (getCanvas().usePad()) {
                        if (getCanvas().getAxisY().hit(x, y)) {
                            getCanvas().getAxisY().mousePressed(x1, y, x2, y);
                        } else if (getCanvas().getAxisY2() != null
                                && getCanvas().getAxisY2().hit(x, y)) {
                            getCanvas().getAxisY2().mousePressed(x1, y, x2, y);
                        } else if (getCanvas().getAxisX().hit(x, y)) {
                            getCanvas().getAxisX().mousePressed(x, y1, x, y2);
                        }
                    }
                    if (getCanvas().getColorAxis() != null && getCanvas().getColorAxis().hit(x, y)) {
                        getCanvas().getColorAxis().setFocused(true);
                    }
                    repaint();
                    _tip.show(mouse);
                }
            }
        });
        setOnMouseReleased(new EventHandler() {
            @Override
            public void handle(Event event) {
                getCanvas().getAxisY().mouseReleased();
                getCanvas().getAxisX().mouseReleased();
                if (getCanvas().getAxisY2() != null) {
                    getCanvas().getAxisY2().mouseReleased();
                }
                repaint();
            }
        });
        setOnMouseDragged(new EventHandler() {
            @Override
            public void handle(Event event) {
                CanvasPanel panel = (CanvasPanel) event.getSource();
                MouseEvent mouse = (MouseEvent) event;
                double x = mouse.getX() / panel.getWidth();
                double y = mouse.getY() / panel.getHeight();
                double x1 = getCanvas().getPad().getX();
                double y1 = getCanvas().getPad().getY();
                double x2 = getCanvas().getPad().getX() + getCanvas().getPad().getWidth();
                double y2 = getCanvas().getPad().getY() + getCanvas().getPad().getHeight();
                if (getCanvas().usePad()) {
                    if (getCanvas().getAxisY().hit(x, y)) {
                        getCanvas().getAxisY().mouseDragged(x1, y, x2, y);
                    } else if (getCanvas().getAxisY2() != null
                            && getCanvas().getAxisY2().hit(x, y)) {
                        getCanvas().getAxisY2().mouseDragged(x1, y, x2, y);
                    } else if (getCanvas().getAxisX().hit(x, y)) {
                        getCanvas().getAxisX().mouseDragged(x, y1, x, y2);
                    }
                }
                if (getCanvas().getColorAxis() != null && getCanvas().getColorAxis().hit(x, y)) {
                    getCanvas().getColorAxis().setFocused(true);
                }
                repaint();
                _tip.hide();
            }
        });

        setOnMouseMoved(new EventHandler() {
            @Override
            public void handle(Event event) {
                CanvasPanel panel = (CanvasPanel) event.getSource();
                MouseEvent mouse = (MouseEvent) event;
                double x = mouse.getX() / panel.getWidth();
                double y = mouse.getY() / panel.getHeight();
                if ((getCanvas().usePad() && (getCanvas().getAxisY().hit(x, y)
                        || getCanvas().getAxisX().hit(x, y) 
                        || (getCanvas().getAxisY2() != null 
                        && getCanvas().getAxisY2().hit(x, y))))
                        || (getCanvas().getColorAxis() != null 
                        && getCanvas().getColorAxis().hit(x, y))) {
                    panel.setCursor(Cursor.CLOSED_HAND);
                } else {
                    panel.setCursor(Cursor.DEFAULT);
                }
                _tip.hide();
            }
        });
        setOnMouseClicked(new EventHandler() {
            @Override
            public void handle(Event event) {
                MouseEvent mouse = (MouseEvent) event;
                if (mouse.getButton() == MouseButton.SECONDARY) {
                    _popup.setPopup(mouse);
                }
            }
        });

    }
    
    public void setCanvas(HistogramCanvas canvas) {
        this.canvas.set(canvas);
        canvas.setxPanel(this);
    }
}
