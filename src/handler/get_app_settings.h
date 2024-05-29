#pragma once

#include "os.h"

/**
 * Handler for GET_APP_SETTINGS command. Send APDU response with app settings.
 *
 * @return zero or positive integer if success, negative integer otherwise.
 *
 */
int handler_get_app_settings(void);
