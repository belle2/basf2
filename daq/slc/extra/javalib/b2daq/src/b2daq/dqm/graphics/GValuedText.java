package b2daq.dqm.graphics;

import b2daq.graphics.GText;
import javafx.beans.property.DoubleProperty;
import javafx.beans.property.SimpleDoubleProperty;
import javafx.scene.paint.Color;

public class GValuedText extends GText {

    private final DoubleProperty value = new SimpleDoubleProperty(0);

    public GValuedText() {
    }

    public GValuedText(String str, double value) {
        super(str, 0, 0, "left top");
        this.value.set(value);
    }

    public GValuedText(String str, double value, String align) {
        super(str, 0, 0, align);
        this.value.set(value);
    }

    public GValuedText(String str, double value, String align,
            Color font_color) {
        super(str, 0, 0, align, font_color);
        this.value.set(value);
    }

    public GValuedText(String str, double value, String align,
            double angle, Color font_color) {
        super(str, 0, 0, align, angle, font_color);
        this.value.set(value);
    }

    public void setValue(double value) {
        this.value.set(value);
    }

    public double getValue() {
        return value.get();
    }

    public DoubleProperty valueProperty() {
        return value;
    }
    
}
