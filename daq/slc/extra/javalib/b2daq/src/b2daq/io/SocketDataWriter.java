package b2daq.io;

import java.io.IOException;
import java.net.Socket;

public class SocketDataWriter extends DataWriter {

	public SocketDataWriter() {
		super();
	}
	
	public SocketDataWriter(Socket socket) throws IOException {
		super(socket.getOutputStream());
	}
}
