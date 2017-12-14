package b2daq.dqm.ui.event;

import b2daq.dqm.graphics.Pad;
import b2daq.dqm.ui.CanvasPanel;
import javafx.event.Event;
import javafx.event.EventHandler;
import javafx.scene.control.Menu;

public class PadPopupMenu extends Menu {

    public PadPopupMenu(String name, Pad pad, CanvasPanel panel) {
        super(name);
        Menu menu = new PadBoundsSettingMenu("Set Bounds", pad, panel);
        getItems().add(menu);
        menu = new LineSettingMenu("Set Line", pad.getLine(), panel);
        getItems().add(menu);
        menu = new FillSettingMenu("Set Fill", pad.getFill(), panel);
        getItems().add(menu);
    }

    private static final long serialVersionUID = 1L;

}
