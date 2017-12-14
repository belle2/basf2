package b2daq.dqm.ui.event;

import b2daq.dqm.core.Histo;
import b2daq.dqm.graphics.GHisto;
import b2daq.dqm.ui.CanvasPanel;
import b2daq.ui.InputDialog;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.scene.control.Menu;
import javafx.scene.control.MenuItem;

public class HistoMenu extends CleanableMenu {

    private static final long serialVersionUID = 1L;
    private Histo _histo = null;
    private CanvasPanel _panel;

    public HistoMenu(Histo histo, CanvasPanel panel) {
        super(histo.getName());
        _histo = histo;
        _panel = panel;
        MenuItem item = new MenuItem("Set Title");
        item.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent event) {
                cleanup();
                String str = InputDialog.showDialog(getParentPopup().getScene(), 
                        "Setting histogram", "Set title of "+histo.getName(), 
                        histo.getTitle());
                if (str != null) {
                    histo.setTitle(str);
                    _panel.repaint();
                }
            }
        });
        getItems().add(item);
        Menu menu = new FontSettingMenu("Set Font", _histo.getFont(), panel);
        getItems().add(menu);
        menu = new LineSettingMenu("Set Line", _histo.getLine(), panel);
        getItems().add(menu);
        menu = new FillSettingMenu("Set Fill", _histo.getFill(), panel);
        getItems().add(menu);
        item = new MenuItem("Set Draw Option");
        item.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent event) {
                cleanup();
                String str = InputDialog.showDialog(getParentPopup().getScene(), 
                        "Setting histogram", "Set draw option for "+_histo.getName(), 
                        _histo.getDraw());
                if (str != null) {
                    _histo.setDraw(str);
                    _panel.repaint();
                }
            }
        });
        getItems().add(item);
    }

}
