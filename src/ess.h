#ifndef ESS_H
#define ESS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

int bt_ess_set_temperature_and_humidity(uint16_t temperature_value, int16_t humidity_value);

#ifdef __cplusplus
}
#endif

#endif // ESS_H