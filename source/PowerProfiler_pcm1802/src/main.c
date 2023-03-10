#include "main_config.h"
#include "usbh_core.h"
#include "board.h"
#include "bl702_common.h"
#if USE_ADC
#include "bflb_adc.h"
#include "adc_dma.h"
#endif
#include "usb_buffer.h"

extern void cdc_acm_init(void);
extern void usb_dc_send_from_ringbuffer(Ring_Buffer_Type *rb);
//extern void i2s_dma_init(void);

int main(void)
{
    board_init();
#if USE_ADC
    adc_dma_init(20000); // If used USB2.0FS -> max 166666 sps!
#endif
    usb_buffers_init();
    cdc_acm_init(); // BL702 USB2.0FS max output 1000 kbytes/s
//    i2s_dma_init();
    while (1) {
    	task_data_out();
    	task_data_out();
    	task_data_out();
    	task_data_out();
    	usb_dc_send_from_ringbuffer(&usb_tx_rb);
    }
}
