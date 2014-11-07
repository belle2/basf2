/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.runcontrol;

import b2daq.core.LogLevel;
import b2daq.logger.core.LogMessage;
import b2daq.logger.ui.LogViewPaneController;
import b2daq.nsm.NSMListenerService;
import b2daq.nsm.NSMMessage;
import b2daq.runcontrol.core.RCCommand;
import b2daq.runcontrol.core.RCState;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
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
public class RunStateLabelController {

    private final Rectangle rect;
    private final Text text;

    private final ContextMenu menu = new ContextMenu();
    private final MenuItem exclude = new MenuItem("EXCLUDE");
    private final MenuItem load = new MenuItem("LOAD");
    private final MenuItem recover = new MenuItem("RECOVER");
    private final MenuItem abort = new MenuItem("ABORT");
    private String nodename;
    private LogViewPaneController m_logview;
    private boolean m_used = false;

    public RunStateLabelController(Rectangle rect, Text text) {
        this.nodename = "";
        this.rect = rect;
        this.text = text;
        setVisible(false);
    }

    public final void setVisible(boolean enabled) {
        rect.setVisible(enabled);
        text.setVisible(enabled);
    }

    public void set(String nodename, LogViewPaneController logview) {
        setVisible(true);
        this.nodename = nodename;
        m_logview = logview;
        menu.getItems().addAll(exclude, load, recover, abort);
        load.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent event) {
                NSMListenerService.request(new NSMMessage(nodename, RCCommand.LOAD));
                m_logview.add(new LogMessage("LOCAL", LogLevel.NOTICE, "Command LOAD >> " + nodename));
            }
        });
        recover.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent event) {
                NSMListenerService.request(new NSMMessage(nodename, RCCommand.RECOVER));
                m_logview.add(new LogMessage("LOCAL", LogLevel.NOTICE, "Command RECOVER >> " + nodename));
            }
        });
        abort.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent event) {
                NSMListenerService.request(new NSMMessage(nodename, RCCommand.ABORT));
                m_logview.add(new LogMessage("LOCAL", LogLevel.NOTICE, "Command ABORT >> " + nodename));
            }
        });
        exclude.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent event) {
                String rcnodename = NSMListenerService.getNSMConfig().getNsmTarget();
                if (exclude.getText().matches("EXCLUDE")) {
                    NSMListenerService.request(new NSMMessage(rcnodename, RCCommand.EXCLUDE, nodename));
                    m_logview.add(new LogMessage("LOCAL", LogLevel.NOTICE, "Command EXCLUDE >> " + nodename));
                } else {
                    NSMListenerService.request(new NSMMessage(rcnodename, RCCommand.INCLUDE, nodename));
                    m_logview.add(new LogMessage("LOCAL", LogLevel.NOTICE, "Command INCLUDE >> " + nodename));
                }
            }
        });

        text.setOnMouseClicked(new EventHandler<MouseEvent>() {
            @Override
            public void handle(MouseEvent e) {
                if (!m_used) {
                    exclude.setText("INCLUDE");
                    load.setDisable(true);
                    recover.setDisable(true);
                    abort.setDisable(true);
                } else {
                    exclude.setText("EXCLUDE");
                    abort.setDisable(false);
                    if (text.getText().matches("NOTREADY")) {
                        load.setDisable(false);
                        recover.setDisable(false);
                        exclude.setDisable(false);
                    } else if (text.getText().matches("READY")) {
                        load.setDisable(false);
                        recover.setDisable(false);
                        exclude.setDisable(false);
                    } else if (text.getText().matches("RUNNING")) {
                        load.setDisable(true);
                        recover.setDisable(false);
                        exclude.setDisable(true);
                    } else if (text.getText().matches("RECOVERING")) {
                        load.setDisable(true);
                        recover.setDisable(true);
                    } else if (text.getText().matches("ABORTING")) {
                        load.setDisable(true);
                        recover.setDisable(true);
                    }
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

    public void update(int value, boolean used) {
        RCState state = new RCState();
        state.copy(value);
        update(state, used);
    }

    public void update(int value) {
        RCState state = new RCState();
        state.copy(value);
        update(state);
    }

    public void update(RCState state, boolean used) {
        m_used = used;
        update(state);
    }

    public void update(RCState state) {
        if (!m_used) {
            set(RCState.OFF_S.getLabel(), Color.WHITESMOKE, Color.LIGHTGRAY, Color.BLACK);
        } else if (state.equals(RCState.OFF_S)) {
            set(state.getLabel(), Color.WHITESMOKE, Color.LIGHTGRAY, Color.BLACK);
        } else if (state.equals(RCState.READY_S)) {
            set(state.getLabel(), Color.YELLOW, Color.YELLOW, Color.BLACK);
        } else if (state.equals(RCState.NOTREADY_S)) {
            set(state.getLabel(), Color.RED, Color.RED, Color.BLACK);
        } else if (state.equals(RCState.RUNNING_S)) {
            set(state.getLabel(), Color.CYAN, Color.CYAN, Color.BLACK);
        } else if (state.isTransition()) {
            set(state.getLabel(), Color.PINK, Color.MAGENTA, Color.WHITE);
        } else {
            set(state.getLabel(), Color.BLACK, Color.GRAY, Color.WHITE);
        }
    }

}
