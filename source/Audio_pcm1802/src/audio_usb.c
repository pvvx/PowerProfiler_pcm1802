#include "usbd_core.h"
#include "usbd_audio.h"

#define USBD_VID           0xffff
#define USBD_PID           0xffff
#define USBD_MAX_POWER     100
#define USBD_LANGID_STRING 1033

#ifdef CONFIG_USB_HS
#define EP_INTERVAL 0x04
#else
#define EP_INTERVAL 0x01
#endif

#define AUDIO_IN_EP  0x81
#define AUDIO_OUT_EP 0x02

/* AUDIO Class Config */
#define AUDIO_FREQ 48000U

#define AUDIO_SAMPLE_FREQ(frq) (uint8_t)(frq), (uint8_t)((frq >> 8)), (uint8_t)((frq >> 16))


#define _AUDIO_AS_DESCRIPTOR_INIT(bInterfaceNumber, bTerminalLink, bNrChannels, bEndpointAddress, wMaxPacketSize, bInterval, ...) \
    0x09,                            /* bLength */                                                                       \
    USB_DESCRIPTOR_TYPE_INTERFACE,   /* bDescriptorType */                                                               \
    bInterfaceNumber,                /* bInterfaceNumber */                                                              \
    0x00,                            /* bAlternateSetting */                                                             \
    0x00,                            /* bNumEndpoints */                                                                 \
    USB_DEVICE_CLASS_AUDIO,          /* bInterfaceClass */                                                               \
    AUDIO_SUBCLASS_AUDIOSTREAMING,   /* bInterfaceSubClass */                                                            \
    AUDIO_PROTOCOL_UNDEFINED,        /* bInterfaceProtocol */                                                            \
    0x00,                            /* iInterface */                                                                    \
    0x09,                            /* bLength */                                                                       \
    USB_DESCRIPTOR_TYPE_INTERFACE,   /* bDescriptorType */                                                               \
    bInterfaceNumber,                /* bInterfaceNumber */                                                              \
    0x01,                            /* bAlternateSetting */                                                             \
    0x01,                            /* bNumEndpoints */                                                                 \
    USB_DEVICE_CLASS_AUDIO,          /* bInterfaceClass */                                                               \
    AUDIO_SUBCLASS_AUDIOSTREAMING,   /* bInterfaceSubClass */                                                            \
    AUDIO_PROTOCOL_UNDEFINED,        /* bInterfaceProtocol */                                                            \
    0x00,                            /* iInterface */                                                                    \
    0x07,                            /* bLength */                                                                       \
    AUDIO_INTERFACE_DESCRIPTOR_TYPE, /* bDescriptorType */                                                               \
    AUDIO_STREAMING_GENERAL,         /* bDescriptorSubtype */                                                            \
    bTerminalLink,                   /* bTerminalLink : Unit ID of the Output Terminal*/                                 \
    0x01,                            /* bDelay */                                                                        \
    WBVAL(AUDIO_FORMAT_PCM),         /* wFormatTag : AUDIO_FORMAT_PCM */                                                 \
    0x08 + PP_NARG(__VA_ARGS__),     /* bLength */                                                                       \
    AUDIO_INTERFACE_DESCRIPTOR_TYPE, /* bDescriptorType */                                                               \
    AUDIO_STREAMING_FORMAT_TYPE,     /* bDescriptorSubtype */                                                            \
    AUDIO_FORMAT_TYPE_I,             /* bFormatType */                                                                   \
    bNrChannels,                     /* bNrChannels */                                                                   \
    2,	/* bSubFrameSize : 2 Bytes per audio subframe */                                    \
    24,	/* bBitResolution : 24 bits per sample */                                           \
    (PP_NARG(__VA_ARGS__)/3),        /* bSamFreqType : only one frequency supported */                                   \
    __VA_ARGS__,                     /* tSamFreq : Audio sampling frequency coded on 3 bytes */                          \
    0x09,                            /* bLength */                                                                       \
    USB_DESCRIPTOR_TYPE_ENDPOINT,    /* bDescriptorType */                                                               \
    bEndpointAddress,                /* bEndpointAddress : IN endpoint 1 */                                              \
    0x01,                            /* bmAttributes */                                                                  \
    WBVAL(wMaxPacketSize),           /* wMaxPacketSize */                                                                \
    bInterval,                       /* bInterval : one packet per frame */                                              \
    0x00,                            /* bRefresh */                                                                      \
    0x00,                            /* bSynchAddress */                                                                 \
    0x07,                            /* bLength */                                                                       \
    AUDIO_ENDPOINT_DESCRIPTOR_TYPE,  /* bDescriptorType */                                                               \
    AUDIO_ENDPOINT_GENERAL,          /* bDescriptor */                                                                   \
    AUDIO_EP_CONTROL_SAMPLING_FEQ,   /* bmAttributes AUDIO_SAMPLING_FREQ_CONTROL */                                      \
    0x00,                            /* bLockDelayUnits */                                                               \
    0x00,                            /* wLockDelay */                                                                    \
    0x00

