package b2daq.graphics;

import javafx.beans.property.DoubleProperty;
import javafx.beans.property.SimpleDoubleProperty;
import javafx.beans.property.SimpleStringProperty;
import javafx.beans.property.StringProperty;
import javafx.scene.paint.Paint;
import javafx.scene.paint.Color;
import javafx.scene.text.FontWeight;

public class FontProperty {

    static public int WEIGHT_NORMAL = 0;
    static public int WEIGHT_BOLD = 1;

    private DoubleProperty size = new SimpleDoubleProperty(1);
    private FontWeight weight;
    private StringProperty family = new SimpleStringProperty("");
    private Paint color = Color.BLACK;
    private Paint background = null;

    public FontProperty() {
        this(Color.BLACK, "Helvetica", 1, FontWeight.BOLD);
    }

    public FontProperty(Paint color, Paint bcolor, String family, double size, FontWeight weight) {
        this.family.set(family);
        this.color = color;
        this.background = bcolor;
        this.size.set(size);
        this.weight = weight;
    }

    public FontProperty(Paint color, String family, double size, FontWeight weight) {
        this(color, null, family, size, weight);
    }

    public FontProperty(Paint color, Paint bcolor) {
        this(color, bcolor, "Helvetica", 1, FontWeight.NORMAL);
    }

    public double getSize() {
        return size.get();
    }

    public FontWeight getWeight() {
        return weight;
    }

    public String getFamily() {
        return family.get();
    }

    public Paint getColor() {
        return color;
    }

    public Paint getBackground() {
        return background;
    }

    public void setSize(double d) {
        size.set(d);
    }

    public void setWeight(FontWeight weight) {
        this.weight = weight;
    }

    public void setColor(Paint c) {
        color = c;
    }

    public void setBackground(Paint c) {
        background = c;
    }

    public void setFamily(String family) {
        this.family.set(family);
    }

}
