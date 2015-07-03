/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package b2daq.apps.runcontrol;

import b2daq.database.ConfigObject;
import b2daq.nsm.NSMCommunicator;
import b2daq.nsm.NSMDBListSetHandler;
import b2daq.nsm.NSMDBSetHandler;
import b2daq.nsm.NSMVSetHandler;
import b2daq.nsm.NSMVar;
import b2daq.nsm.ui.NSMRequestHandlerUI;
import b2daq.runcontrol.core.RCState;
import b2daq.runcontrol.ui.RCStateLabel;
import java.io.IOException;
import java.util.HashMap;
import java.util.logging.Level;
import java.util.logging.Logger;
import javafx.beans.property.IntegerProperty;
import javafx.beans.property.SimpleIntegerProperty;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.geometry.Insets;
import javafx.scene.Scene;
import javafx.scene.control.ContextMenu;
import javafx.scene.control.Label;
import javafx.scene.control.MenuItem;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.GridPane;
import javafx.stage.Modality;
import javafx.stage.Stage;
import javafx.stage.StageStyle;

/**
 *
 * @author tkonno
 */
public class RCStateViewPane extends GridPane {

    private final SimpleIntegerProperty m_col = new SimpleIntegerProperty(1);
    private final SimpleIntegerProperty m_row = new SimpleIntegerProperty(1);
    private int m_index = 0;
    private final HashMap<String, RCStateLabel> m_rclabel = new HashMap<>();
    private final HashMap<String, String> m_rcconfig = new HashMap<>();
    private final HashMap<String, String> m_table = new HashMap<>();
    private String m_nodename = "";
    private final RunControlMainPane m_main;

    RCStateViewPane() {
        this(null, "", 1, 1);
    }

    RCStateViewPane(RunControlMainPane main, String nodename, int col, int row) {
        m_main = main;
        m_nodename = nodename;
        m_col.set(col);
        m_row.set(row);
        setHgap(10);
        setVgap(10);
        setPadding(new Insets(0, 10, 0, 10));
        NSMRequestHandlerUI.get().add(new NSMVSetHandler(true, m_nodename, "nnodes", NSMVar.INT) {
            @Override
            public boolean handleVSet(NSMVar var) {
                int nnodes = var.getInt();
                for (int i = 0; i < nnodes; i++) {
                    NSMVSetHandler handler = new NSMVSetHandler(true, m_nodename, "node[" + i + "].name", NSMVar.TEXT) {
                        @Override
                        public boolean handleVSet(NSMVar var1) {
                            add(var1.getText());
                            addConfigMenuItem(var1.getText());
                            String node = var1.getText().toLowerCase();
                            NSMVSetHandler handler1 = new NSMVSetHandler(false, m_nodename, node + ".rcstate", NSMVar.TEXT) {
                                @Override
                                public boolean handleVSet(NSMVar var2) {
                                    String node = var2.getName().replace(".rcstate", "");
                                    setState(node, var2.getText());
                                    return true;
                                }
                            };
                            NSMRequestHandlerUI.get().add(handler1);
                            handler1 = new NSMVSetHandler(false, m_nodename, node + ".rcconfig", NSMVar.TEXT) {
                                @Override
                                public boolean handleVSet(NSMVar var2) {
                                    String node = var2.getName().replace(".rcconfig", "");
                                    String config = var2.getText();
                                    String str[] = config.split("@");
                                    if (str.length > 1) config = str[1];
                                    m_rcconfig.put(node, config);
                                    return true;
                                }
                            };
                            NSMRequestHandlerUI.get().add(handler1);
                            handler1 = new NSMVSetHandler(false, m_nodename, node + ".dbtable", NSMVar.TEXT) {
                                @Override
                                public boolean handleVSet(NSMVar var2) {
                                    String node = var2.getName().replace(".dbtable", "");
                                    m_table.put(node, var2.getText());
                                    return true;
                                }
                            };
                            NSMRequestHandlerUI.get().add(handler1);
                            handler1 = new NSMVSetHandler(false, m_nodename, node + ".used", NSMVar.INT) {
                                @Override
                                public boolean handleVSet(NSMVar var2) {
                                    String node = var2.getName().replace(".used", "");
                                    if (var2.getInt() == 0) {
                                        setState(node, RCState.OFF_S);
                                    } else {
                                        try {
                                            NSMCommunicator.get().requestVGet(m_nodename, node + ".rcstate");
                                        } catch (IOException ex) {
                                        }
                                    }
                                    return true;
                                }
                            };
                            NSMRequestHandlerUI.get().add(handler1);
                            return true;
                        }
                    };
                    NSMRequestHandlerUI.get().add(handler);
                }
                return true;
            }
        });
    }

