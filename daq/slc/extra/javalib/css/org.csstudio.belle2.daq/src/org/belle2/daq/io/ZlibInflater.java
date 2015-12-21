package org.belle2.daq.io;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.IOException;
import java.util.zip.DataFormatException;
import java.util.zip.Inflater;

public class ZlibInflater extends DataReader {

    private byte[] _comp_buf = new byte[1024 * 1024 * 125];
    private int _comp_size;
    private final Inflater _inflater = new Inflater();
    private byte[] _buffer = new byte[1024 * 1024 * 10];
    private final ByteArrayOutputStream _byte_stream = new ByteArrayOutputStream();

    public void setBufferSize(int size) {
        if (_buffer.length < size) {
            _buffer = new byte[size];
        }
    }

    public void setInput(byte[] buf) {
        if (buf.length > _comp_buf.length) {
            _comp_buf = new byte[buf.length];
        }
        for (int i = 0; i < buf.length; i++) {
            _comp_buf[i] = buf[i];
        }
        _comp_size = buf.length;
        _inflater.setInput(_comp_buf, 0, _comp_size);
    }

    public void readBuffer(DataReader reader) throws IOException, DataFormatException {
        int buf_size = reader.readInt();
        setBufferSize(buf_size);
        int comp_size = reader.readInt();
        if (comp_size > _comp_buf.length) {
            _comp_buf = new byte[comp_size];
        }
        _comp_size = comp_size;
        for (int i = 0; i < _comp_size; i++) {
            _comp_buf[i] = reader.readByte();
        }
        _inflater.setInput(_comp_buf, 0, _comp_size);
        inflate();
    }

    public void inflate() throws DataFormatException, IOException {
        _byte_stream.reset();
        while (true) {
            int size = _inflater.inflate(_buffer, 0, _comp_size);
            _byte_stream.write(_buffer, 0, size);
            if (_inflater.getRemaining() == 0) {
                break;
            }
        }
        _byte_stream.flush();
        _inflater.reset();
        _stream = new DataInputStream(new ByteArrayInputStream(
                _byte_stream.toByteArray()));
    }

}
