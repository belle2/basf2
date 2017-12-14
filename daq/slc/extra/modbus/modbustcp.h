#ifndef _modbustcp_h
#define _modbustcp_h

#if defined(__cplusplus)
extern "C" {
#endif

struct modbus_pdu {
  unsigned char func_code;
  unsigned char data [256];
} __attribute__((packed));

typedef struct modbus_pdu modbus_pdu_t;

struct modbusap_header {
  unsigned short trans_id;
  unsigned short protocol;
  unsigned short length;
  unsigned char unit_id;
} __attribute__((packed));

typedef struct modbusap_header modbusap_header_t;

struct modbus_adu {
  modbusap_header_t hdr;
  modbus_pdu_t pdu;
} __attribute__((packed));

typedef struct modbus_adu modbus_adu_t;

struct modbus_req_record {
  unsigned char ref;
  unsigned short fileno;
  unsigned short recordno;
  unsigned short nrecords;
} __attribute__((packed));

typedef struct modbus_req_record modbus_req_record_t;

struct modbus_rrecord {
  unsigned char nbyte;
  unsigned char ref;
  unsigned short data[124];
} __attribute__((packed));

typedef struct modbus_rrecord modbus_rrecord_t;

struct modbus_req_wrecord {
  unsigned char ref;
  unsigned short fileno;
  unsigned short recordno;
  unsigned short nrecords;
  unsigned short data[124];
} __attribute__((packed));

typedef struct modbus_req_wrecord modbus_req_wrecord_t;

struct modbus_regmask {
  unsigned short topregid;
  unsigned short and_mask;
  unsigned short or_mask;
} __attribute__((packed));

typedef struct modbus_regmask modbus_regmask_t;

int modbus_connect(const char* hostname, int port);

int modbus_send_recv(int fd, const modbus_adu_t* data_in,
                     modbus_adu_t* data_out, int timeout);

int modbus_coil_read(int fd, modbus_adu_t* data_in,
                     unsigned short top_coilid,
                     unsigned short nread,
                     unsigned char* ndata,
                     unsigned char* data,
                     int timeout);

int modbus_input_read(int fd, modbus_adu_t* data_in,
                      unsigned short top_coilid,
                      unsigned short nread,
                      unsigned char* ndata,
                      unsigned char* pdata,
                      int timeout);

int modbus_reg_read(int fd, modbus_adu_t* data_in,
                    unsigned short top_coilid,
                    unsigned short nread,
                    unsigned char* ndata,
                    unsigned short* pdata,
                    int timeout);

int modbus_inputreg_read(int fd, modbus_adu_t* data_in,
                         unsigned short top_coilid,
                         unsigned short nread,
                         unsigned char* ndata,
                         unsigned short* pdata,
                         int timeout);

int modbus_coil_write(int fd, modbus_adu_t* data_in,
                      unsigned short coilid,
                      unsigned short enabled,
                      int timeout);

int modbus_reg_write(int fd, modbus_adu_t* data_in,
                     unsigned short regid,
                     unsigned short wdata,
                     int timeout);

int modbus_muticoil_write(int fd, modbus_adu_t* data_in,
                          unsigned short topcoilid,
                          unsigned short nwdata,
                          unsigned char* wdata,
                          int timeout);

int modbus_mutireg_write(int fd, modbus_adu_t* data_in,
                         unsigned short topcoilid,
                         unsigned short nwdata,
                         unsigned short* wdata,
                         int timeout);

int modbus_record_read(int fd, modbus_adu_t* data_in,
                       int nreq,
                       modbus_req_record_t* req,
                       unsigned int* nrecord,
                       modbus_rrecord_t* record,
                       int timeout);

int modbus_record_write(int fd, modbus_adu_t* data_in,
                        int nreq, modbus_req_record_t* req,
                        unsigned short** wdata,
                        int timeout);

int modbus_regmask_write(int fd, modbus_adu_t* data_in,
                         modbus_regmask_t mask,
                         int timeout);

int modbus_multireg_read_write(int fd, modbus_adu_t* data_in,
                               unsigned short rregid,
                               unsigned short nrreg,
                               unsigned short* rdata,
                               unsigned short wregid,
                               unsigned short nwreg,
                               unsigned char  nwdata,
                               unsigned short* wdata,
                               int timeout);

int modbus_fifo_read(int fd, modbus_adu_t* data_in,
                     unsigned short fifoid,
                     unsigned short* ndata,
                     unsigned short* pdata,
                     int timeout);

#if defined(__cplusplus)
}
#endif

#endif
