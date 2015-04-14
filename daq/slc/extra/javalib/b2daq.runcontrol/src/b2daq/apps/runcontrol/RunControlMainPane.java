/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.runcontrol;

import b2daq.logger.core.LogMessage;
import b2daq.logger.ui.LogViewPaneController;
import b2daq.nsm.NSMCommand;
import b2daq.nsm.NSMCommunicator;
import b2daq.nsm.NSMLogHandler;
import b2daq.nsm.NSMMessage;
import b2daq.nsm.NSMVSetHandler;
import b2daq.nsm.NSMVar;
import b2daq.nsm.ui.NSMRequestHandlerUI;
import b2daq.runcontrol.core.RCState;
import b2daq.runcontrol.ui.RCStateLabel;
import java.io.IOException;
import static java.lang.Integer.MAX_VALUE;
import java.util.logging.Level;
import java.util.logging.Logger;
import javafx.event.ActionEvent;
import javafx.fxml.FXMLLoader;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.control.Label;
import javafx.scene.control.Menu;
import javafx.scene.control.MenuBar;
import javafx.scene.control.MenuItem;
import javafx.scene.control.TitledPane;
import javafx.scene.layout.HBox;
import javafx.scene.layout.Priority;
import javafx.scene.layout.VBox;

/**
 *
 * @author tkonno
 */
public class RunControlMainPane extends VBox {

    private final RCStateLabel m_rclabel;
    private final Label m_label_config;
    private final Label m_label_runnos;
    private int m_expno;
    private int m_runno;
    private int m_subno;
    private String m_operators = "";
    private String m_comment = "";
    private final MenuBar m_bar;
    private final Menu m_config_menu;
    private final RCStateViewPane m_stateview;
    private boolean m_updated_expno = false;
    private boolean m_updated_operators = false;
    private boolean m_updated_comment = false;
    private final String m_nodename;

