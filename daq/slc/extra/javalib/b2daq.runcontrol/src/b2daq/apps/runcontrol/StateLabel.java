/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.runcontrol;

import b2daq.runcontrol.core.RCState;
import javafx.beans.InvalidationListener;
import javafx.beans.Observable;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.geometry.VPos;
import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.control.ContextMenu;
import javafx.scene.control.MenuItem;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.AnchorPane;
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
public class StateLabel extends AnchorPane {

    private final Canvas canvas = new Canvas();
    private Paint line = Color.BLACK;
    private Paint fill = Color.GRAY;
    private Paint font = Color.BLACK;
    private String text = "UNKNOWN";

    private final ContextMenu menu = new ContextMenu();
    private final MenuItem boot = new MenuItem("BOOT");
    private final MenuItem load = new MenuItem("LOAD");
    private final MenuItem recover = new MenuItem("RECOVER");
    private final MenuItem abort = new MenuItem("ABORT");
    private String m_nodename = "";

    public final void repaint() {
        double width = getWidth();
        double height = getHeight();
        canvas.setWidth(width);
        canvas.setHeight(height);
        GraphicsContext c = canvas.getGraphicsContext2D();
        c.setFill(fill);
        c.fillRoundRect(5, 5, width - 10, height - 10, 4, 4);
        c.setStroke(line);
        c.strokeRoundRect(5, 5, width - 10, height - 10, 4, 4);
        c.setFill(font);
        c.setTextAlign(TextAlignment.CENTER);
        c.setTextBaseline(VPos.CENTER);
        c.setFont(Font.font("Sans", FontWeight.BOLD, 13));
        c.fillText(text, Math.round(width / 2), Math.round(height / 2));
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
        final ContextMenu cmenu = new ContextMenu();
        final AnchorPane pane = this;
        MenuItem boot = new MenuItem("BOOT");
        MenuItem load = new MenuItem("LOAD");
        MenuItem recover = new MenuItem("RECOVER");
        MenuItem abort = new MenuItem("ABORT");
        cmenu.getItems().addAll(boot, load, recover, abort);
        boot.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent event) {
                System.out.println("Cut...");
            }
        });
        setOnMouseClicked(new EventHandler<MouseEvent>() {
            @Override
            public void handle(MouseEvent e) {
                cmenu.show(pane, e.getScreenX(), e.getScreenY());
            }

        });
    }

    public String getText() {
        return text;
    }

    public void setText(String value) {
        text = value;
    }

    public void set(String value, Paint line, Paint fill, Paint font) {
        text = value;
        this.font = font;
        this.fill = fill;
        this.line = line;
    }

    public void update(RCState state) {
        if (state.equals(RCState.RUNNING_S)) {
            set(state.getLabel(), Color.LIMEGREEN, Color.GREEN, Color.WHITE);
        } else if (state.isStable()) {
            set(state.getLabel(), Color.CYAN, Color.BLUE, Color.WHITE);
        } else if (state.isTransition()) {
            set(state.getLabel(), Color.YELLOW, Color.GOLD, Color.WHITE);
        } else if (state.isStable()) {
            set(state.getLabel(), Color.PLUM, Color.PURPLE, Color.WHITE);
        } else {
            set(state.getLabel(), Color.LIGHTGRAY, Color.GRAY, Color.WHITE);
        }
    }
}
