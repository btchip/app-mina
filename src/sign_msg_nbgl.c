#ifdef HAVE_NBGL
#include "menu.h"
#include "sign_msg.h"
#include "utils.h"
#include "nbgl_use_case.h"
#include "transaction.h" // for TX_BITSTRINGS_BYTES

#define MAX_ELEM_CNT 3

static char _msg[TX_BITSTRINGS_BYTES + 1];
static char _account[20];
static uint8_t _network;
static uint8_t _msg_data[TX_BITSTRINGS_BYTES];
static uint8_t _msg_length;

typedef struct {
    nbgl_layoutTagValue_t tagValuePair[MAX_ELEM_CNT];
    nbgl_layoutTagValueList_t tagValueList;
} MessageContext_t;

static MessageContext_t messageContext;

static void review_choice(bool confirm) {
    if (confirm) {
        nbgl_useCaseSpinner("Processing");
        sign_message(_msg_data, _msg_length);
        nbgl_useCaseReviewStatus(STATUS_TYPE_MESSAGE_SIGNED, ui_idle);
    } else {
        sendResponse(0, false);
        nbgl_useCaseReviewStatus(STATUS_TYPE_MESSAGE_REJECTED, ui_idle);
    }
}

static void prepare_message_context(void) {
    uint8_t nbPairs = 0;

    if (_network != MAINNET_ID) {
        messageContext.tagValuePair[nbPairs].item = "Network";
        messageContext.tagValuePair[nbPairs].value = "Testnet";
        nbPairs++;
    }

    messageContext.tagValuePair[nbPairs].item = "Account";
    messageContext.tagValuePair[nbPairs].value = _account;
    nbPairs++;

    messageContext.tagValuePair[nbPairs].item = "Message";
    messageContext.tagValuePair[nbPairs].value = _msg;
    nbPairs++;

    messageContext.tagValueList.pairs = messageContext.tagValuePair;
    messageContext.tagValueList.nbPairs = nbPairs;
}

void ui_sign_msg(uint8_t *dataBuffer, uint8_t dataLength)
{
    // Store message data for signing callback
    memcpy(_msg_data, dataBuffer, dataLength);
    _msg_length = dataLength;

    // Extract account and network
    uint32_t account = read_uint32_be(dataBuffer);
    _network = dataBuffer[4];

    // Format account number
    snprintf(_account, sizeof(_account), "%d", account);

    // Copy message
    memcpy(_msg, dataBuffer + 5, dataLength - 5);
    _msg[dataLength - 5] = '\0';

    prepare_message_context();

    // Start review
    nbgl_useCaseReview(TYPE_MESSAGE,
                       &messageContext.tagValueList,
                       &C_Mina_64px,
                       "Review message",
                       NULL,
                       "Sign message",
                       review_choice);
}
#endif // HAVE_NBGL