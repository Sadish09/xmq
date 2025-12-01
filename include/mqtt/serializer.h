#pragma once
#include <stdint.h>

/*
  Total size: 20 bytes (fixed part)

  Wire layout (offsets):
  0..7    : session_id      (uint64_t)  - stable session identifier (BE)
   8      : session_flags   (uint8_t)   - bitfield
   9      : expiry_index    (uint8_t)   - maps to configured expiry durations
  10..11  : flow_window     (uint16_t)  - max inflight/unacked messages (BE)
  12..15  : last_ack_seq    (uint32_t)  - last sequence acknowledged by client (BE)
  16..19  : resume_token    (uint32_t)  - short resume token or index (BE)

  TLVs, if present, follow immediately after this fixed header.
 */

//OR these into session_flags byte
#define SESSION_FLAG_PERSISTENT      (1u << 7)  //session should persist after disconnect
#define SESSION_FLAG_CLEAN_START     (1u << 6)  //client requested clean start
#define SESSION_FLAG_REATTACH_REQ    (1u << 5)  //client requests reattach
#define SESSION_FLAG_RESUMING        (1u << 4)  //packet is part of resume handshake
// bits 3..0 reserved for future flags


//The actual durations should be defined in server config; these constants are indices used in wire format to keep headers small
enum {
    SESSION_EXPIRY_NEVER = 0,
    SESSION_EXPIRY_1H    = 1,
    SESSION_EXPIRY_6H    = 2,
    SESSION_EXPIRY_12H   = 3,
    SESSION_EXPIRY_24H   = 4,
    SESSION_EXPIRY_72H   = 5,
    SESSION_EXPIRY_7D    = 6,
    SESSION_EXPIRY_RESERVED_START = 7
};

//TLV types reserved for session metadata (follow fixed header)
enum {
    SESSION_TLV_NONE = 0,
    SESSION_TLV_PERMISSIONS_HASH = 1,
    SESSION_TLV_PENDING_SUBS_HASH = 2, // compact representation of pending subscriptions
    SESSION_TLV_STORED_QOS_STATE = 3, // binary blob of stored QoS state
    SESSION_TLV_USER_META = 4, // JSON or binary user metadata
    SESSION_TLV_VENDOR_EXT = 0xFE
};

#pragma pack(push, 1)
struct session_header_wire {
    uint64_t session_id;
    uint8_t  session_flags;
    uint8_t  expiry_index;
    uint16_t flow_window;
    uint32_t last_ack_seq;
    uint32_t resume_token;
} __attribute__((packed));
#pragma pack(pop)

_Static_assert(sizeof(struct session_header_wire) == 20,"session_header_wire must be exactly 20 bytes");

//host
struct session_header {
    uint64_t session_id;
    uint8_t  session_flags;
    uint8_t  expiry_index;
    uint16_t flow_window;
    uint32_t last_ack_seq;
    uint32_t resume_token;
};