/* AudioFreq * DataSize (2 bytes) * NumChannels (Stereo: 2) */
#define AUDIO_OUT_PACKET ((uint32_t)((AUDIO_FREQ * 2 * 2) / 1000)) // 96000*3*2/1000 = 576
/* 16bit(2 Bytes) 双声道(Mono:2) */
#define AUDIO_IN_PACKET ((uint32_t)((AUDIO_FREQ * 2 * 2) / 1000)) // 96000*3*2/1000 = 576

#define USB_AUDIO_CONFIG_DESC_SIZ (unsigned long)(9 +                                       \
                                                  AUDIO_AC_DESCRIPTOR_INIT_LEN(2) +         \
                                                  AUDIO_SIZEOF_AC_INPUT_TERMINAL_DESC +     \
                                                  AUDIO_SIZEOF_AC_FEATURE_UNIT_DESC(2, 1) + \
                                                  AUDIO_SIZEOF_AC_OUTPUT_TERMINAL_DESC +    \
                                                  AUDIO_SIZEOF_AC_INPUT_TERMINAL_DESC +     \
                                                  AUDIO_SIZEOF_AC_FEATURE_UNIT_DESC(2, 1) + \
                                                  AUDIO_SIZEOF_AC_OUTPUT_TERMINAL_DESC +    \
                                                  AUDIO_AS_DESCRIPTOR_INIT_LEN(1) +         \
                                                  AUDIO_AS_DESCRIPTOR_INIT_LEN(1))

#define AUDIO_AC_SIZ (AUDIO_SIZEOF_AC_HEADER_DESC(2) +          \
                      AUDIO_SIZEOF_AC_INPUT_TERMINAL_DESC +     \
                      AUDIO_SIZEOF_AC_FEATURE_UNIT_DESC(2, 1) + \
                      AUDIO_SIZEOF_AC_OUTPUT_TERMINAL_DESC +    \
                      AUDIO_SIZEOF_AC_INPUT_TERMINAL_DESC +     \
                      AUDIO_SIZEOF_AC_FEATURE_UNIT_DESC(2, 1) + \
                      AUDIO_SIZEOF_AC_OUTPUT_TERMINAL_DESC)

const uint8_t audio_descriptor[] = {
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0xef, 0x02, 0x01, USBD_VID, USBD_PID, 0x0001, 0x01),
    USB_CONFIG_DESCRIPTOR_INIT(USB_AUDIO_CONFIG_DESC_SIZ, 0x03, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    AUDIO_AC_DESCRIPTOR_INIT(0x00, 0x03, AUDIO_AC_SIZ, 0x00, 0x01, 0x02),
    AUDIO_AC_INPUT_TERMINAL_DESCRIPTOR_INIT(0x01, AUDIO_INTERM_DESKTOP_MIC, 0x02, 0x0003),
    AUDIO_AC_FEATURE_UNIT_DESCRIPTOR_INIT(0x02, 0x01, 0x01, 0x03, 0x00),
    AUDIO_AC_OUTPUT_TERMINAL_DESCRIPTOR_INIT(0x03, AUDIO_TERMINAL_STREAMING, 0x02),
    AUDIO_AC_INPUT_TERMINAL_DESCRIPTOR_INIT(0x04, AUDIO_TERMINAL_STREAMING, 0x02, 0x0003),
    AUDIO_AC_FEATURE_UNIT_DESCRIPTOR_INIT(0x05, 0x04, 0x01, 0x03, 0x00),
    AUDIO_AC_OUTPUT_TERMINAL_DESCRIPTOR_INIT(0x06, AUDIO_OUTTERM_SPEAKER, 0x05),
    AUDIO_AS_DESCRIPTOR_INIT(0x01, 0x04, 0x02, AUDIO_OUT_EP, AUDIO_OUT_PACKET, EP_INTERVAL, AUDIO_SAMPLE_FREQ_3B(AUDIO_FREQ)),
    AUDIO_AS_DESCRIPTOR_INIT(0x02, 0x03, 0x02, AUDIO_IN_EP, AUDIO_IN_PACKET, EP_INTERVAL, AUDIO_SAMPLE_FREQ_3B(AUDIO_FREQ)),
    ///////////////////////////////////////
    /// string0 descriptor
    ///////////////////////////////////////
    USB_LANGID_INIT(USBD_LANGID_STRING),
    ///////////////////////////////////////
    /// string1 descriptor
    ///////////////////////////////////////
    0x14,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'C', 0x00,                  /* wcChar0 */
    'h', 0x00,                  /* wcChar1 */
    'e', 0x00,                  /* wcChar2 */
    'r', 0x00,                  /* wcChar3 */
    'r', 0x00,                  /* wcChar4 */
    'y', 0x00,                  /* wcChar5 */
    'U', 0x00,                  /* wcChar6 */
    'S', 0x00,                  /* wcChar7 */
    'B', 0x00,                  /* wcChar8 */
    ///////////////////////////////////////
    /// string2 descriptor
    ///////////////////////////////////////
    0x26,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'C', 0x00,                  /* wcChar0 */
    'h', 0x00,                  /* wcChar1 */
    'e', 0x00,                  /* wcChar2 */
    'r', 0x00,                  /* wcChar3 */
    'r', 0x00,                  /* wcChar4 */
    'y', 0x00,                  /* wcChar5 */
    'U', 0x00,                  /* wcChar6 */
    'S', 0x00,                  /* wcChar7 */
    'B', 0x00,                  /* wcChar8 */
    ' ', 0x00,                  /* wcChar9 */
    'U', 0x00,                  /* wcChar10 */
    'A', 0x00,                  /* wcChar11 */
    'C', 0x00,                  /* wcChar12 */
    ' ', 0x00,                  /* wcChar13 */
    'D', 0x00,                  /* wcChar14 */
    'E', 0x00,                  /* wcChar15 */
    'M', 0x00,                  /* wcChar16 */
    'O', 0x00,                  /* wcChar17 */
    ///////////////////////////////////////
    /// string3 descriptor
    ///////////////////////////////////////
    0x16,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    '2', 0x00,                  /* wcChar0 */
    '0', 0x00,                  /* wcChar1 */
    '2', 0x00,                  /* wcChar2 */
    '1', 0x00,                  /* wcChar3 */
    '0', 0x00,                  /* wcChar4 */
    '3', 0x00,                  /* wcChar5 */
    '1', 0x00,                  /* wcChar6 */
    '0', 0x00,                  /* wcChar7 */
    '0', 0x00,                  /* wcChar8 */
    '1', 0x00,                  /* wcChar9 */
#ifdef CONFIG_USB_HS
    ///////////////////////////////////////
    /// device qualifier descriptor
    ///////////////////////////////////////
    0x0a,
    USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER,
    0x00,
    0x02,
    0x00,
    0x00,
    0x00,
    0x40,
    0x01,
    0x00,
#endif
    0x00
};

