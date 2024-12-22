#ifdef HAVE_BAGL
#include "menu.h"
#include "sign_msg.h"
#include "utils.h"
#include "transaction.h" // for TX_BITSTRINGS_BYTES

// Account buffer size (uint32 max = "4294967295" = 10 chars + null)
#define MAX_ACCOUNT_STR_LEN 11
static char _msg[TX_BITSTRINGS_BYTES + 1];  // +1 for null terminator
static char _account[MAX_ACCOUNT_STR_LEN];
static uint8_t _network;
static uint8_t _msg_data[TX_BITSTRINGS_BYTES];
static uint8_t _msg_length;

UX_STEP_NOCB(
    ux_sign_msg_flow_topic_step,
    pnn,
    {
        &C_icon_eye,
        "Sign",
        "Message"
    }
);

UX_STEP_NOCB(
    ux_sign_msg_flow_network_step,
    bn,
    {
        "Network",
        "testnet"
    }
);

UX_STEP_NOCB(
    ux_sign_msg_flow_account_step,
    bn,
    {
        "Account",
        _account
    }
);

UX_STEP_NOCB(
    ux_sign_msg_flow_msg_step,
    bnnn_paging,
    {
        .title = "Message",
        .text = _msg
    }
);

UX_STEP_NOCB_INIT(
    ux_sign_msg_done_flow_done_step,
    pb,
    sign_message(_msg_data, _msg_length),
    {
        &C_icon_validate_14,
        "Done"
    }
);

UX_FLOW(
    ux_sign_msg_done_flow,
    &ux_sign_msg_done_flow_done_step
);

UX_STEP_TIMEOUT(
    ux_sign_msg_comfort_flow_signing_step,
    pb,
    1,
    ux_sign_msg_done_flow,
    {
        &C_icon_processing,
        "Signing..."
    }
);

UX_FLOW(
    ux_sign_msg_comfort_flow,
    &ux_sign_msg_comfort_flow_signing_step
);

UX_STEP_VALID(
    ux_sign_msg_flow_approve_step,
    pb,
    ux_flow_init(0, ux_sign_msg_comfort_flow, NULL),
    {
        &C_icon_validate_14,
        "Approve"
    }
);

UX_STEP_VALID(
    ux_sign_msg_flow_reject_step,
    pb,
    sendResponse(0, false),
    {
        &C_icon_crossmark,
        "Reject"
    }
);

UX_FLOW(ux_sign_msg_flow_testnet,
        &ux_sign_msg_flow_topic_step,
        &ux_sign_msg_flow_network_step,
        &ux_sign_msg_flow_account_step,
        &ux_sign_msg_flow_msg_step,
        &ux_sign_msg_flow_approve_step,
        &ux_sign_msg_flow_reject_step);

UX_FLOW(ux_sign_msg_flow_mainnet,
        &ux_sign_msg_flow_topic_step,
        &ux_sign_msg_flow_account_step,
        &ux_sign_msg_flow_msg_step,
        &ux_sign_msg_flow_approve_step,
        &ux_sign_msg_flow_reject_step);

void ui_sign_msg(uint8_t *dataBuffer, uint8_t dataLength)
{
    if (dataLength < 5 || dataLength > TX_BITSTRINGS_BYTES) {
        THROW(INVALID_PARAMETER);
    }

    // Store raw data for signing
    memcpy(_msg_data, dataBuffer, dataLength);
    _msg_length = dataLength;

    // Format account for display
    uint32_t account = read_uint32_be(dataBuffer);
    snprintf(_account, sizeof(_account), "%d", account);

    // Store network ID (1 byte)
    _network = dataBuffer[4];

    // Copy message for display (+1 for null terminator)
    size_t msg_display_len = dataLength - 5;
    if (msg_display_len > TX_BITSTRINGS_BYTES) {
        THROW(INVALID_PARAMETER);
    }
    memcpy(_msg, dataBuffer + 5, msg_display_len);
    _msg[msg_display_len] = '\0';

    // Show appropriate flow
    if (_network == MAINNET_ID) {
        ux_flow_init(0, ux_sign_msg_flow_mainnet, NULL);
    } else {
        ux_flow_init(0, ux_sign_msg_flow_testnet, NULL);
    }
}
#endif // HAVE_BAGL