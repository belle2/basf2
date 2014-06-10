package b2daq.dqm.graphics;

import b2daq.dqm.core.Histo;
import b2daq.graphics.GraphicsDrawer;
import b2daq.graphics.GRect;
import javafx.beans.property.SimpleStringProperty;
import javafx.beans.property.StringProperty;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.scene.paint.Color;

public class Legend extends GRect {

    public static final int DIRECTION_Y = 0;
    public static final int DIRECTION_X = 1;
    
    private final StringProperty title = new SimpleStringProperty("");
    private final ObservableList<Histo> histograms = FXCollections.observableArrayList();
    private int direction;

    public Legend() {
        x = 0.68;
        y = 0.02;
        width = 0.26;
        height = 0.08;
        setFillColor(Color.WHITE);
        setLineColor(Color.BLACK);
        font.setSize(0.9);
    }

    public void setBounds(double x, double y, double width, double height) {
        this.x = x;
        this.y = y;
        this.width = width;
        this.height = height;
    }

    public void setDirection(int direction) {
        this.direction = direction;
    }
    
    public void setDirection(String direction) {
        if (direction.matches("x")) {
            this.direction = DIRECTION_X;
        } else {
            this.direction = DIRECTION_Y;
        }
    }
    
    public String getDirection() {
        if (this.direction == DIRECTION_X) {
            return "x";
        } else {
            return "y";
        }
    }

    public void add(Histo h) {
        histograms.add(h);
        height = histograms.size() * 0.03 + 0.02;
    }

    public void setTitle(String title) {
        this.title.set(title);
    }

    public String getTitle() {
        return title.get();
    }

    public ObservableList<Histo> getHistograms() {
        return histograms;
    }

    public void setHistograms(ObservableList<GHisto> histograms) {
        histograms.addAll(histograms);
    }

    @Override
    public void draw(GraphicsDrawer canvas) {
        canvas.setFont(getFont());
        canvas.setFill(getFill());
        canvas.setLine(getLine());
        canvas.drawRect(x, y, width, height);
        int n = 0;
        for (Histo h : histograms) {
            canvas.setFill(h.getFill());
            canvas.setLine(h.getLine());
            if (h.getDraw().matches(".*L.*")) {
                canvas.drawLine(x + 0.02, y + 0.01 * (2.5 + n * 3), x + 0.06, y + 0.01 * (2.5 + n * 3));
                if (h.getFill() != null && h.getFill().getColor() != null) {
                    canvas.drawRect(x + 0.02, y + 0.01 * (1.5 + n * 3), 0.055, 0.02);
                }
            }
            if (h.getDraw().matches(".*P.*")) {
                canvas.drawMarker(x + 0.03, y + 0.01 * (2.5 + n * 3));
            }
            canvas.drawString(h.getTitle(), x + 0.08, y + 0.01 * (2.5 + n * 3), "left middle");
            n++;
        }
    }

}