volatile bool tx_flag = 0;
volatile bool rx_flag = 0;

void usbd_audio_open(uint8_t intf)
{
    if (intf == 1) {
        rx_flag = 1;
        printf("OPEN1\r\n");
    } else {
        tx_flag = 1;
        printf("OPEN2\r\n");
    }
}
void usbd_audio_close(uint8_t intf)
{
    if (intf == 1) {
        rx_flag = 1;
        printf("CLOSE1\r\n");
    } else {
        tx_flag = 0;
        printf("CLOSE2\r\n");
    }
}

#ifdef CONFIG_USB_HS
#define AUDIO_OUT_EP_MPS 512
#else
#define AUDIO_OUT_EP_MPS 64
#endif

USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t write_buffer[2048];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t out_buffer[AUDIO_OUT_PACKET];

volatile bool ep_tx_busy_flag = false;

void usbd_configure_done_callback(void)
{
    /* setup first out ep read transfer */
    usbd_ep_start_read(AUDIO_OUT_EP, out_buffer, AUDIO_OUT_PACKET);
}

void usbd_audio_out_callback(uint8_t ep, uint32_t nbytes)
{
    USB_LOG_RAW("actual out len:%d\r\n", nbytes);
    usbd_ep_start_read(AUDIO_OUT_EP, out_buffer, AUDIO_OUT_PACKET);
}

void usbd_audio_in_callback(uint8_t ep, uint32_t nbytes)
{
    USB_LOG_RAW("actual in len:%d\r\n", nbytes);
    ep_tx_busy_flag = false;
}

static struct usbd_endpoint audio_in_ep = {
    .ep_cb = usbd_audio_in_callback,
    .ep_addr = AUDIO_IN_EP
};

static struct usbd_endpoint audio_out_ep = {
    .ep_cb = usbd_audio_out_callback,
    .ep_addr = AUDIO_OUT_EP
};

struct usbd_interface intf0;
struct usbd_interface intf1;
struct usbd_interface intf2;

void audio_init()
{
    usbd_desc_register(audio_descriptor);
    usbd_add_interface(usbd_audio_init_intf(&intf0));
    usbd_add_interface(usbd_audio_init_intf(&intf1));
    usbd_add_interface(usbd_audio_init_intf(&intf2));
    usbd_add_endpoint(&audio_in_ep);
    usbd_add_endpoint(&audio_out_ep);

    usbd_audio_add_entity(0x02, AUDIO_CONTROL_FEATURE_UNIT);
    usbd_audio_add_entity(0x05, AUDIO_CONTROL_FEATURE_UNIT);

    usbd_initialize();
}

void audio_test()
{
    while (1) {
        if (tx_flag) {
           memset(write_buffer, 'a', 2048);
           ep_tx_busy_flag = true;
           usbd_ep_start_write(AUDIO_IN_EP, write_buffer, AUDIO_OUT_PACKET);
           while (ep_tx_busy_flag) {
           }
        }
    }
}
