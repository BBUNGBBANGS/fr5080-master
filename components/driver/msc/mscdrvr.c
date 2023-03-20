/**********************************************************************************************************
*                        ZCAN Microelectronics Technology  Corp
*                       (c) Copyright 2009 - 2013, All Rights Reserved
*
*                       File Name :         mscdrvr.c
*                       Description:        usb disk  driver
*                       Revision History:
*
*                       DATE                    VERSION             AUTHOR              NOTE
*                   ----------              ----------          --------           --------
*                   2013/05/5                    0.1                     ALI                 Creation
**********************************************************************************************************/
#include <string.h>

#include "common.h"
#include "platform.h"
#include "usb.h"
#include "usbhost.h"
#include "scsi.h"
#include "mscdrvr.h"
#include "system.h"
#include "msg.h"
#include "ff.h"
#include "gpio.h"
#include "diskio.h"
extern volatile  uint8_t g_usbh_link_stat;


/* Driver interface routines. */

#if 0
/* Data. */

CLASS_DRIVER    mscdrvr_driver =
{
    0x08,          /* uint8_t   class;       */
    0x00,          /* uint8_t   subclass;    */
    0x50,          /* uint8_t   protocol;    */
    0x0000,        /* uint16_t  vendor_ID;   */
    0x0000,        /* uint16_t  product_ID;  */
    mscdrvr_init,  /* void   (*init)( void ); */
    mscdrvr_start, /* uint16_t (*start)( USB_DEVICE *dev ); */
    mscdrvr_stop,  /* uint16_t (*stop)(void);  */
    mscdrvr_run,   /* void   (*run)(void);  */
    mscdrvr_ioctl, /* uint16_t (*ioctl)( USB_DEVICE *, uint16_t, uint16_t, uint16_t ); */
};
#endif
USB_DEVICE  g_usb_msc_device;
MSC_DRIVE msc_drive_list[MSC_DRIVES_SUPPORTED];
/*
 * The USB Frameworks used by this MSC driver only allows a single
 * USB device transaction at a time.  Therefore, it is not required
 * for the folowing buffers to be unique for each USB device and
 * can be shared.  However, it is required that these buffers be
 * allocated in a memory area usable by the USB controller.
 */
static MSC_CBW msc_cbw = {0};
static MSC_CSW msc_csw = {0};
extern FATFS Fatfs_buf[2];		/* File system object for each logical drive */


extern void audio_decoder_Stop(void);

//DISKINTF msc_disk_intf = { msc_scsi_request };
static void msc_config_bulk_out_endpoint(ENDPOINT_DESCRIPTOR *ep)
{
    uint16_t packet_size;
    packet_size = ep->wMaxPacketSize[0] | (ep->wMaxPacketSize[1] << 8);
    REGB(OTG_INDEX) = ep->bEndpointAddress;
    REGB(OTG_TXMAXP) = packet_size / 8;     // 64 bytes
    REGB(OTG_TXFIFO1) = 0;   // FIFO address 0x40
    REGB(OTG_TXFIFO2) = ((packet_size / 8) - 1) << 5;

    REGB(OTG_TXCSR1) = USBH_TXCSR1_FLUSHFIFO | USBH_TXCSR1_CLRDATATOG;
    REGB(OTG_TXTYPE) = 0x20 | ep->bEndpointAddress;
    REGB(OTG_TXINTERVAL) = 255;
}

static void msc_config_bulk_in_endpoint(ENDPOINT_DESCRIPTOR *ep)
{
    uint16_t packet_size;
    packet_size = ep->wMaxPacketSize[0] | (ep->wMaxPacketSize[1] << 8);
    REGB(OTG_INDEX) = ep->bEndpointAddress ^ 0x80;
    REGB(OTG_RXMAXP) = packet_size / 8;     // 64 bytes
    REGB(OTG_RXFIFO1) = 0x08;   // FIFO address 0x40
    REGB(OTG_RXFIFO2) = (((packet_size / 8) - 1) << 5); //|0x10;
    REGB(OTG_RXCSR1) = USBH_RXCSR1_FLUSHFIFO | USBH_RXCSR1_CLRDATATOG;
    REGB(OTG_RXTYPE) = 0x20 | ep->bEndpointAddress;
    REGB(OTG_RXINTERVAL) = 255;
}

ENDPOINT_DESCRIPTOR *msc_find_bulk_endpoint(uint8_t ep_dir)
{
    COMMON_DESCRIPTOR   *common_ptr;
    uint8_t                   ep_count;
    USB_DEVICE *dev = &g_usb_msc_device;
    ENDPOINT_DESCRIPTOR   *ep_ptr;

    // Loop through current interface looking for the bulk endpoint that matches.
    // point to first endpoint in interface
    common_ptr = (COMMON_DESCRIPTOR *)((uint8_t *)dev->inf_descr + dev->inf_descr->bLength);
    ep_count = dev->inf_descr->bNumEndpoints;

    while ((common_ptr->bDescriptorType == USB_ENDPOINT_DESCRIPTOR_TYPE) && (ep_count))
    {

        /* Check the type of this descriptor. */
        switch(common_ptr->bDescriptorType)
        {
            /* Looking for all endpoint descriptors in current interface */
        case USB_ENDPOINT_DESCRIPTOR_TYPE:
        {
            ep_ptr = (ENDPOINT_DESCRIPTOR *)common_ptr;

            if ( (ep_ptr->bmAttributes & USB_ENDPOINT_ATTRIBUTES_MASK) == USB_BULK_TRANSFER_TYPE &&
                    (ep_ptr->bEndpointAddress & USB_ENDPOINT_DIRECTION_MASK) == ep_dir )
            {
                return ep_ptr;
            }

            ep_count--;
        }
        break;

        default:
            break;
        }

        /* Increment the cur_ptr by the length of this structure.
         */

        common_ptr = (COMMON_DESCRIPTOR *)((uint8_t *)common_ptr + common_ptr->bLength);
    }

    /* No bulk endpoint match found */
    return 0;
}

