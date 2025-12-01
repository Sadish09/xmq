#pragma once
#include <stdint.h>

/*
Client Header — Wire Format
Total size: 16 bytes (packed)
  Wire layout (offsets):
  0 to 3   : client_id_hash   (uint32_t)  - short id or hash (BE)
  4        : client_flags     (uint8_t)   - bitfield
  5        : proto_caps       (uint8_t)    - feature bits
  6 to 7   : keepalive        (uint16_t)  - seconds (BE)
  8 to 11  : seq_or_ts        (uint32_t)  - sequence number or low-32 timestamp (BE)
  12 to 15 : auth_token_id    (uint32_t)  - token/table index (BE)

NOTE: If you need longer client IDs, include a TLV entry after this fixed header with type = CLIENT_TLV_LONG_ID containing the full GUID/string
*/

//flag bit positions
//usage: set bits into client_flags byte
#define CLIENT_FLAG_LIVENESS    (1u << 7)  // keepalive/only probe packet
#define CLIENT_FLAG_WANTS_ACK   (1u << 6)  // request acknowledgement
#define CLIENT_FLAG_ENCRYPTED   (1u << 5)  // payload encrypted
#define CLIENT_FLAG_COMPRESSED  (1u << 4)  // payload compressed
#define CLIENT_FLAG_QOS_HIGH    (1u << 3)  // higher QoS requested
#define CLIENT_FLAG_RESERVED    (1u << 0)  // lower bits reserved

//proto cap bits
//upper nibble: major features, lower nibble: minor/version if desired
#define CLIENT_CAP_BATCHING     (1u << 4)
#define CLIENT_CAP_WILL         (1u << 5)
#define CLIENT_CAP_RETAIN       (1u << 6)
#define CLIENT_CAP_RESERVED     (1u << 7)

//TLV types reserved for client metadata (refer to https://devopedia.org/tlv-format for explaination)
enum {
    CLIENT_TLV_NONE = 0,
    CLIENT_TLV_USERNAME = 1,
    CLIENT_TLV_DEVICE_MODEL = 2,
    CLIENT_TLV_APP_VERSION = 3,
    CLIENT_TLV_LONG_CLIENT_ID = 4,
    CLIENT_TLV_SUPPORTED_CIPHERS = 5,
    CLIENT_TLV_VENDOR_EXT = 0xFE
};

#pragma pack(push, 1)
struct client_header_wire {
    uint32_t client_id_hash;   // wire: BE
    uint8_t  client_flags;
    uint8_t  proto_caps;
    uint16_t keepalive;        // wire: BE
    uint32_t seq_or_ts;        // wire: BE
    uint32_t auth_token_id;    // wire: BE
} __attribute__((packed));
#pragma pack(pop)

_Static_assert(sizeof(struct client_header_wire) == 16,
               "client_header_wire must be 16 bytes");

// Host-side (convenience) representation (host byte order)
struct client_header {
    uint32_t client_id_hash;
    uint8_t  client_flags;
    uint8_t  proto_caps;
    uint16_t keepalive;
    uint32_t seq_or_ts;
    uint32_t auth_token_id;
};
