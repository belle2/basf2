/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package b2daq.logger.ui;

import b2daq.logger.core.LogMessage;
import b2daq.core.LogLevel;
import java.net.URL;
import java.text.SimpleDateFormat;
import java.util.Comparator;
import java.util.Date;
import java.util.ResourceBundle;
import javafx.collections.FXCollections;
import javafx.collections.ListChangeListener;
import javafx.collections.ObservableList;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.TableCell;
import javafx.scene.control.TableColumn;
import javafx.scene.control.TableRow;
import javafx.scene.control.TableView;
import javafx.scene.control.cell.PropertyValueFactory;
import javafx.util.Callback;

/**
 * FXML Controller class
 *
 * @author tkonno
 */
public class LogViewPaneController implements Initializable {

    @FXML
    private TableView table;
    @FXML
    private TableColumn col_from;
    @FXML
    private TableColumn col_level;
    @FXML
    private TableColumn col_date;
    @FXML
    private TableColumn col_message;
    
    private final ObservableList<LogMessage> data = FXCollections.observableArrayList();

    public void add(LogMessage log) {
        data.add(log);
    }
    
    public void add(String log) {
        data.add(new LogMessage("LOCAL", LogLevel.DEBUG, log));
    }
    /**
     * Initializes the controller class.
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        table.setItems(data);
        col_from.setCellValueFactory(
                new PropertyValueFactory<>("from"));
        col_level.setCellValueFactory(
                new PropertyValueFactory<>("level"));
        col_level.setComparator(new Comparator<LogLevel>() {
            @Override
            public int compare(LogLevel t, LogLevel t1) {
                return t.hiegher(t1)?1:0;
            }
        });
        col_level.setCellFactory(new Callback<TableColumn<LogMessage, LogLevel>, TableCell<LogMessage, LogLevel>>() {
            @Override
            public TableCell<LogMessage, LogLevel> call(TableColumn<LogMessage, LogLevel> param) {
                return new TableCell<LogMessage, LogLevel>() {
                    private final SimpleDateFormat dateformat = new SimpleDateFormat("HH:mm:ss dd/MM/yyyy");
                    @Override
                    protected void updateItem(LogLevel item, boolean empty) {
                        super.updateItem(item, empty);
                        if (!empty) {
                            setText(item.getLabel());
                        } else {
                            setText(null);
                        }
                    }
                };
            }
        });
        col_date.setCellValueFactory(
                new PropertyValueFactory<>("date"));
        col_date.setComparator(new Comparator<Date>() {
            @Override
            public int compare(Date t, Date t1) {
                return t1.compareTo(t);
            }
        });
        col_date.setCellFactory(new Callback<TableColumn<LogMessage, Date>, TableCell<LogMessage, Date>>() {
            @Override
            public TableCell<LogMessage, Date> call(TableColumn<LogMessage, Date> param) {
                return new TableCell<LogMessage, Date>() {
                    private final SimpleDateFormat dateformat = new SimpleDateFormat("HH:mm:ss dd/MM/yyyy");
                    @Override
                    protected void updateItem(Date item, boolean empty) {
                        super.updateItem(item, empty);
                        if (!empty) {
                            setText(dateformat.format(item));
                        } else {
                            setText(null);
                        }
                    }
                };
            }
        });
        col_message.setCellValueFactory(
                new PropertyValueFactory<>("message"));
        table.getItems().addListener((ListChangeListener<LogMessage>) (c -> {
            c.next();
            final int size = table.getItems().size();
            if (size > 0) {
                table.scrollTo(size - 1);
            }
        }));
        table.setRowFactory(new Callback<TableView<LogMessage>, TableRow<LogMessage>>() {
            @Override
            public TableRow<LogMessage> call(TableView<LogMessage> tableView) {
                final TableRow<LogMessage> row = new TableRow<LogMessage>() {
                    @Override
                    protected void updateItem(LogMessage log, boolean empty) {
                        super.updateItem(log, empty);
                        if (!isEmpty()) {
                            getStyleClass().removeAll("log-debug", "log-info",
                                    "log-notice", "log-warning",
                                    "log-error", "log-fatal");
                            if (log.getLevel().equals(LogLevel.DEBUG)) {
                                getStyleClass().add("log-debug");
                            } else if (log.getLevel().equals(LogLevel.INFO)) {
                                getStyleClass().add("log-info");
                            } else if (log.getLevel().equals(LogLevel.NOTICE)) {
                                getStyleClass().add("log-notice");
                            } else if (log.getLevel().equals(LogLevel.WARNING)) {
                                getStyleClass().add("log-warning");
                            } else if (log.getLevel().equals(LogLevel.ERROR)) {
                                getStyleClass().add("log-error");
                            } else if (log.getLevel().equals(LogLevel.FATAL)) {
                                getStyleClass().add("log-fatal");
                            }
                        }
                    }
                };
                return row;
            }
        });
    }    
    
}