MSC_DRIVE *msc_alloc_device(uint8_t lun)
{
    uint16_t i;
    USB_DEVICE *dev = &g_usb_msc_device;

    for (i = 0; i < MSC_DRIVES_SUPPORTED; i++)
    {
        if (!msc_drive_list[i].used)
        {
            /* Indicate this device is now in use */
            msc_drive_list[i].used = TRUE;

            /* Set link to USB device & LUN */
            msc_drive_list[i].dev = dev;
            msc_drive_list[i].lun = lun;

            /* Locate required Bulk endpoints */
            msc_drive_list[i].bulk_out = msc_find_bulk_endpoint(USB_ENDPOINT_OUT);
            msc_drive_list[i].bulk_in  = msc_find_bulk_endpoint(USB_ENDPOINT_IN);

            /* Make sure both bulk endpoints were located */
            if ((!msc_drive_list[i].bulk_out) || (!msc_drive_list[i].bulk_in))
            {
                //PRINTF(("msc_alloc_device: invalid bulk endpoints"));
                memset(&msc_drive_list[i], 0x00, sizeof(MSC_DRIVE));
                return 0;
            }

            msc_config_bulk_out_endpoint(msc_drive_list[i].bulk_out);
            msc_config_bulk_in_endpoint(msc_drive_list[i].bulk_in);
            return &msc_drive_list[i];
        }
    }

    //PRINTF(("msc_alloc_device: no available msc device"));
    return 0;
}

int msc_GetMaxLUN( uint8_t *maxlun )
{
    USB_DEVICEREQUEST setup_pkt;

    memset(&setup_pkt, 0, sizeof(USB_DEVICEREQUEST));
    setup_pkt.bmRequestType = 0xA1;
    setup_pkt.bRequest = MSC_BOT_GET_MAX_LUN;
    setup_pkt.wLength[0] = 1;
    *maxlun = 0;
    /*send setup pkt*/
    usbh_load_setup_pkt(&setup_pkt);

    if(usbh_wait_ep0_tx_done())
        return 1;

    if(usbh_setup_receive_data(maxlun, 1))
        return 1;

    return 0;
}

int msc_bulk_out(MSC_DRIVE *mscdev, void *buffer, uint16_t size)
{
    uint8_t epaddr = mscdev->bulk_out->bEndpointAddress;

    if(usbh_bulk_out(epaddr, buffer, size))
        return 1;

    return 0;
}

int msc_bulk_in(MSC_DRIVE *mscdev, void *buffer, uint16_t size)
{
    uint8_t epaddr = mscdev->bulk_in->bEndpointAddress;
    epaddr ^= 0x80;
    if(usbh_bulk_in(epaddr, buffer, size))
        return 1;

    return 0;
}

int msc_CBW( MSC_DRIVE *mscdev, uint8_t *cdb, uint8_t cdbLength, uint16_t dataLength, uint16_t flags )
{
    uint16_t  result;
    uint16_t  cbwSize = USB_BOT_CBW_SIZE;

    /* Set CBW signature */
    msc_cbw.Signature = MSC_CBW_SIGNATURE;

    /* Set CBW tag */
    msc_cbw.Tag = (uint32_t)mscdev; //   msc_cbw.Tag =(uint16_t)mscdev;

    /* Set CBW Lun */
    msc_cbw.Lun = mscdev->lun;

    /* Set CBW data length */
    msc_cbw.DataLength = dataLength;

    /* Set CBW flags - direction */
    msc_cbw.Flags = (flags & SCSI_FLAG_DIR_READ) ? MSC_CBW_DIRECTION_IN : MSC_CBW_DIRECTION_OUT;

    /* Set CBW CDB Length */
    msc_cbw.CDBLength = cdbLength;

    /* Set CBW CDB */
    memcpy(msc_cbw.CDB, cdb, cdbLength);

    /* Issue CBW */
    result = msc_bulk_out(mscdev, &msc_cbw, cbwSize);

    return result;
}

int msc_DATA( MSC_DRIVE *mscdev, uint8_t *data, uint16_t *dataLength, uint16_t flags )
{
    int  result;

    /* make sure we have data to transfer */
    if (!data || !dataLength || !(*dataLength))
        return 1;

    /* Issue DATA */
    if (flags & SCSI_FLAG_DIR_READ)
    {
        result = msc_bulk_in(mscdev, data, *dataLength);
#if 0
        /* If DATA stalled, clear bulk-in endpoint and abort data transfer */
        if (result)
            return usbh_clear_endpoint(mscdev->bulk_in);
#endif
    }
    else
    {
        result = msc_bulk_out(mscdev, data, *dataLength);
#if 0
        /* If DATA stalled, clear bulk-out endpoint and abort data transfer */
        if (result == USB_STATUS_STALL)
        {
            result = usb_clear_endpoint( mscdev->dev, mscdev->bulk_out );
        }
#endif
    }

    return result;
}

int msc_CSW( MSC_DRIVE *mscdev )
{
    int  result;
    uint16_t  cswSize = USB_BOT_CSW_SIZE;


    /* Issue CSW */
    result = msc_bulk_in(mscdev, &msc_csw, cswSize);

    return result;
}

uint8_t msc_Command( MSC_DRIVE *mscdev, uint8_t *cdb, uint8_t cdbLength, uint8_t *data, uint16_t *dataLength, uint16_t flags )
{
    int usb_result;

    /* Issue CBW */
    usb_result = msc_CBW(mscdev, cdb, cdbLength, *dataLength, flags);

    if (usb_result == 0)
    {
        /* Determine if there is a data phase */
        if ( (data != 0) && (*dataLength) )
        {
            /* Issue data request */
            usb_result = msc_DATA(mscdev, data, dataLength, flags);
        }

        /* If everything is okay, proceed to CSW phase */
        if (usb_result == 0)
        {
            uint8_t i;

            /* Based on possible STALL conditions, we might need to
             * retry the CBW (up to 2 times)
             */
            for (i = 0; i < 2; i++)
            {
                /* Issue CSW */
                usb_result = msc_CSW(mscdev);

                if ((usb_result != 0) && (i == 0))
                {
                    /* CSW stalled, clear CSW endpoint (bulk-in) and retry */
                    //usbh_setup_clear_endpoint( mscdev->bulk_in );
                }
                else
                {
                    /* Either success or failure, break out of loop */
                    break;
                }
            }
        }
    }

    if (usb_result != 0 || msc_csw.Status == MSC_CSW_PHASE_ERROR)
    {
        /* Peform MSC Reset Recovery on USB device */
        //msc_BOTResetRecovery(mscdev);
        //usbh_setup_clear_endpoint((mscdev->bulk_in)->bEndpointAddress);
        msc_csw.Status = MSC_CSW_PHASE_ERROR;
    }

    return msc_csw.Status;
}

