#pragma once

#include <stdbool.h>
#include <stdint.h>

void rpm_init(void);
void rpm_set_filter_reference(float setpoint);
float rpm_get(void);
bool rpm_signal_recent(uint32_t timeout_ms);
