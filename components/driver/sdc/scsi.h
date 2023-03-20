#ifndef _SCSI_H
#define _SCSI_H

#include <stdint.h>

/* Macro definitions. */

#define SCSI_FLAG_DIR_READ              0x0001
#define SCSI_FLAG_DO_REQUEST_SENSE      0x0002


#define SCSI_STATUS_SUCCESS             0x00
#define SCSI_STATUS_ERROR               0x01
#define SCSI_STATUS_DEVICE_FAILURE      0x02

#define SCSI_STATUS_MASK                0x000000FF
#define SCSI_SK_MASK                    0x0000FF00
#define SCSI_ASC_MASK                   0x00FF0000
#define SCSI_ASCQ_MASK                  0xFF000000

#define SCSI_EXTRACT_STATUS(result)     (uint8_t)(result & SCSI_STATUS_MASK)
#define SCSI_EXTRACT_SK(result)         (uint8_t)((result & SCSI_SK_MASK)>>8)
#define SCSI_EXTRACT_ASC(result)        (uint8_t)((result & SCSI_ASC_MASK)>>16)
#define SCSI_EXTRACT_ASCQ(result)       (uint8_t)((result & SCSI_ASCQ_MASK)>>24)


/* SCSI OpCode Commands */
#define SCSI_OPCODE_FORMAT_UNIT			0x04	// output
#define SCSI_OPCODE_INQUIRY                 0x12
#define SCSI_OPCODE_START_STOP              0x1b
#define SCSI_OPCODE_MODE_SELECT             0x55
#define SCSI_OPCODE_MODE_SENSE6              0x1a
#define SCSI_OPCODE_MODE_SENSE10              0x5a
#define SCSI_OPCODE_READ10                  0x28
#define SCSI_OPCODE_READ_CAPACITY           0x25
#define SCSI_OPCODE_READ_FORMAT_CAPACITY	0x23	// input
#define SCSI_OPCODE_REQUEST_SENSE           0x03
#define SCSI_OPCODE_TEST_UNIT_READY         0x00
#define SCSI_OPCODE_WRITE10                 0x2a
#define SCSI_OPCODE_WRITEANDVERIFY                 0x2e
#define SCSI_OPCODE_MODE_SENSE              0x5a

#define BigEndian16(val) ( ((val>>8)&0xFF)|((val&0xFF)<<8) )
#define BigEndian32(val) ( ((val>>24)&0xFF)|(((val>>16)&0xFF)<<8)|(((val>>8)&0xFF)<<16)|((val&0xFF)<<24) )

/* Type definitions. */

typedef __packed struct _SCSI_CMD_INQUIRY
{
    uint8_t       OpCode;
    uint8_t       LUN;
    uint8_t       Reserved1[2];
    uint8_t       Length;
    uint8_t       Reserved2[7];
} SCSI_CMD_INQUIRY;
#define SCSI_CMD_INQUIRY_SIZE   sizeof(SCSI_CMD_INQUIRY)

typedef __packed struct _SCSI_DATA_INQUIRY
{
    uint8_t       PeripheralDeviceType;
    uint8_t       DeviceTypeQualifier;
    uint8_t       IsoEcma;
    uint8_t       Reserved1;
    uint8_t       AdditionalLength;
    uint8_t       Reserved2[3];
    uint8_t       VendorInfo[8];
    uint8_t       ProductInfo[16];
    uint8_t       ProductRevision[4];
} SCSI_DATA_INQUIRY;

#define SCSI_PERIPH_DIRECT_ACCESS      0x00
#define SCSI_DTQ_RMB_MASK             0x80

typedef __packed struct _SCSI_CMD_READ_CAPACITIES
{
    uint8_t       OpCode;
    uint8_t       LUN;
    uint8_t       Reserved1[10] ;
} SCSI_CMD_READ_CAPACITIES;
#define SCSI_CMD_READ_CAPACITIES_SIZE   sizeof(SCSI_CMD_READ_CAPACITIES)

typedef __packed struct _SCSI_DATA_CAPACITIES
{
    uint32_t      LBA;
    uint32_t      BlockSize;
} SCSI_DATA_CAPACITIES;

typedef __packed struct _SCSI_CMD_TUR
{
    uint8_t       OpCode;
    uint8_t       LUN;
    uint8_t       Reserved1[10] ;
} SCSI_CMD_TUR;
#define SCSI_CMD_TUR_SIZE   sizeof(SCSI_CMD_TUR)

typedef __packed struct _SCSI_CMD_REQUEST_SENSE
{
    uint8_t       OpCode;
    uint8_t       LUN;
    uint8_t       Reserved1[2] ;
    uint8_t       Length;
    uint8_t       Reserved2[7] ;
} SCSI_CMD_REQUEST_SENSE;
#define SCSI_CMD_REQUEST_SENSE_SIZE   sizeof(SCSI_CMD_REQUEST_SENSE)

typedef __packed struct _SCSI_DATA_REQUEST_SENSE
{
    uint8_t       ErrorCode;
    uint8_t       Reserved1;
    uint8_t       SK;
    uint8_t       Info[4];
    uint8_t       AddLength;
    uint8_t       Reserved2[4];
    uint8_t       ASC;
    uint8_t       ASCQ;
    uint8_t       Reserved3;
    uint8_t       SKS[3];
} SCSI_DATA_REQUEST_SENSE;
#define SCSI_DATA_REQUEST_SENSE_SIZE   sizeof(SCSI_DATA_REQUEST_SENSE)

typedef __packed struct _SCSI_CMD_MODE_SENSE
{
    uint8_t       OpCode;
    uint8_t       LUN;
    uint8_t       PageCode;
    uint8_t       Reserved[4];
    uint16_t      Length;
    uint8_t       Control;
} SCSI_CMD_MODE_SENSE;
#define SCSI_CMD_MODE_SENSE_SIZE   sizeof(SCSI_CMD_MODE_SENSE)

typedef __packed struct _SCSI_DATA_MODE_SENSE
{
    uint16_t      Length;
    uint8_t       MediumType;
    uint8_t       WP;
    uint8_t       Reserved[4];
} SCSI_DATA_MODE_SENSE;

#define SCSI_DATA_MODE_SENSE_SIZE   sizeof(SCSI_DATA_MODE_SENSE)

typedef __packed struct _SCSI_CMD_READ10
{
    uint8_t       OpCode;
    uint8_t       LUN;
    uint32_t      LBA;
    uint8_t       Reserved1;
    uint16_t      Length;
    uint8_t       Reserved2[3] ;
} SCSI_CMD_READ10;
#define SCSI_CMD_READ10_SIZE   sizeof(SCSI_CMD_READ10)

typedef __packed struct _SCSI_CMD_WRITE10
{
    uint8_t       OpCode;
    uint8_t       LUN;
    uint32_t      LBA;
    uint8_t       Reserved1;
    uint16_t      Length;
    uint8_t       Reserved2[3] ;
} SCSI_CMD_WRITE10;
#define SCSI_CMD_WRITE10_SIZE   sizeof(SCSI_CMD_WRITE10)


#endif /* inc_scsi_h */

/* End of file: scsi.h */