uint32_t msc_SCSIRequest( MSC_DRIVE *mscdev, uint8_t *cdb, uint8_t cdbLength, uint8_t *data, uint16_t *dataLength, uint16_t flags, uint8_t retries )
{
    uint32_t result;
    uint16_t origDataLength = *dataLength;

    do
    {
        *dataLength = origDataLength;
        result = (uint32_t)msc_Command( mscdev, cdb, cdbLength, data, dataLength, flags );

        if ((result == MSC_CSW_ERROR) && (flags & SCSI_FLAG_DO_REQUEST_SENSE))
        {
            uint8_t rsResult;
            SCSI_CMD_REQUEST_SENSE  rsCmd;
            SCSI_DATA_REQUEST_SENSE rsData;
            uint16_t rsDataLength = SCSI_DATA_REQUEST_SENSE_SIZE;

            memset(&rsCmd, 0, SCSI_CMD_REQUEST_SENSE_SIZE);
            rsCmd.OpCode = SCSI_OPCODE_REQUEST_SENSE;
            rsCmd.Length = rsDataLength;

            rsResult = msc_Command(mscdev, (uint8_t *)&rsCmd, SCSI_CMD_REQUEST_SENSE_SIZE, (uint8_t *)&rsData, &rsDataLength, SCSI_FLAG_DIR_READ );
            if (rsResult == MSC_CSW_SUCCESS)
            {
                /* Add SenseKey */
                result = result | (((uint32_t)(rsData.SK & 0x0F)) << 8);

                /* Add Addtional Sense Code */
                result = result | (((uint32_t)rsData.ASC) << 16);

                /* Add Addtional Sense Code Qualifier */
                result = result | (((uint32_t)rsData.ASCQ) << 24);
            }
        }

    }
    while ((result == MSC_CSW_PHASE_ERROR) && (retries--));

    return result;
}

uint32_t msc_SCSIInquiry( MSC_DRIVE *mscdev, uint8_t *inquiryData, uint16_t *inquiryDataLength )
{
    SCSI_CMD_INQUIRY    inquiryCmd;

    memset(&inquiryCmd, 0, SCSI_CMD_INQUIRY_SIZE);
    inquiryCmd.OpCode = SCSI_OPCODE_INQUIRY;
    inquiryCmd.Length = (uint8_t) * inquiryDataLength;

    return msc_SCSIRequest(mscdev, (uint8_t *)&inquiryCmd, SCSI_CMD_INQUIRY_SIZE, inquiryData, inquiryDataLength, (SCSI_FLAG_DIR_READ | SCSI_FLAG_DO_REQUEST_SENSE), MSC_DEFAULT_RETRIES);
}

uint32_t msc_SCSIReadCap( MSC_DRIVE *mscdev, uint8_t *capData, uint16_t *capDataLength )
{
    SCSI_CMD_READ_CAPACITIES    capCmd;

    memset(&capCmd, 0, SCSI_CMD_READ_CAPACITIES_SIZE);
    capCmd.OpCode = SCSI_OPCODE_READ_CAPACITY;

    return msc_SCSIRequest(mscdev, (uint8_t *)&capCmd, SCSI_CMD_READ_CAPACITIES_SIZE, capData, capDataLength, (SCSI_FLAG_DIR_READ | SCSI_FLAG_DO_REQUEST_SENSE), MSC_DEFAULT_RETRIES);
}

uint32_t msc_SCSITestUnitReady( MSC_DRIVE *mscdev )
{
    uint16_t       turDataLength = 0;
    SCSI_CMD_TUR  turCmd;

    memset(&turCmd, 0, SCSI_CMD_TUR_SIZE);
    turCmd.OpCode = SCSI_OPCODE_TEST_UNIT_READY;

    return msc_SCSIRequest(mscdev, (uint8_t *)&turCmd, SCSI_CMD_TUR_SIZE, NULL, &turDataLength, SCSI_FLAG_DO_REQUEST_SENSE, 1);
}

uint32_t msc_SCSIRead( MSC_DRIVE *mscdev, uint32_t lba, uint8_t *data, uint16_t *dataLength )
{
    SCSI_CMD_READ10    readCmd;

    memset(&readCmd, 0, SCSI_CMD_READ10_SIZE);
    readCmd.OpCode = SCSI_OPCODE_READ10;
    readCmd.LBA = BigEndian32(lba);
    readCmd.Length = BigEndian16((*dataLength) / MSC_BLOCK_SIZE);

    return msc_SCSIRequest(mscdev, (uint8_t *)&readCmd, SCSI_CMD_READ10_SIZE, data, dataLength, (SCSI_FLAG_DIR_READ | SCSI_FLAG_DO_REQUEST_SENSE), MSC_DEFAULT_RETRIES);
}



/* Volume Mananger Functions. */

uint32_t msc_scsi_request( uint16_t context, uint8_t *cdb, uint8_t cdb_length, uint8_t *buffer, uint16_t *size, uint16_t flags )
{
    MSC_DRIVE *msc_drive = (MSC_DRIVE *)context;

    if (msc_drive != 0)
        return msc_SCSIRequest(msc_drive, cdb, cdb_length, buffer, size, flags, 3);

    return SCSI_STATUS_DEVICE_FAILURE;
}



bool msc_UpdateDriveMedia(MSC_DRIVE *msc_drive)
{
    bool result = FALSE;

    /* Get Device Capacity (to check block size) */
    SCSI_DATA_CAPACITIES capData;
    uint16_t data_buffer_length = sizeof(SCSI_DATA_CAPACITIES);
    if (SCSI_EXTRACT_STATUS(msc_SCSIReadCap( msc_drive, (uint8_t *)&capData, &data_buffer_length )) == MSC_CSW_SUCCESS)
    {
        msc_drive->lba = BigEndian32(capData.LBA);
        msc_drive->block_size = BigEndian32(capData.BlockSize);

        result = TRUE;
    }
    else
    {
        //PRINTF(("mscdrvr_start: failed ReadCapacity"));
    }

    return result;
}

