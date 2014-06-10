/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.pscontrol;

import b2daq.core.LogLevel;
import b2daq.logger.core.LogMessage;
import b2daq.logger.ui.LogViewPaneController;
import b2daq.nsm.NSMListenerService;
import b2daq.nsm.NSMMessage;
import b2daq.hvcontrol.core.HVCommand;
import b2daq.hvcontrol.core.HVState;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.fxml.FXML;
import javafx.scene.control.ContextMenu;
import javafx.scene.control.MenuItem;
import javafx.scene.input.MouseEvent;
import javafx.scene.paint.Color;
import javafx.scene.paint.Paint;
import javafx.scene.shape.Rectangle;
import javafx.scene.text.Text;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class PowerStateLabelController {

    @FXML
    private Rectangle rect;
    @FXML
    private Text text;

    private final ContextMenu menu = new ContextMenu();
    private final MenuItem boot = new MenuItem("TURNOFF");
    private final MenuItem load = new MenuItem("STANDBY");
    private final MenuItem recover = new MenuItem("SHOULDER");
    private final MenuItem abort = new MenuItem("PEAK");
    private String m_nodename = "";
    private LogViewPaneController m_logview;
    private int m_crate;
    private final HVState state_org = new HVState();

    public PowerStateLabelController(Rectangle rect, Text text) {
        this.rect = rect;
        this.text = text;
        setVisible(false);
    }
    
    public final void setVisible(boolean enabled) {
        rect.setVisible(enabled);
        text.setVisible(enabled);
    }
    
    public void set(String nodename, int crate, LogViewPaneController logview) {
        setVisible(true);
        m_nodename = nodename;
        m_crate = crate;
        m_logview = logview;
        menu.getItems().addAll(boot, load, recover, abort);
        boot.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent event) {
                NSMListenerService.request(new NSMMessage(m_nodename, HVCommand.TURNOFF));
                m_logview.add(new LogMessage("LOCAL", LogLevel.NOTICE, "Command TURNOFF>>"+m_nodename));
            }
        });
        load.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent event) {
                NSMListenerService.request(new NSMMessage(m_nodename, HVCommand.STANDBY));
                m_logview.add(new LogMessage("LOCAL", LogLevel.NOTICE, "Command STANDBY>>"+m_nodename));
            }
        });
        recover.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent event) {
                NSMListenerService.request(new NSMMessage(m_nodename, HVCommand.SHOULDER));
                m_logview.add(new LogMessage("LOCAL", LogLevel.NOTICE, "Command SHOULDER>>"+m_nodename));
            }
        });
        abort.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent event) {
                NSMListenerService.request(new NSMMessage(m_nodename, HVCommand.PEAK));
                m_logview.add(new LogMessage("LOCAL", LogLevel.NOTICE, "Command PEAK>>"+m_nodename));
            }
        });

        text.setOnMouseClicked(new EventHandler<MouseEvent>() {
            @Override
            public void handle(MouseEvent e) {
                if (text.getText().matches("INITIAL")) {
                    boot.setDisable(false);
                    load.setDisable(true);
                    recover.setDisable(false);
                } else if (text.getText().matches("CONFIGURED")) {
                    boot.setDisable(true);
                    load.setDisable(false);
                    recover.setDisable(false);
                } else if (text.getText().matches("READY")) {
                    boot.setDisable(true);
                    load.setDisable(false);
                    recover.setDisable(false);
                } else if (text.getText().matches("RUNNING")) {
                    boot.setDisable(true);
                    load.setDisable(true);
                    recover.setDisable(true);
                } else if (text.getText().matches("SHOULDERING")) {
                    boot.setDisable(true);
                    load.setDisable(true);
                    recover.setDisable(true);
                } else if (text.getText().matches("PEAKING")) {
                    boot.setDisable(true);
                    load.setDisable(true);
                    recover.setDisable(true);
                }
                menu.show(text, e.getScreenX(), e.getScreenY());
            }
        });
    }

    public String getText() {
        return text.getText();
    }

    public void setText(String value) {
        text.setText(value);
    }

    public void set(String value, Paint line, Paint fill, Paint font) {
        rect.setFill(fill);
        rect.setStroke(line);
        text.setText(value);
        text.setFill(font);
    }

    void setFont(Paint font) {
        text.setFill(font);
    }

    public boolean update(int value) {
        HVState state = new HVState();
        state.copy(value);
        return update(state);
    }

    public boolean update(HVState state) {
        boolean renewed = (state.equals(state_org));
        state_org.copy(state);
        if (state.equals(HVState.OFF_S)) {
            set(state.getLabel(), Color.WHITESMOKE, Color.LIGHTGRAY, Color.BLACK);
        } else if (state.equals(HVState.PEAK_S)) {
            set(state.getLabel(), Color.LIGHTGREEN, Color.LIMEGREEN, Color.WHITE);
        } else if (state.isStable()) {
            set(state.getLabel(), Color.CYAN, Color.DEEPSKYBLUE, Color.WHITE);
        } else if (state.isTransition()) {
            set(state.getLabel(), Color.PINK, Color.MAGENTA, Color.WHITE);
        } else {
            set(state.getLabel(), Color.BLACK, Color.GRAY, Color.WHITE);
        }
        return renewed;
    }

}
