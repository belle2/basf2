package org.belle2.daq.io;

import java.io.IOException;
import java.net.Socket;

public class SocketDataReader extends DataReader {

    public SocketDataReader() {
        super();
    }

    public SocketDataReader(Socket socket) throws IOException {
        super(socket.getInputStream());
    }
}