bool msc_DriveReady(MSC_DRIVE *msc_drive, uint8_t tries)
{
    /* Make sure media is present and ready */
    while (tries)
    {
        if (SCSI_EXTRACT_STATUS(msc_SCSITestUnitReady(msc_drive)) == MSC_CSW_SUCCESS)
        {
            return TRUE;
        }

        tries--;
    }

    return FALSE;
}

bool msc_ValidateDrive(MSC_DRIVE *msc_drive)
{
    bool result = FALSE;

    /* Get Inquiry data */
    SCSI_DATA_INQUIRY inquiryData;
    uint16_t data_buffer_length = sizeof(SCSI_DATA_INQUIRY);
    if (SCSI_EXTRACT_STATUS(msc_SCSIInquiry( msc_drive, (uint8_t *)&inquiryData, &data_buffer_length )) == MSC_CSW_SUCCESS)
    {
        /* Make sure this is a device type we support */
        if (inquiryData.PeripheralDeviceType == SCSI_PERIPH_DIRECT_ACCESS)
        {
            /* This seems to be an okay device */
            result = TRUE;

            /* Set indication on whether this MSC drive is removable */
            msc_drive->removable = (inquiryData.DeviceTypeQualifier & SCSI_DTQ_RMB_MASK);
        }
        else
        {
            //PRINTF(("mscdrvr_start: invalid peripheral type"));
        }
    }
    else
    {
        //PRINTF(("mscdrvr_start: failed Inquiry"));
    }

    return result;
}

/* Driver functionality. */

int mscdrvr_init()
{

 CODE_PATCH_RUN(PATCH_PLACE_INDEX_2_MISC, int ( *)(int) , INDEX_2_MISC_PARAM_001);   
    if(usbh_init() == 1)
    {
        return 1;
    }
 
    //while(usbh_check_connect());
    if(usbh_connect())
        return 1;

    memset(msc_drive_list, 0x00, sizeof(msc_drive_list));

    if(mscdrvr_start())
        return 1;

    return 0;

    //

}
RES_T mscdrvr_read_blocks(uint8_t *buf, uint32_t block_index, uint16_t block_count)
{
    MSC_DRIVE *mscdrive = &msc_drive_list[0];
    block_count *= 512;
    if(msc_SCSIRead(mscdrive, block_index, buf, &block_count))
        return RES_USBH_ERR_READ_SECTOR;

    return RES_SUCCESS;
}

int mscdrvr_start()
{
    uint8_t   max_luns;
    uint8_t   lun;
    MSC_DRIVE *msc_drive =  0;
    USB_DEVICE *dev = &g_usb_msc_device;

    /* Make sure interface descriptor is valid */
    if (!dev->inf_descr)
    {
        //PRINTF(("mscdrvr_start: invalid interface descriptor"));
        return 1;
    }

    /* Attempt to set interface. Actually no need to do this */
#if 0
    if( usbh_setup_set_interface(dev->inf_descr))
    {
        //PRINTF(("mscdrvr_start: unable to set interface"));
        return 1;
    }
#endif

    /* Get Max LUNs */
    if(msc_GetMaxLUN(&max_luns))
        max_luns = 0;

    max_luns += 1;

    /* Loop through all possible LUNs */
    for (lun = 0; lun < max_luns; lun++)
    {
        /* allocate a unused MSC device structure */
        msc_drive = msc_alloc_device(lun);

        if (msc_drive)
        {
            /* Validate device */
            if (msc_ValidateDrive(msc_drive))
            {
                /* Determine if media is present */
                if (msc_DriveReady(msc_drive, MSC_IS_READY_TRIES))
                {
                    /* Update media disk info (LBA/BlockSize) */

                    if (msc_UpdateDriveMedia(msc_drive) && msc_drive->block_size == MSC_BLOCK_SIZE)
                    {
#ifdef MSC_DO_PERF_TEST
                        static uint8_t tempBuffer[MSC_BLOCK_SIZE]  = { 0 };
                        static uint16_t tempLength = sizeof(tempBuffer);

                        msc_SCSIRead( msc_drive, 0, &tempBuffer[0], &tempLength );
                        msc_SCSIRead( msc_drive, 1, &tempBuffer[0], &tempLength );
                        msc_SCSIRead( msc_drive, 0, &tempBuffer[0], &tempLength );
#endif
                        /* Let the volume manager know that a new drive is ready. */
                        //vol_mgr_add( VOL_MGR_TYPE_USB, &msc_disk_intf, (uint16_t)msc_drive );

                        /* TODO: need to figure out how to determine if above was successful or not */
                        /*          right now, just assume it is                                    */
                        msc_drive->mounted = TRUE;
                    }
                }
            }
            else
            {
                /* Unsupported device, remove from active list */
                memset(msc_drive, 0x00, sizeof(MSC_DRIVE));
            }
        }
        else
        {
            //PRINTF(("mscdrvr_start: unable to allocate new MSC device"));
            return 1;
        }
    }

    return 0;
}


int mscdrvr_stop()
{
    uint16_t i;
    USB_DEVICE *dev = &g_usb_msc_device;

    for (i = 0; i < MSC_DRIVES_SUPPORTED; i++)
    {
        if ((msc_drive_list[i].used) && (msc_drive_list[i].dev == dev))
        {
            //vol_mgr_remove( VOL_MGR_TYPE_USB, &msc_disk_intf, (uint16_t)&msc_drive_list[i] );
            memset(&msc_drive_list[i], 0x00, sizeof(MSC_DRIVE));
        }
    }

    return 0;
}



/* This is the driver's idle function. */

