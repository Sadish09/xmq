#pragma once
#include <stdint.h>

/* Broker Header — 12 Bytes (packed wire format)
 Layout:
0..3   broker_id      (uint32_t, BE)
4      role_flags     (uint8_t)
5      cap_ver        (uint8_t)
6 to 7   epoch          (uint16_t, BE)
8 to 11  auth_hash_id   (uint32_t, BE)
*/

#pragma pack(push, 1) //Tell the compiler NOT to pad this struct with extra bytes
struct broker_header_wire {
    uint32_t broker_id;
    uint8_t  role_flags;
    uint8_t  cap_ver;
    uint16_t epoch;
    uint32_t auth_hash_id;
};
#pragma pack(pop)

_Static_assert(sizeof(struct broker_header_wire) == 12,"broker_header_wire must be 12 bytes"); //This incase somehow we do cross size limits

// host byte order
struct broker_header {
    uint32_t broker_id;
    uint8_t  role_flags;
    uint8_t  cap_ver;
    uint16_t epoch;
    uint32_t auth_hash_id;
};
