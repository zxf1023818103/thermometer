#ifndef ESS_H
#define ESS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

int16_t bt_ess_get_temperature(void);
uint16_t bt_ess_get_humidity(void);
int bt_ess_set_humidity(uint16_t value);
int bt_ess_set_temperature(int16_t value);

#ifdef __cplusplus
}
#endif

#endif // ESS_H