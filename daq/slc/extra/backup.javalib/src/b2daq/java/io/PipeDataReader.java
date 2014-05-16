package b2daq.java.io;

import java.io.DataInputStream;
import java.io.IOException;
import java.io.PipedInputStream;

public class PipeDataReader extends DataReader {

	private PipedInputStream _pipe_stream;

	public PipeDataReader() throws IOException {
		_pipe_stream = new PipedInputStream();
		_stream = new DataInputStream(_pipe_stream);
	}

	public PipedInputStream getInputPipe() {
		return _pipe_stream;
	}
	
}
