// SPDX-FileCopyrightText: 2025 Ledger SAS
// SPDX-License-Identifier: Apache-2.0

/*
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#pragma once

#ifdef HAVE_HARDCODED_JETTONS

#include <stdbool.h>
#include <stddef.h>
#include <inttypes.h>

#define JETTON_NAME_MAX_SIZE 8UL

/**
 * @brief Compute Jetton wallet address.
 *
 * @param jetton_id Jetton identifier, use as index for hardcoded jettons table
 * @param owner jetton's owner account address
 * @param jetton_wallet computed jetton wallet address
 *
 * @return true is success, false otherwise.
 */
bool jetton_get_wallet_address(size_t jetton_id, const address_t *owner, address_t *jetton_wallet);

/**
 * @brief Retrieve Jetton name (ticker).
 *
 * @param jetton_id Jetton identifier, use as index for hardcoded jettons table
 *
 * @return jetton name c string, empty string if jetton_id isn't valid.
 */
const char *jetton_get_name(size_t jetton_id);

/**
 * @brief Retrieve Jetton decimals.
 *
 * @param jetton_id Jetton identifier, use as index for hardcoded jettons table
 *
 * @return jetton decimal, 0 if jetton_id isn't valid.
 */
uint8_t jetton_get_decimals(size_t jetton_id);

/**
 * @brief Compute Jetton wallet address.
 *
 * @param jetton_ticker Jetton ticker to looked up in the jettons table
 * @param owner jetton's owner account address
 * @param jetton_wallet computed jetton wallet address
 *
 * @return true is success, false otherwise.
 */
bool jetton_get_wallet_address_by_name(const char *jetton_ticker,
                                       const address_t *owner,
                                       address_t *jetton_wallet);

#endif /* HAVE_HARDCODED_JETTONS */
