package b2daq.dqm.ui.event;

import b2daq.dqm.ui.CanvasPanel;
import b2daq.graphics.FillProperty;
import b2daq.ui.InputDialog;
import javafx.event.ActionEvent;
import javafx.event.Event;
import javafx.event.EventHandler;
import javafx.scene.control.Menu;
import javafx.scene.control.MenuItem;
import javafx.scene.paint.Color;

public class FillSettingMenu extends CleanableMenu {

    private FillProperty _fill;
    private CanvasPanel _panel;

    public FillSettingMenu(String title, FillProperty fill, CanvasPanel panel) {
        super(title);
        _fill = fill;
        _panel = panel;
        MenuItem item = new MenuItem("Set Color");
        item.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent event) {
                cleanup();
                String str = InputDialog.showDialog(getParentPopup().getScene(),
                        "Setting fill", "Set fill color",
                        _fill.getColor() != null ? _fill.getColor().toString() : "");
                if (str != null) {
                    _fill.setColor(Color.web(str));
                    _panel.repaint();
                }
            }
        });
        getItems().add(item);
        item = new MenuItem("Set Transparency");
        item.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent event) {
                cleanup();
                String str = InputDialog.showDialog(getParentPopup().getScene(),
                        "Setting fill", "Set fill transparency",
                        "" + _fill.getTransparency());
                if (str != null) {
                    _fill.setTransparency(Double.parseDouble(str));
                    _panel.repaint();
                }
            }
        });
        getItems().add(item);
        item = new MenuItem("Set Style");
        item.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent event) {
                cleanup();
                String str = InputDialog.showDialog(getParentPopup().getScene(),
                        "Setting fill", "Set fill transparency",
                        _fill.getStyle());
                if (str != null) {
                    _fill.setStyle(str);
                    _panel.repaint();
                }
            }
        });
        getItems().add(item);
    }

}