void mscdrvr_run()
{
    USB_DEVICE *dev = &g_usb_msc_device;
    /* When umsc_timer_count expires then
      * check MSC devices for media insertion
      * or removal
      */
    if (dev->idleCheck)
    {
        uint16_t i;

        for (i = 0; i < MSC_DRIVES_SUPPORTED; i++)
        {
            /* Determine if there are any active (removable) MSC drives for this USB device */
            if ((msc_drive_list[i].used) && (msc_drive_list[i].dev == dev) && (msc_drive_list[i].removable))
            {
                /* Determine if media is ready */
                if (msc_DriveReady(&msc_drive_list[i], 1))
                {
                    /* If media was not mounted, then mount it */
                    if (!msc_drive_list[i].mounted)
                    {
                        /* Update media disk info (LBA/BlockSize) */
                        if (msc_UpdateDriveMedia(&msc_drive_list[i]) && msc_drive_list[i].block_size == MSC_BLOCK_SIZE)
                        {
                            /* Let the volume manager know that a new drive is ready. */
                            //vol_mgr_add( VOL_MGR_TYPE_USB, &msc_disk_intf, (uint16_t)&msc_drive_list[i] );

                            /* TODO: need to figure out how to determine if above was successful or not */
                            /*          right now, just assume it is                                    */
                            msc_drive_list[i].mounted = TRUE;
                        }
                    }
                }
                /* Media is not ready */
                else
                {
                    /* If media was mounted, then unmount it */
                    if (msc_drive_list[i].mounted)
                    {
                        /* Let the volume manager know that a drive no longer ready. */
                        //vol_mgr_remove( VOL_MGR_TYPE_USB, &msc_disk_intf, (uint16_t)&msc_drive_list[i] );
                        msc_drive_list[i].mounted = FALSE;
                    }
                }
            }
        }

        dev->idleCheck = FALSE;
    }

    return;
}
#pragma push
#pragma O0
void usb_host_dev_check(void)
{
    //FIRST USB IS LOWER RES,0X581C, DM UP=1M,DP=2K DOWN
    /*
    count 0:check usb host in
    count 1:check usb host out
    count 2:check usb dev in
    count 3:check usb dev out
    count 4:check usb dev and host all in
    count 5:if Usb_Status=usb in.but actual  usb disk isn't in
    count 6:            //if Usb_Status=usb dev  in.but actual  usb disk isn't in
    count 7:check IF USB AND DEV ACTUAL IN, BUT DON'T SET OTG CTRL REGISTER
      */
    static volatile uint16 count[10] = {0};
    volatile uint32 reg = 0;
// CODE_PATCH_RUN_VOID(PATCH_PLACE_INDEX_26_USB_SD_AUX_CHECK, void ( *)(int) , INDEX_26_MISC_PARAM_001);

#if 1
//has usb host and device.do nothing
//do nothing ,在检查host和device都在时，检查不准确。

//check usb host
    reg=REGW(SC_OTGCTL)&pskeys.sc_usbhost_bitmusk;
    if(Usb_Status == USB_IN)
    {
      if((reg== (pskeys.sc_usb_hostout_status1&pskeys.sc_usbhost_bitmusk)) || (reg == (pskeys.sc_usb_hostout_status2&pskeys.sc_usbhost_bitmusk))) //if  check
    {
            if(count[1]++ >= 10)
            {
                //,MEANS USB IS OUT
                if((pskeys.mode_check.mode_cs_para.CS_mp3_usb & MODE_ALLOWED) == MODE_ALLOWED)
                {
                    Post_Ap_Message(MSG_SYS_MEDIA_CHG, USB_DET_OUT, 0, 0, 0);
                }
                Usb_Status = USB_OUT;
		REGW(SC_OTGCTL)=pskeys.sc_otgctl_reg;		//set dm up 1M,set 0x581c;			
                pskeys.mode_check.mode_cs_para.CS_mp3_usb &= MODE_STATUS_OUT; //means usb out.
                pskeys.null_usbhost_flag = 0x0;
                f_unmount(&Fatfs_buf[PHY_USBHOST]);

 
                count[1] = 0;
                ////DEBUG_PRINTF("UOUT\r\n");
                if(MediaStatus == MEDIA_USBHOST)
                {
                    audio_decoder_Stop();
                    MediaStatus = MEDIA_NONE;
                }
            }
        }
        else
        {
            count[1] = 0;
        }
    }
    else if(Usb_Status == USB_OUT )
    {
        if((reg == (pskeys.sc_usb_hostin_status1&pskeys.sc_usbhost_bitmusk)) || (reg == (pskeys.sc_usb_hostin_status2&pskeys.sc_usbhost_bitmusk)))
        {
            if(count[0] ++ >= 6)
            {
                //0x1300581c  //insert u disk ,
                if((pskeys.mode_check.mode_cs_para.CS_mp3_usb & MODE_ALLOWED) == MODE_ALLOWED)
                {
                    Post_Ap_Message(MSG_SYS_MEDIA_CHG, USB_DET_IN, 0, 0, 0);
                }
                Usb_Status = USB_IN;
                pskeys.mode_check.mode_cs_para.CS_mp3_usb |= MODE_STATUS_IN; //means usb in.
                pskeys.null_sd_flag = 0x0;
                IO_WRITE(SC_OTGCTL, pskeys.sc_otgctl_host_reg);//set dm up 1M,set 3DCF
                count[0] = 0;
                ////DEBUG_PRINTF("UIN\r\n");
            }
        }
        else
        {
            count[0] = 0;
        }
    }
    if(pskeys.null_usbhost_flag == EVENT_NULL_USBHOST)
    {
        pskeys.mode_check.mode_cs_para.CS_mp3_usb &= MODE_STATUS_OUT; //means sd out.
    }

    reg=REGW(SC_OTGCTL)&pskeys.sc_usbdev_bitmusk;
    if(UsbDev_Status == USBDEV_OUT)
    {
        //usb dev is null
        if(reg == (pskeys.sc_usb_devin_status&pskeys.sc_usbdev_bitmusk) )//insert  line to pc's usb
        {
            if(count[2]++ >= 6)
            {
                ////DEBUG_PRINTF("UdevIN\r\n");
                if( (Card_Status == SD_IN) && ((pskeys.mode_check.mode_cs_para.CS_usb_storage & MODE_ALLOWED) == MODE_ALLOWED) )	//usb storage,if sd card in, turn to usb storage, or turn to usb audio.
                {
                    Post_Ap_Message(MSG_SYS_USBDEV_DET_CHG, USBSTORAGE_DET_IN, 0, 0, 0);
                    pskeys.mode_check.mode_cs_para.CS_usb_storage |= MODE_STATUS_IN; //means usb in.
                }
                else if((pskeys.mode_check.mode_cs_para.CS_usb_audio & MODE_ALLOWED) == MODE_ALLOWED) //turn to usb audio.
                {
                    pskeys.mode_check.mode_cs_para.CS_usb_audio &= MODE_STATUS_OUT;  //meansno sd card.
                    Post_Ap_Message(MSG_SYS_USBDEV_DET_CHG, USBAUDIO_DET_IN, 0, 0, 0);
                }
                UsbDev_Status = USBDEV_IN;
                pskeys.mode_check.mode_cs_para.CS_usb_audio |= MODE_STATUS_IN; //means usb in.
                count[2] = 0;

            }
        }
        else
        {
            count[2] = 0;
        }
    }
    else if(UsbDev_Status == USBDEV_IN)
    {
        if ((Card_Status == SD_IN) && ((pskeys.mode_check.mode_cs_para.CS_usb_storage & MODE_STATUS_BIT) == 0))
        {
            pskeys.mode_check.mode_cs_para.CS_usb_storage |= MODE_STATUS_IN; //means usb staorage allowed
        }
        //usb dev is null
        if(reg ==  (pskeys.sc_usb_devout_status&pskeys.sc_usbdev_bitmusk) )//GET OUT  line FROM  pc's usb
        {
            if(count[3]++ >= 6)
            {

                if(((pskeys.mode_check.mode_cs_para.CS_usb_storage & MODE_ALLOWED) == MODE_ALLOWED) || ((pskeys.mode_check.mode_cs_para.CS_usb_audio & MODE_ALLOWED) == MODE_ALLOWED))
                    Post_Ap_Message(MSG_SYS_USBDEV_DET_CHG, USBDEV_DET_OUT, 0, 0, 0);
                UsbDev_Status = USBDEV_OUT;
                pskeys.mode_check.mode_cs_para.CS_usb_storage &= MODE_STATUS_OUT; //means usb DEV OUT.
                pskeys.mode_check.mode_cs_para.CS_usb_audio &= MODE_STATUS_OUT; //means usbDEV OUT
                count[3] = 0;
                IO_WRITE(SC_OTGCTL, pskeys.sc_otgctl_reg);//set dm up 1M,set 0x581c;
            }
        }
        else
        {
            count[3] = 0;
        }


    }

    //speciAl  process:if usb or usb dev staus is in, but reg is 0x581c
    reg = REGW(SC_OTGCTL)&pskeys.sc_usbidle_bitmusk;
    if((Usb_Status == USB_IN) && (reg== (pskeys.sc_usb_idle_status&pskeys.sc_usbidle_bitmusk)))
    {
        //if Usb_Status=usb in.but actual  usb disk isn't in
        if(count[5]++ >= 150)
        {
            Usb_Status = USB_OUT;
		REGW(SC_OTGCTL)=pskeys.sc_otgctl_reg;				
            pskeys.mode_check.mode_cs_para.CS_mp3_usb &= MODE_STATUS_OUT; //means usb out.
            pskeys.null_usbhost_flag = 0x0;
            f_unmount(&Fatfs_buf[PHY_USBHOST]);

            count[5] = 0;
            if((pskeys.mode_check.mode_cs_para.CS_mp3_usb & MODE_ALLOWED) == MODE_ALLOWED)
            {
                Post_Ap_Message(MSG_SYS_MEDIA_CHG, USB_DET_OUT, 0, 0, 0);
            }
            if(MediaStatus == MEDIA_USBHOST)
            {
                audio_decoder_Stop();
                MediaStatus = MEDIA_NONE;
            }
        }
    }
    else
    {
        count[5] = 0;
    }
	//ADD20130731
	//FOR SOME USB OUT, BUT HOST INIT ALSO WORK, THIS MAKE USB AS HOST IN STATUS,//0XFFFF
	//IF USB OUT,BUT USB REG IS HOST IN
	//MAKE SURE USB IS OUT
    if((Usb_Status == USB_OUT) && (reg== (pskeys.sc_usb_hostout_status2&pskeys.sc_usbidle_bitmusk)))
    {
        //if Usb_Status=usb in.but actual  usb disk isn't in
        if(count[8]++ >= 150)
        {
            Usb_Status = USB_OUT;
		REGW(SC_OTGCTL)=pskeys.sc_otgctl_reg;				
            pskeys.mode_check.mode_cs_para.CS_mp3_usb &= MODE_STATUS_OUT; //means usb out.
            pskeys.null_usbhost_flag = 0x0;
            f_unmount(&Fatfs_buf[PHY_USBHOST]);
            count[8] = 0;
            if(MediaStatus == MEDIA_USBHOST)
            {
                audio_decoder_Stop();
                MediaStatus = MEDIA_NONE;
            }
        }
    }
    else
    {
        count[8] = 0;
    }
	
    if((UsbDev_Status == USBDEV_IN) && (reg== (pskeys.sc_usb_idle_status&pskeys.sc_usbidle_bitmusk)))
    {
        //if Usb_Status=usb dev  in.but actual  usb disk isn't in
        if(count[6]++ >= 150)
        {
            UsbDev_Status = USBDEV_OUT;
            pskeys.mode_check.mode_cs_para.CS_usb_storage &= MODE_STATUS_OUT; //means usb DEV OUT.
            pskeys.mode_check.mode_cs_para.CS_usb_audio &= MODE_STATUS_OUT; //means usbDEV OUT
            count[6] = 0;
            IO_WRITE(SC_OTGCTL, pskeys.sc_otgctl_reg);//set dm up 1M,set 0x581c;
            if(((pskeys.mode_check.mode_cs_para.CS_usb_storage & MODE_ALLOWED) == MODE_ALLOWED) ||
                    ((pskeys.mode_check.mode_cs_para.CS_usb_audio & MODE_ALLOWED) == MODE_ALLOWED))
            {
                Post_Ap_Message(MSG_SYS_MEDIA_CHG, USB_DET_OUT, 0, 0, 0);
            }
        }
    }
    else
    {
        count[6] = 0;
    }
//ADD 20130731 
//IF USBDEV IS OUT,BUTREG IS USBDEV REG STATUS,NEED TO CLEAR
    if((UsbDev_Status == USBDEV_OUT) && (reg== (pskeys.sc_usb_devout_status&pskeys.sc_usbidle_bitmusk)))
    {
        //if Usb_Status=usb dev  in.but actual  usb disk isn't in
        if(count[9]++ >= 150)
        {
            UsbDev_Status = USBDEV_OUT;
            pskeys.mode_check.mode_cs_para.CS_usb_storage &= MODE_STATUS_OUT; //means usb DEV OUT.
            pskeys.mode_check.mode_cs_para.CS_usb_audio &= MODE_STATUS_OUT; //means usbDEV OUT
            count[9] = 0;
            IO_WRITE(SC_OTGCTL, pskeys.sc_otgctl_reg);//set dm up 1M,set 0x581c;
        }
    }
    else
    {
        count[9] = 0;
    }	
  //////////////////
     //speciAl  process:if usb status is 0000581c ,0x00003dcf,0x000039db, means turn to 0x581c.
    reg = REGW(SC_OTGCTL)&pskeys.sc_usbspecial_bitmusk;
    if(reg== 0x0)
    {
        //if Usb_Status=usb in.but actual  usb disk isn't in
        if(count[4]++ >= 100)
        {
            Usb_Status = USB_OUT;
             UsbDev_Status = USBDEV_OUT;      
	     REGW(SC_OTGCTL)=pskeys.sc_otgctl_reg;				 
            pskeys.mode_check.mode_cs_para.CS_mp3_usb &= MODE_STATUS_OUT; //means usb out.
            pskeys.mode_check.mode_cs_para.CS_usb_storage &= MODE_STATUS_OUT; //means usb DEV OUT.
            pskeys.mode_check.mode_cs_para.CS_usb_audio &= MODE_STATUS_OUT; //means usbDEV OUT           
            pskeys.null_usbhost_flag = 0x0;
            f_unmount(&Fatfs_buf[PHY_USBHOST]);
            count[4] = 0;
            if(MediaStatus == MEDIA_USBHOST)
            {
                audio_decoder_Stop();
                MediaStatus = MEDIA_NONE;
            }
        }
    }
    else
    {
        count[4] = 0;
    } 

#else
 
    if(REGW(SC_OTGCTL) == pskeys.sc_usb_hostdev_in_status)//has usb host and device.do nothing
    {
        if(count[4]++ >= 6)
        {
            count[4] = 0;
            Usb_Status = USB_OUT;
            UsbDev_Status = USBDEV_OUT;
            Post_Ap_Message(MSG_AP_SWITCH, EVENT_NULL, 0, 0, 0);//TURN TO OTHER MODE.
            pskeys.mode_check.mode_cs_para.CS_usb_storage &= MODE_STATUS_OUT; //means usb DEV OUT.
            pskeys.mode_check.mode_cs_para.CS_usb_audio &= MODE_STATUS_OUT; //means usbDEV OUT
            pskeys.mode_check.mode_cs_para.CS_mp3_usb &= MODE_STATUS_OUT; //means usb out.
            return;
        }
    }
    else
    {
        count[4] = 0;
    }
    if(Usb_Status == USB_IN)
    {
/*
		if(usbh_check_connect() == 1)
		{

                         ////DEBUG_PRINTF("UOUTtest\r\n");   

		} */
 //       if((REGW(SC_OTGCTL) == pskeys.sc_usb_hostout_status1) || (REGW(SC_OTGCTL) == pskeys.sc_usb_hostout_status2)) //if  check
        if(((REGW(SC_OTGCTL)&0X1FFFFFF) == pskeys.sc_usb_hostout_status2)) //if  check

{
            if(count[1]++ >= 10)
            {
                //IF 0X08003DCF ,MEANS USB IS OUT
                if((pskeys.mode_check.mode_cs_para.CS_mp3_usb & MODE_ALLOWED) == MODE_ALLOWED)
                {
                    Post_Ap_Message(MSG_SYS_MEDIA_CHG, USB_DET_OUT, 0, 0, 0);
                }
                Usb_Status = USB_OUT;
                pskeys.mode_check.mode_cs_para.CS_mp3_usb &= MODE_STATUS_OUT; //means usb out.
                pskeys.null_usbhost_flag = 0x0;
                f_unmount(&Fatfs_buf[PHY_USBHOST]);
                IO_WRITE(SC_OTGCTL, pskeys.sc_otgctl_reg);//set dm up 1M,set 0x581c;
                count[1] = 0;
                ////DEBUG_PRINTF("UOUT\r\n");
                if(MediaStatus == MEDIA_USBHOST)
                {
                    audio_decoder_Stop();
                    MediaStatus = MEDIA_NONE;
                }
            }
        }
        else
        {
            count[1] = 0;
        }
    }
    else if(Usb_Status == USB_OUT )
    {
        if((REGW(SC_OTGCTL) == pskeys.sc_usb_hostin_status1) || (REGW(SC_OTGCTL) == pskeys.sc_usb_hostin_status2))
        {
            if(count[0] ++ >= 6)
            {
                //0x1300581c  //insert u disk ,
                if((pskeys.mode_check.mode_cs_para.CS_mp3_usb & MODE_ALLOWED) == MODE_ALLOWED)
                {
                    Post_Ap_Message(MSG_SYS_MEDIA_CHG, USB_DET_IN, 0, 0, 0);
                }
                Usb_Status = USB_IN;
                pskeys.mode_check.mode_cs_para.CS_mp3_usb |= MODE_STATUS_IN; //means usb in.
                pskeys.null_sd_flag = 0x0;
                IO_WRITE(SC_OTGCTL, pskeys.sc_otgctl_host_reg);//set dm up 1M,set 3DCF
                count[0] = 0;
                ////DEBUG_PRINTF("UIN\r\n");
            }
        }
        else
        {
            count[0] = 0;
        }
    }
    if(pskeys.null_usbhost_flag == EVENT_NULL_USBHOST)
    {
        pskeys.mode_check.mode_cs_para.CS_mp3_usb &= MODE_STATUS_OUT; //means sd out.
    }

    if(UsbDev_Status == USBDEV_OUT)
    {
        //usb dev is null
        if(REGW(SC_OTGCTL) == pskeys.sc_usb_devin_status )//insert  line to pc's usb
        {
            if(count[2]++ >= 6)
            {
                ////DEBUG_PRINTF("UdevIN\r\n");
                if( (Card_Status == SD_IN) && ((pskeys.mode_check.mode_cs_para.CS_usb_storage & MODE_ALLOWED) == MODE_ALLOWED) )	//usb storage,if sd card in, turn to usb storage, or turn to usb audio.
                {
                    Post_Ap_Message(MSG_SYS_USBDEV_DET_CHG, USBSTORAGE_DET_IN, 0, 0, 0);
                    pskeys.mode_check.mode_cs_para.CS_usb_storage |= MODE_STATUS_IN; //means usb in.
                }
                else if((pskeys.mode_check.mode_cs_para.CS_usb_audio & MODE_ALLOWED) == MODE_ALLOWED) //turn to usb audio.
                {
                    pskeys.mode_check.mode_cs_para.CS_usb_audio &= MODE_STATUS_OUT;  //meansno sd card.
                    Post_Ap_Message(MSG_SYS_USBDEV_DET_CHG, USBAUDIO_DET_IN, 0, 0, 0);
                }
                UsbDev_Status = USBDEV_IN;
                pskeys.mode_check.mode_cs_para.CS_usb_audio |= MODE_STATUS_IN; //means usb in.
                count[2] = 0;

            }
        }
        else
        {
            count[2] = 0;
        }
    }
    else if(UsbDev_Status == USBDEV_IN)
    {
        if ((Card_Status == SD_IN) && ((pskeys.mode_check.mode_cs_para.CS_usb_storage & MODE_STATUS_BIT) == 0))
        {
            pskeys.mode_check.mode_cs_para.CS_usb_storage |= MODE_STATUS_IN; //means usb staorage allowed
        }
        //usb dev is null
        if(REGW(SC_OTGCTL) ==  pskeys.sc_usb_devout_status )//GET OUT  line FROM  pc's usb
        {
            if(count[3]++ >= 6)
            {
                ////DEBUG_PRINTF("UdevOUT\r\n");
                if(((pskeys.mode_check.mode_cs_para.CS_usb_storage & MODE_ALLOWED) == MODE_ALLOWED) || ((pskeys.mode_check.mode_cs_para.CS_usb_audio & MODE_ALLOWED) == MODE_ALLOWED))
                    Post_Ap_Message(MSG_SYS_USBDEV_DET_CHG, USBDEV_DET_OUT, 0, 0, 0);
                UsbDev_Status = USBDEV_OUT;
                pskeys.mode_check.mode_cs_para.CS_usb_storage &= MODE_STATUS_OUT; //means usb DEV OUT.
                pskeys.mode_check.mode_cs_para.CS_usb_audio &= MODE_STATUS_OUT; //means usbDEV OUT
                count[3] = 0;
                IO_WRITE(SC_OTGCTL, pskeys.sc_otgctl_reg);//set dm up 1M,set 0x581c;
            }
        }
        else
        {
            count[3] = 0;
        }


    }

    //speciAl  process
    reg = REGW(SC_OTGCTL)&0xffff;
    if((Usb_Status == USB_IN) && (reg== pskeys.sc_usb_idle_status))
    {
        //if Usb_Status=usb in.but actual  usb disk isn't in
        if(count[5]++ >= 150)
        {
            Usb_Status = USB_OUT;
            pskeys.mode_check.mode_cs_para.CS_mp3_usb &= MODE_STATUS_OUT; //means usb out.
            pskeys.null_usbhost_flag = 0x0;
            f_unmount(&Fatfs_buf[PHY_USBHOST]);
            IO_WRITE(SC_OTGCTL, pskeys.sc_otgctl_reg);//set dm up 1M,set 0x581c;
            count[5] = 0;
            if((pskeys.mode_check.mode_cs_para.CS_mp3_usb & MODE_ALLOWED) == MODE_ALLOWED)
            {
                Post_Ap_Message(MSG_SYS_MEDIA_CHG, USB_DET_OUT, 0, 0, 0);
            }
            if(MediaStatus == MEDIA_USBHOST)
            {
                audio_decoder_Stop();
                MediaStatus = MEDIA_NONE;
            }
        }
    }
    else
    {
        count[5] = 0;
    }

    if((UsbDev_Status == USBDEV_IN) && (reg== pskeys.sc_usb_idle_status))
    {
        //if Usb_Status=usb dev  in.but actual  usb disk isn't in
        if(count[6]++ >= 150)
        {
            UsbDev_Status = USBDEV_OUT;
            pskeys.mode_check.mode_cs_para.CS_usb_storage &= MODE_STATUS_OUT; //means usb DEV OUT.
            pskeys.mode_check.mode_cs_para.CS_usb_audio &= MODE_STATUS_OUT; //means usbDEV OUT
            count[6] = 0;
            IO_WRITE(SC_OTGCTL, pskeys.sc_otgctl_reg);//set dm up 1M,set 0x581c;
            if(((pskeys.mode_check.mode_cs_para.CS_usb_storage & MODE_ALLOWED) == MODE_ALLOWED) ||
                    ((pskeys.mode_check.mode_cs_para.CS_usb_audio & MODE_ALLOWED) == MODE_ALLOWED))
            {
                Post_Ap_Message(MSG_SYS_MEDIA_CHG, USB_DET_OUT, 0, 0, 0);
            }
        }
    }
    else
    {
        count[6] = 0;
    }
#endif
    return;
}
#pragma pop
uint32_t get_usb_cap(uint32_t lun)
{
    return msc_drive_list[lun].block_size;
}

#ifdef MSC_DO_PERF_TEST
uint32_t msc_SCSIWrite( MSC_DRIVE *mscdev, uint32_t lba, uint8_t *data, uint16_t *dataLength )
{
    SCSI_CMD_WRITE10    writeCmd;

    memset(&writeCmd, 0, SCSI_CMD_WRITE10_SIZE);
    writeCmd.OpCode = SCSI_OPCODE_WRITE10;
    writeCmd.LBA = BigEndian32(lba);
    writeCmd.Length = BigEndian16((*dataLength) / MSC_BLOCK_SIZE);

    return msc_SCSIRequest(mscdev, (uint8_t *)&writeCmd, SCSI_CMD_WRITE10_SIZE, data, dataLength, SCSI_FLAG_DO_REQUEST_SENSE, MSC_DEFAULT_RETRIES);
}
#endif