    void add(String nodename) {
        int col = m_index / m_row.get() * 2;
        int row = m_index % m_row.get();
        Label label = new Label(nodename);
        add(label, col, row);
        RCStateLabel rclabel = new RCStateLabel();
        rclabel.setPrefSize(100, 100);
        add(rclabel, col + 1, row);
        rclabel.setOnMouseClicked(new RCNodeMenuHandler(rclabel, nodename.toLowerCase()));
        m_rclabel.put(nodename.toLowerCase(), rclabel);
        m_index++;
    }

    void setState(String nodename, String state) {
        m_rclabel.get(nodename).update(new RCState(state));
    }

    void setState(String nodename, RCState state) {
        m_rclabel.get(nodename).update(state);
    }

    public IntegerProperty colProperty() {
        return m_col;
    }

    public IntegerProperty rowProperty() {
        return m_row;
    }

    public int getCol() {
        return m_col.get();
    }

    public int getRow() {
        return m_row.get();
    }

    public void setCol(int col) {
        m_col.set(col);
    }

    public void setRow(int row) {
        m_row.set(row);
    }

    private class RCNodeMenuHandler implements EventHandler<MouseEvent> {

        private final String m_node;
        private final RCStateLabel m_label;
        private String m_config;

        public RCNodeMenuHandler(RCStateLabel label, String node) {
            m_label = label;
            m_node = node;
        }

        @Override
        public void handle(MouseEvent e) {
            ContextMenu menu = new ContextMenu();
            MenuItem vset = new MenuItem("VSET");
            MenuItem exclude = new MenuItem("EXCLUDE");
            MenuItem configure = new MenuItem("CONFIGURE");
            MenuItem load = new MenuItem("LOAD");
            MenuItem recover = new MenuItem("RECOVER");
            MenuItem abort = new MenuItem("ABORT");
            RCState state = new RCState(m_label.getText());
            if (state.equals(RCState.NOTREADY_S) || state.equals(RCState.READY_S)) {
                menu.getItems().add(vset);
                vset.setOnAction((ActionEvent event) -> {
                    VSetDialog.showDialog(null, "NSMV Set " + m_node,
                            "NSM Variable at " + m_node, m_node);
                });
            }
            if (state.equals(RCState.NOTREADY_S) || state.equals(RCState.READY_S)
                    || state.equals(RCState.UNKNOWN) || state.equals(RCState.OFF_S)) {
                menu.getItems().add(exclude);
                if (m_label.getText().equals("OFF")) {
                    exclude.setText("INCLUDE");
                }
                exclude.setOnAction((ActionEvent event) -> {
                    try {
                        int used = m_label.getText().equals("OFF") ? 1 : 0;
                        NSMCommunicator.get().requestVSet(m_nodename, new NSMVar(m_node + ".used", used));
                    } catch (IOException e1) {
                        e1.printStackTrace();
                    }
                });
            }
            if (state.equals(RCState.NOTREADY_S)) {
                menu.getItems().add(configure);
                configure.setOnAction((ActionEvent event) -> {
                    String vname = m_node.toLowerCase() + ".rcconfig";
                    NSMRequestHandlerUI.get().add(new NSMVSetHandler(true, m_nodename, vname, NSMVar.TEXT) {
                        @Override
                        public boolean handleVSet(NSMVar var) {
                            m_config = var.getText();
                            String nodename = var.getName().replace(".rcconfig", "").toUpperCase();
                            String table = m_table.get(nodename.toLowerCase());
                            NSMRequestHandlerUI.get().add(new NSMDBListSetHandler(true, table, nodename, "RC:") {
                                @Override
                                public boolean handleDBListSet(String[] list) {
                                    String message = "RC Config for " + nodename;
                                    String[] s = m_config.split("@");
                                    if (s.length > 1) m_config = s[1];
                                    String conf = RunConfigDialog.showDialog(getScene(), message, message, m_config, list);
                                    if (conf == null) return true;
                                    m_config = conf;
                                    s = conf.split("@");
                                    if (s.length > 1) m_config = s[1];
                                    try {
                                        NSMCommunicator.get().requestVSet(m_nodename, new NSMVar(vname, m_config));
                                    } catch (IOException e) {
                                        Logger.getLogger(RCButtonPane.class.getName()).log(Level.SEVERE, null, e);
                                    }
                                    return true;
                                }
                            });
                            return true;
                        }
                    });
                });
            }
            if (state.equals(RCState.NOTREADY_S) || state.equals(RCState.READY_S)) {
                menu.getItems().add(load);
                load.setOnAction((ActionEvent event) -> {
                    try {
                        NSMCommunicator.get().requestVSet(m_nodename, new NSMVar(m_node + ".rcrequest", "load"));
                    } catch (IOException e1) {
                    }
                });
            }
            if (!state.equals(RCState.UNKNOWN) && !state.equals(RCState.OFF_S)) {
                menu.getItems().add(recover);
                recover.setOnAction((ActionEvent event) -> {
                    try {
                        NSMCommunicator.get().requestVSet(m_nodename, new NSMVar(m_node + ".rcrequest", "recover"));
                    } catch (IOException e1) {
                    }
                });
            }
            if (!state.equals(RCState.UNKNOWN) && !state.equals(RCState.OFF_S)) {
                menu.getItems().add(abort);
                abort.setOnAction((ActionEvent event) -> {
                    try {
                        NSMCommunicator.get().requestVSet(m_nodename, new NSMVar(m_node + ".rcrequest", "abort"));
                    } catch (IOException e1) {
                    }
                });
            }
            menu.show(m_label, e.getScreenX(), e.getScreenY());
        }
    }

