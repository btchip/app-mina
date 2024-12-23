#ifdef HAVE_NBGL
#include "menu.h"
#include "sign_msg.h"
#include "utils.h"
#include "nbgl_use_case.h"
#include "transaction.h" // for TX_BITSTRINGS_BYTES

#define MAX_ELEM_CNT 3
#define MAX_ACCOUNT_STR_LEN 11

static char _msg[TX_BITSTRINGS_BYTES + 1];  // +1 for null terminator
static char _account[MAX_ACCOUNT_STR_LEN];
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
        nbgl_useCaseSpinner("Signing message");
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
    if (dataLength < 5 || dataLength > TX_BITSTRINGS_BYTES) {
        THROW(INVALID_PARAMETER);
    }

    // Store raw data for signing
    memcpy(_msg_data, dataBuffer, dataLength);
    _msg_length = dataLength;

    // Format account number for display
    uint32_t account = read_uint32_be(dataBuffer);
    snprintf(_account, sizeof(_account), "%d", account);

    // Store network ID
    _network = dataBuffer[4];

    // Copy message for display
    size_t msg_display_len = dataLength - 5;
    memcpy(_msg, dataBuffer + 5, msg_display_len);
    _msg[msg_display_len] = '\0';

    prepare_message_context();

    nbgl_useCaseReview(TYPE_MESSAGE,
                       &messageContext.tagValueList,
                       &C_Mina_64px,
                       "Review message",
                       NULL,
                       "Sign message",
                       review_choice);
}
#endif // HAVE_NBGL