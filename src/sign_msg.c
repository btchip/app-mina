#include <assert.h>
#include <stdlib.h>

#include "menu.h"
#include "sign_msg.h"
#include "utils.h"
#include "crypto.h"
#include "random_oracle_input.h"
#include "transaction.h"

void handle_sign_msg(uint8_t p1, uint8_t p2, uint8_t *dataBuffer,
                    uint8_t dataLength, volatile unsigned int *flags, volatile unsigned int *tx)
{
    UNUSED(p1);
    UNUSED(p2);
    UNUSED(flags);
    Keypair   kp;
    Signature sig;
    Field   input_fields[0];
    uint8_t bits[TX_BITSTRINGS_BYTES];
    ROInput   roinput = roinput_create(input_fields, bits);
    uint32_t  account;
    uint8_t network;
    uint8_t i;

    if ((dataLength < 5) || (dataLength > 5 + TX_BITSTRINGS_BYTES)) {
        THROW(INVALID_PARAMETER);
    }

    account = read_uint32_be(dataBuffer);
    network = dataBuffer[4];

    for (i=0; i < dataLength - 5; i++) {
        uint8_t digit = dataBuffer[5 + i];
        if (digit != '\r' && digit != '\n' && !(digit - 0x20 < 0x5f)) {
            THROW(INVALID_PARAMETER);
        }
    }

    generate_keypair(&kp, account);
    
    roinput_add_bytes_le(&roinput, dataBuffer + 5, dataLength - 5);

    if (!sign(&sig, &kp, &roinput, network)) {
        THROW(INVALID_PARAMETER);
    }

    memmove(G_io_apdu_buffer, &sig, sizeof(sig));

    *tx = sizeof(sig);
    THROW(0x9000);  
}