    public RunControlMainPane(String nodename) throws IOException {
        m_nodename = nodename.toUpperCase();
        m_bar = new MenuBar();
        Menu file_menu = new Menu("_File");
        MenuItem open_item = new MenuItem("_Open");
        MenuItem exit_item = new MenuItem("E_xit");
        exit_item.setOnAction((ActionEvent t) -> {
            System.exit(0);
        });
        file_menu.getItems().addAll(open_item, exit_item);
        Menu setting_menu = new Menu("_Setting");
        MenuItem run_item = new MenuItem("_Run");
        run_item.setOnAction((ActionEvent t) -> {
            NSMRequestHandlerUI.get().add(new NSMVSetHandler(true, m_nodename, "expno", NSMVar.INT) {
                @Override
                public boolean handleVSet(NSMVar var) {
                    m_expno = var.getInt();
                    m_updated_expno = true;
                    openDialog();
                    return true;
                }
            });
            NSMRequestHandlerUI.get().add(new NSMVSetHandler(true, m_nodename, "operators", NSMVar.TEXT) {
                @Override
                public boolean handleVSet(NSMVar var) {
                    m_operators = var.getText();
                    m_updated_operators = true;
                    openDialog();
                    return true;
                }
            });
            NSMRequestHandlerUI.get().add(new NSMVSetHandler(true, m_nodename, "comment", NSMVar.TEXT) {
                @Override
                public boolean handleVSet(NSMVar var) {
                    m_comment = var.getText();
                    m_updated_comment = true;
                    openDialog();
                    return true;
                }
            });
        });
        m_config_menu = new Menu("_Config");
        setting_menu.getItems().add(run_item);
        setting_menu.getItems().add(m_config_menu);
        m_bar.getMenus().addAll(file_menu, setting_menu);
        getChildren().add(m_bar);
        HBox hbox = new HBox();
        HBox.setHgrow(hbox, Priority.ALWAYS);
        hbox.setAlignment(Pos.CENTER_LEFT);
        Label label_node = new Label(nodename.toUpperCase());
        HBox.setMargin(label_node, new Insets(5, 5, 5, 5));
        hbox.getChildren().add(label_node);
        m_rclabel = new RCStateLabel();
        m_rclabel.setMinSize(100, 35);
        hbox.getChildren().add(m_rclabel);
        NSMRequestHandlerUI.get().add(new NSMVSetHandler(false, nodename, "rcstate", NSMVar.TEXT) {
            @Override
            public boolean handleVSet(NSMVar var) {
                m_rclabel.update(new RCState(var.getText()));
                return true;
            }
        });
        Label label = new Label("Config : ");
        HBox.setMargin(label, new Insets(5, 5, 5, 5));
        hbox.getChildren().add(label);
        m_label_config = new Label("");
        HBox.setMargin(m_label_config, new Insets(5, 5, 5, 5));
        hbox.getChildren().add(m_label_config);
        NSMRequestHandlerUI.get().add(new NSMVSetHandler(false, nodename, "rcconfig", NSMVar.TEXT) {
            @Override
            public boolean handleVSet(NSMVar var) {
                String[] s = var.getText().split("@");
                m_label_config.setText(s.length > 1 ? s[1] : s[0]);
                return true;
            }
        });
        Label label_runnos = new Label("Run # : ");
        HBox.setMargin(label_runnos, new Insets(5, 5, 5, 5));
        hbox.getChildren().add(label_runnos);
        m_label_runnos = new Label("");
        HBox.setMargin(m_label_runnos, new Insets(5, 5, 5, 5));
        hbox.getChildren().add(m_label_runnos);
        NSMRequestHandlerUI.get().add(new NSMVSetHandler(false, nodename, "expno", NSMVar.INT) {
            @Override
            public boolean handleVSet(NSMVar var) {
                m_expno = var.getInt();
                updateRunnos();
                return true;
            }
        });
        NSMRequestHandlerUI.get().add(new NSMVSetHandler(false, nodename, "runno", NSMVar.INT) {
            @Override
            public boolean handleVSet(NSMVar var) {
                m_runno = var.getInt();
                updateRunnos();
                return true;
            }
        });
        NSMRequestHandlerUI.get().add(new NSMVSetHandler(false, nodename, "subno", NSMVar.INT) {
            @Override
            public boolean handleVSet(NSMVar var) {
                m_subno = var.getInt();
                updateRunnos();
                return true;
            }
        });
        getChildren().add(hbox);

        hbox = new HBox();
        RCButtonPane button = new RCButtonPane();
        button.setNode(nodename);
        m_stateview = new RCStateViewPane(this, nodename, 5, 5);
        hbox.getChildren().add(button);
        hbox.getChildren().add(m_stateview);
        getChildren().add(hbox);
        HBox.setHgrow(hbox, Priority.ALWAYS);
        hbox.setMinSize(10, 220);
        hbox.setMaxSize(MAX_VALUE, MAX_VALUE);
        FXMLLoader loader = new FXMLLoader(getClass().getResource("LogViewPane.fxml"));
        TitledPane logview = loader.load();
        final LogViewPaneController logger = loader.getController();
        NSMRequestHandlerUI.get().add(new NSMLogHandler() {
            @Override
            public boolean handleLog(LogMessage msg) {
                logger.add(msg);
                return true;
            }

            @Override
            public boolean connected() {
                logger.add("Connected");
                return true;
            }
        });
        NSMRequestHandlerUI.get().add(new NSMVSetHandler(true, nodename, "log.dbtable", NSMVar.TEXT) {
            @Override
            public boolean handleVSet(NSMVar var) {
                try {
                    int[] pars = {50};
                    NSMCommunicator.get().request(new NSMMessage("", NSMCommand.LOGLIST, pars, var.getText() + "/"));
                } catch (IOException ex) {
                    Logger.getLogger(RunControlMainPane.class.getName()).log(Level.SEVERE, null, ex);
                }
                return true;
            }
        });
        HBox.setHgrow(logview, Priority.ALWAYS);
        logview.setMaxSize(MAX_VALUE, MAX_VALUE * 0.7);
        getChildren().add(logview);
    }

    private void updateRunnos() {
        m_label_runnos.setText(String.format("%04d.%06d.%03d", m_expno, m_runno, m_subno));
    }

    public Menu getConfigMenu() {
        return m_config_menu;
    }

    private void openDialog() {
        if (m_updated_expno && m_updated_operators && m_updated_comment) {
           RunSettingDialog.showDialog(m_nodename, "Run Setting", m_expno, m_runno, m_operators, m_comment);
           m_updated_expno = m_updated_operators = m_updated_comment = false;
        }
    }
}
