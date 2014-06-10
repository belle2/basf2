/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package b2daq.logger.core;

import b2daq.core.LogLevel;
import java.text.SimpleDateFormat;
import java.util.Date;
import javafx.beans.property.SimpleObjectProperty;
import javafx.beans.property.SimpleStringProperty;

/**
 *
 * @author tkonno
 */
public class LogMessage {
    
    private final SimpleStringProperty from;
    private final SimpleObjectProperty level;
    private final SimpleObjectProperty date;
    private final SimpleStringProperty message;
    static private final SimpleDateFormat dateformat = new SimpleDateFormat("HH:mm:ss dd/MM/yyyy");//"yyyy/MM/dd HH:mm:ss"
    
    static public final String getCSSPath() {
        return LogMessage.class.getResource("LogMessage.css").toExternalForm();
    }
    
    public LogMessage(String from, LogLevel level, Date date, String message) {
        this.from = new SimpleStringProperty(from);
        this.level = new SimpleObjectProperty(level);
        this.date = new SimpleObjectProperty(date);
        this.message = new SimpleStringProperty(message);
    }
    
    public LogMessage(String from, LogLevel level, String message) {
        this(from, level, new Date(), message);
    }
    
    public String getFrom() {
        return from.get();
    }

    public LogLevel getLevel() {
        return (LogLevel)level.get();
    }

    public Date getDate() {
        return (Date)date.get();
    }

    public String getDateText() {
        return dateformat.format(getDate());
    }

    public String getMessage() {
        return message.get();
    }
    
}