    private void addConfigMenuItem(String node) {
        MenuItem item = new MenuItem(node);
        m_main.getConfigMenu().getItems().add(item);
        item.setOnAction((ActionEvent t) -> {
            MenuItem item1 = (MenuItem) t.getSource();
            String node1 = item1.getText();
            String config = m_rcconfig.get(node1.toLowerCase());
            String table = m_table.get(node1.toLowerCase());
            if (config != null) {
                String str[] = config.split("@");
                if (str.length > 1) config = str[1];
                NSMDBSetHandler handler = new NSMDBSetHandler(table, node1, config, true) {
                    @Override
                    public boolean handleDBSet(ConfigObject obj) {
                        ConfigViewPane dialog = new ConfigViewPane(obj);
                        Scene scene1 = new Scene(dialog, 350, 550);
                        Stage stage = new Stage(StageStyle.DECORATED);
                        stage.setScene(scene1);
                        stage.initModality(Modality.WINDOW_MODAL);
                        stage.setResizable(true);
                        stage.setTitle(obj.getName());
                        stage.show();
                        return true;
                    }
                };
                NSMRequestHandlerUI.get().add(handler);
            } else {
                String vname = node1.toLowerCase() + ".rcconfig";
                NSMRequestHandlerUI.get().add(new NSMVSetHandler(true, m_nodename, vname, NSMVar.TEXT) {
                    @Override
                    public boolean handleVSet(NSMVar var) {
                        String config = var.getText();
                        String str[] = config.split("@");
                        if (str.length > 1) config = str[1];
                        String nodename = var.getName().replace(".rcconfig", "").toUpperCase();
                        String table = m_table.get(nodename.toLowerCase());
                        NSMDBSetHandler handler = new NSMDBSetHandler(table, nodename, config, true) {
                            @Override
                            public boolean handleDBSet(ConfigObject obj) {
                                ConfigViewPane dialog = new ConfigViewPane(obj);
                                Scene scene1 = new Scene(dialog, 350, 550);
                                Stage stage = new Stage(StageStyle.DECORATED);
                                stage.setScene(scene1);
                                stage.initModality(Modality.WINDOW_MODAL);
                                stage.setResizable(true);
                                stage.setTitle(obj.getName());
                                stage.show();
                                return true;
                            }
                        };
                        NSMRequestHandlerUI.get().add(handler);
                        return true;
                    }
                });
            }
        });
    }
}
