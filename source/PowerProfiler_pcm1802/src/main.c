#include "main_config.h"
#include "usbh_core.h"
#include "board.h"
#include "bl702_common.h"
#include "usb_buffer.h"

extern void audio_init(void);
extern void audio_test(void);

int main(void)
{
    board_init();
    audio_init();
    while (1) {
        audio_test();
    }
}
