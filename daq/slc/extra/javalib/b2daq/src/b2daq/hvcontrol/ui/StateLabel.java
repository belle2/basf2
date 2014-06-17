/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.hvcontrol.ui;

import b2daq.hvcontrol.core.HVState;
import javafx.beans.InvalidationListener;
import javafx.beans.Observable;
import javafx.beans.property.SimpleStringProperty;
import javafx.beans.property.StringProperty;
import javafx.geometry.VPos;
import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.layout.Pane;
import javafx.scene.paint.Color;
import javafx.scene.paint.Paint;
import javafx.scene.text.Font;
import javafx.scene.text.FontWeight;
import javafx.scene.text.TextAlignment;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public final class StateLabel extends Pane {

    private final Canvas canvas = new Canvas();
    private Paint line = Color.TRANSPARENT;
    private Paint fill = Color.TRANSPARENT;
    private Paint font = Color.BLACK;
    private final StringProperty text = new SimpleStringProperty("DOWN");
    private final HVState state_org = new HVState();
    private final StringProperty name = new SimpleStringProperty("");
    
    public final void repaint() {
        double width = getWidth();
        double height = getHeight();
        GraphicsContext c = canvas.getGraphicsContext2D();
        c.clearRect(0, 0, canvas.getWidth(), canvas.getHeight());
        canvas.setWidth(width);
        canvas.setHeight(height);
        c.setFill(fill);
        c.fillRoundRect(5, 5, width - 10, height - 10, 4, 4);
        c.setStroke(line);
        c.strokeRoundRect(5, 5, width - 10, height - 10, 4, 4);
        c.setFill(font);
        c.setTextAlign(TextAlignment.CENTER);
        c.setTextBaseline(VPos.CENTER);
        c.setFont(Font.font("Sans", FontWeight.BOLD, 13));
        c.fillText(text.get(), Math.round(width / 2), Math.round(height / 2));
    }

    public StateLabel() {
        getChildren().add(canvas);
        InvalidationListener listener = (Observable o) -> {
            repaint();
        };
        setMaxWidth(120);
        setMaxHeight(36);
        widthProperty().addListener(listener);
        heightProperty().addListener(listener);
        update(HVState.get(0));
        repaint();
    }

    public StateLabel(String name) {
        this();
        this.name.set(name);
    }
    
    public String getText() {
        return text.get();
    }

    public void setText(String value) {
        text.set(value);
    }

    public StringProperty textProperty() {
        return text;
    }
    
    public void set(String value, Paint line, Paint fill, Paint font) {
        text.set(value);
        this.font = font;
        this.fill = fill;
        this.line = line;
        repaint();
    }

    public String getName() {
        return name.get();
    }

    public void setName(String value) {
        name.set(value);
    }

    public StringProperty nameProperty() {
        return name;
    }
    
    public boolean update(HVState state) {
        boolean renewed = (state.equals(state_org));
        state_org.copy(state);
        if (state.equals(HVState.OFF_S)) {
            set(state.getLabel(), Color.RED, Color.RED, Color.BLACK);
        } else if (state.equals(HVState.STANDBY_S)) {
            set(state.getLabel(), Color.YELLOW, Color.YELLOW, Color.BLACK);
        } else if (state.equals(HVState.SHOULDER_S)) {
            set(state.getLabel(), Color.rgb(0, 176, 80), Color.rgb(0, 176, 80), Color.BLACK);
        } else if (state.equals(HVState.PEAK_S)) {
            //set(state.getLabel(), Color.LIGHTGREEN, Color.LIMEGREEN, Color.WHITE);
            set(state.getLabel(), Color.CYAN, Color.CYAN, Color.BLACK);
        } else if (state.isTransition()) {
            set(state.getLabel(), Color.ORANGE, Color.ORANGE, Color.BLACK);
        } else {
            set(state.getLabel(), Color.BLACK, Color.BLACK, Color.WHITE);
        }
        return renewed;
    }

}
