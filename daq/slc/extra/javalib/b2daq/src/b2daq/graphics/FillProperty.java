package b2daq.graphics;

import javafx.scene.paint.Paint;

public class FillProperty {

    private Paint color;
    private double transparency;

    public FillProperty() {
        this(null, 1);
    }

    public FillProperty(Paint c) {
        this(c, 1);
    }

    public FillProperty(Paint c, int t) {
        color = c;
        transparency = t;
    }

    public void setColor(Paint c) {
        color = c;
    }

    public void setTransparency(double t) {
        transparency = t;
    }

    public Paint getColor() {
        return color;
    }

    public double getTransparency() {
        return transparency;
    }

    public String getStyle() {
        return "";
    }

    public void setStyle(String str) {
    }

    public void copy(FillProperty pro) {
        if (pro == null) {
            return;
        }
        transparency = pro.transparency;
        color = pro.color;
    }

}
