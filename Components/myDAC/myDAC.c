#include <stdio.h>
#include "driver/dac.h"
#include "myDAC.h"

dac_oneshot_handle_t myDAC_handle;

void myDAC_setup(void) 
{
    dac_oneshot_config_t config = 
    {
        .chan_id = DAC_CHANNEL_1,
    };
    dac_oneshot_new_channel(&config, &myDAC_handle);
}

void myDAC_WriteVoltage(int value)
{
    dac_oneshot_write(myDAC_handle, value);
}