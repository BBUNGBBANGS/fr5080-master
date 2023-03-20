#ifndef _MSCDRVR_H
#define _MSCDRVR_H

#include <stdint.h>
#include <stdbool.h>

#include "type_lib.h"
#include "usb.h"

//#include "integer.h"
//#include "diskio.h"
/* Macro definitions. */

#define MSC_LUNS_PER_DEVICE             2
#define MSC_NUM_SUPPORTED_DEVICES       1
#define MSC_DRIVES_SUPPORTED            MSC_NUM_SUPPORTED_DEVICES * MSC_LUNS_PER_DEVICE
#define MSC_BLOCK_SIZE                  512

#define MSC_IS_READY_TRIES              250

#define MSC_CBW_SIGNATURE               0x43425355
#define MSC_CBW_DIRECTION_OUT           0
#define MSC_CBW_DIRECTION_IN            0x80
#define MSC_CSW_SIGNATURE               0x53425355

#define MSC_CSW_SUCCESS                 0x00
#define MSC_CSW_ERROR                   0x01
#define MSC_CSW_PHASE_ERROR             0x02

#define USB_MSC_SUBCLASS_8070I          0x05
#define USB_MSC_SUBCLASS_SCSI           0x06

#define MSC_BOT_GET_MAX_LUN             0xfe
#define MSC_BOT_RESET                   0xff

#define MSC_DEFAULT_RETRIES             3
#define MSC_DEFAULT_TIMEOUT             30000 /* 30 seconds */

#define MSC_MEDIA_CHECK_TIMEOUT         3000 /* 3 seconds */


/* Data Interface Class Protocol Codes */
#define MASS_BULK_ONLY_PROTOCOL                 0x50    // Bulk-Only Transport



//#define msc_bulk_out(mscdev, buff, size)   usb_bulk_out(mscdev->dev, mscdev->bulk_out, buff, size, MSC_DEFAULT_TIMEOUT)
//#define msc_bulk_in(mscdev, buff, size)    usb_bulk_in(mscdev->dev, mscdev->bulk_in, buff, size, MSC_DEFAULT_TIMEOUT)

#define msc_memcpy(dest, source, length)   if (length%2)                            \
                                                memcpy2((uint16 *)dest, (uint16 *)source, length/2);    \
                                            else                                    \
                                                memcpy(dest,source,length)

/* Type definitions. */

typedef struct _MSC_DRIVE
{
    bool                    used;
    bool                    mounted;
    bool                    removable;
    USB_DEVICE              *dev;
    uint32_t                  lba;
    uint32_t                  block_size;
    ENDPOINT_DESCRIPTOR *bulk_out;
    ENDPOINT_DESCRIPTOR *bulk_in;
    uint16_t                  lun;
} MSC_DRIVE;

#define USB_BOT_CDB_SIZE                        0x10
typedef __packed struct _MSC_CBW
{
    uint32_t      Signature;
    uint32_t      Tag;
    uint32_t      DataLength;
    uint8_t       Flags;
    uint8_t       Lun;
    uint8_t       CDBLength;
    uint8_t       CDB[USB_BOT_CDB_SIZE];
} MSC_CBW;
#define USB_BOT_CBW_SIZE   sizeof(MSC_CBW)

typedef __packed struct _MSC_CSW
{
    uint32_t      Signature;
    uint32_t      Tag;
    uint32_t      DataResidue;
    uint8_t       Status;
} MSC_CSW;

#define USB_BOT_CSW_SIZE   sizeof(MSC_CSW)

enum Mass_Cmd_State
{
    MASS_IDLE,
    MASS_RECEIVED_CBW,
    MASS_RECEIVE_DATA,
    MASS_RECEIVED_DATA,
    MASS_SEND_DATA,
    MASS_SENDED_DATA,
    MASS_SEND_CSW,
    MASS_SENDED_CSW,
};
////function type



//DISKINTF msc_disk_intf = { msc_scsi_request };
//static void msc_config_bulk_out_endpoint(ENDPOINT_DESCRIPTOR *ep);
//static void msc_config_bulk_in_endpoint(ENDPOINT_DESCRIPTOR *ep);
ENDPOINT_DESCRIPTOR *msc_find_bulk_endpoint(uint8_t ep_dir);
MSC_DRIVE *msc_alloc_device(uint8_t lun);
int msc_GetMaxLUN( uint8_t *maxlun );
int msc_bulk_out(MSC_DRIVE *mscdev, void *buffer, uint16_t size);
int msc_bulk_in(MSC_DRIVE *mscdev, void *buffer, uint16_t size);
int msc_CBW( MSC_DRIVE *mscdev, uint8_t *cdb, uint8_t cdbLength, uint16_t dataLength, uint16_t flags );
int msc_DATA( MSC_DRIVE *mscdev, uint8_t *data, uint16_t *dataLength, uint16_t flags );
int msc_CSW( MSC_DRIVE *mscdev );
uint8_t msc_Command( MSC_DRIVE *mscdev, uint8_t *cdb, uint8_t cdbLength, uint8_t *data, uint16_t *dataLength, uint16_t flags );
uint32_t msc_SCSIRequest( MSC_DRIVE *mscdev, uint8_t *cdb, uint8_t cdbLength, uint8_t *data, uint16_t *dataLength, uint16_t flags, uint8_t retries );
uint32_t msc_SCSIInquiry( MSC_DRIVE *mscdev, uint8_t *inquiryData, uint16_t *inquiryDataLength );
uint32_t msc_SCSIReadCap( MSC_DRIVE *mscdev, uint8_t *capData, uint16_t *capDataLength );
uint32_t msc_SCSITestUnitReady( MSC_DRIVE *mscdev );
uint32_t msc_SCSIRead( MSC_DRIVE *mscdev, uint32_t lba, uint8_t *data, uint16_t *dataLength );
/* Volume Mananger Functions. */
uint32_t msc_scsi_request( uint16_t context, uint8_t *cdb, uint8_t cdb_length, uint8_t *buffer, uint16_t *size, uint16_t flags );
bool msc_UpdateDriveMedia(MSC_DRIVE *msc_drive);
bool msc_DriveReady(MSC_DRIVE *msc_drive, uint8_t tries);
bool msc_ValidateDrive(MSC_DRIVE *msc_drive);
/* Driver functionality. */
int mscdrvr_init(void);
int mscdrvr_init_debug(void);
RES_T mscdrvr_read_blocks(uint8_t *buf, uint32_t block_index, uint16_t block_count);
int mscdrvr_start(void);
int mscdrvr_stop(void);
/* This is the driver's idle function. */
void mscdrvr_run(void);
void usb_host_dev_check(void);
uint32_t get_usb_cap(uint32_t lun);

#ifdef MSC_DO_PERF_TEST
uint32_t msc_SCSIWrite( MSC_DRIVE *mscdev, uint32_t lba, uint8_t *data, uint16_t *dataLength );
#endif
#endif /* _MSCDRVR_H */

