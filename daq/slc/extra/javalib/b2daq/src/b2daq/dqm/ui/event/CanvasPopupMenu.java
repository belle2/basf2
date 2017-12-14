package b2daq.dqm.ui.event;

import b2daq.dqm.graphics.HistogramCanvas;
import b2daq.dqm.ui.CanvasPanel;
import b2daq.ui.InputDialog;
import javafx.event.Event;
import javafx.event.EventHandler;
import javafx.scene.control.ContextMenu;
import javafx.scene.control.Menu;
import javafx.scene.control.MenuItem;

public class CanvasPopupMenu extends ContextMenu {

    private static final long serialVersionUID = 1L;
    private HistogramCanvas _canvas;
    private CanvasPanel _panel;

    public CanvasPopupMenu(String name, HistogramCanvas canvas, CanvasPanel panel) {
        //super(name);
        _canvas = canvas;
        _panel = panel;
        this.setAutoHide(false);
        Menu menu = new FontSettingMenu("Set Title", canvas.getFont(), panel);
        MenuItem item = new MenuItem("Set Position");
        item.setOnAction(new EventHandler() {
            @Override
            public void handle(Event event) {
                hide();
                String str = InputDialog.showDialog(getScene(), 
                        "Setting canvas", "Set title position",
                        _canvas.getTitlePosition());
                if (str != null) {
                    _canvas.setTitlePosition(str);
                    _panel.repaint();
                }
            }
        });
        menu.getItems().add(item);
        item = new MenuItem("Set Text");
        item.setOnAction(new EventHandler() {
            @Override
            public void handle(Event event) {
                hide();
                String str = InputDialog.showDialog(getScene(), 
                        "Setting canvas", "Set title",
                        _canvas.getTitle());
                if (str != null) {
                    _canvas.setTitle(str);
                    _panel.repaint();
                }
            }
        });
        menu.getItems().add(item);
        getItems().add(menu);
        menu = new LineSettingMenu("Set Line", canvas.getLine(), panel);
        getItems().add(menu);
        menu = new FillSettingMenu("Set Fill", canvas.getFill(), panel);
        getItems().add(menu);
    }

}
