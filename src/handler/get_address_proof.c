/*****************************************************************************
 *   Ledger App Boilerplate.
 *   (c) 2020 Ledger SAS.
 *
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
 *****************************************************************************/

#include <stdint.h>  // uint*_t
#include <string.h>  // memset, explicit_bzero

#include "get_address_proof.h"

#include "../proof/proof_deserialize.h"
#include "../globals.h"
#include "common/mybuffer.h"
#include "../ui/display.h"

int handler_get_address_proof(uint8_t flags, buffer_t *cdata) {
    explicit_bzero(&G_context, sizeof(G_context));

    if (!deserialize_proof(cdata, flags)) {
        return 0;
    }

    G_context.req_type = GET_PROOF;
    G_context.state = STATE_NONE;

    return ui_display_proof(flags);
}
