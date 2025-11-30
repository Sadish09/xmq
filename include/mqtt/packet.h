#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <map>
#include <optional>
#include <memory>

namespace xmq::mqtt {

// MQTT Control Packet
enum class PacketType : uint8_t {
    RESERVED = 0,
    CONNECT = 1,
    CONNACK = 2,
    PUBLISH = 3,
    PUBACK = 4,
    PUBREC = 5,
    PUBREL = 6,
    PUBCOMP = 7,
    SUBSCRIBE = 8,
    SUBACK = 9,
    UNSUBSCRIBE = 10,
    UNSUBACK = 11,
    PINGREQ = 12,
    PINGRESP = 13,
    DISCONNECT = 14,
    AUTH = 15
};

// Reason / Error codes (subset; this section will be expanded in future for Global error codes)
enum class ReasonCode : uint8_t {
    SUCCESS = 0x00,
    NORMAL_DISCONNECTION = 0x00,
    GRANTED_QOS_0 = 0x00,
    UNSPECIFIED_ERROR = 0x80,
    MALFORMED_PACKET = 0x81,
    PROTOCOL_ERROR = 0x82,
    IMPLEMENTATION_SPECIFIC_ERROR = 0x83,
};

// MQTT v5 defines the following data types, we represent them as typed values
enum class PropertyType {
    BYTE,
    TWO_BYTE_INT,
    FOUR_BYTE_INT,
    VARINT,
    UTF8_STRING,
    BINARY_DATA
};

struct Property {
    uint32_t id;                    // property identifier (per MQTT v5)
    PropertyType type;
    // value: only one of the below will be used depending on 'type'
    std::optional<uint8_t> byte_val;
    std::optional<uint16_t> two_byte_int;
    std::optional<uint32_t> four_byte_int;
    std::optional<uint32_t> varint;
    std::optional<std::string> utf8_str;
    std::optional<std::vector<uint8_t>> binary;
};

/*
 * User Properties convenience: MQTT v5 allows multiple name/value pairs
 * where both are UTF-8 strings. We'll expose a map but keep insertion order
 * by storing vector<pair>.
 */
using UserPropertyList = std::vector<std::pair<std::string,std::string>>;

/*
 * Common header elements
 */
struct FixedHeader {
    PacketType type;
    uint8_t flags;          // lower 4 bits are packet-specific (e.g. DUP/QoS/Retain for PUBLISH)
    uint32_t remaining_len; // decoded Remaining Length (variable int in wire format)
};

/*
 * Generic MqttPacket: a lightweight container for any control packet.
 * For richer handling we provide strong types below for commonly used packets.
 */
struct MqttPacket {
    FixedHeader header;
    std::vector<uint8_t> raw_variable_header_and_payload; // raw bytes; when parsed, parsed fields present below
    // Optional parsed variants:
    // One of the following optional structs will be set depending on header.type
    struct ConnectPacket;
    struct ConnAckPacket;
    struct PublishPacket;
    struct AuthPacket;
    // ... other packets omitted for brevity

    std::unique_ptr<ConnectPacket> connect;
    std::unique_ptr<ConnAckPacket> connack;
    std::unique_ptr<PublishPacket> publish;
    std::unique_ptr<AuthPacket> auth;
};

/*
 * CONNECT packet (high level fields)
 * - protocol name/version
 * - clean start
 * - keep alive
 * - properties
 * - payload: client id, will, user name, password
 */
struct MqttPacket::ConnectPacket {
    std::string protocol_name; // "MQTT"
    uint8_t protocol_version;  // 5
    uint8_t connect_flags;
    uint16_t keep_alive_seconds;

    // Properties (MQTT v5)
    std::vector<Property> properties;
    UserPropertyList user_properties; // convenience

    // Payload fields
    std::string client_id;
    // will, username, password omitted for brevity
};

/*
 * CONNACK
 */
struct MqttPacket::ConnAckPacket {
    ReasonCode reason_code;
    std::vector<Property> properties;
    UserPropertyList user_properties;
};

/*
AUTH packet (MQTT v5) — used for extended authentication.

  For PQC we will:
   - use 'auth_method' to indicate "pqc-kem-v1" or similar
   - place server's public key into 'auth_data' (binary) OR as a binary property
   - client will send 'auth_data' containing the KEM ciphertext (binary)

 AUTH fields:
   - auth_method: UTF-8 string
   - auth_data: binary (may be empty or contain pubkey/ciphertext)
   - reason_string: optional UTF-8
   - properties (including user properties)
 */
struct MqttPacket::AuthPacket {
    std::string auth_method;              // e.g. "pqc-kem-v1"
    std::vector<uint8_t> auth_data;       // binary blob (pubkey or ciphertext)
    std::optional<std::string> reason_string;
    std::vector<Property> properties;
    UserPropertyList user_properties;     // e.g. pqc.alg = "kyber512"
};

struct MqttPacket::PublishPacket {
    uint8_t dup;
    uint8_t qos;          // qos level, can be 0,1 or 2
    bool retain;
    std::string topic_name;
    std::optional<uint16_t> packet_identifier; // present if QoS > 0

    std::vector<Property> properties;  // message properties
    UserPropertyList user_properties;  // convenience for user properties
    std::vector<uint8_t> payload;      // raw payload bytes

    // optional helpers to identify our app-layer encryption header
    bool payload_is_encrypted = false;
    std::string payload_encryption_alg; // to be defined by user
};

/*
 PQC-specific constants (strings for user properties / auth method names)
 */
namespace pqc {
    inline const char* AUTH_METHOD_PQC_KEM = "pqc-kem-v1";
    inline const char* USER_PROP_PQC_ALG = "pqc.alg";   // any NIST apprroved algorrithm
    inline const char* USER_PROP_PQC_MODE = "pqc.mode"; // PQC orr classic
    inline const char* USER_PROP_PQC_NONCE = "pqc.nonce"; // support for base64/hex nonce
}

/*
 NOTE:
  - Serializer/Deserializer must implement Remaining Length encoding/decoding.
  - Properties need to be encoded using MQTT v5 property IDs & types.
  - For PQC, prefer putting public keys/ciphertexts in `auth_data` (binary).User properties hold alg names and small flags only.
  - Enforce size limits for binary properties and auth_data to avoid DoS.
 */


// Encode a varint (Remaining Length) per MQTT spec; returns bytes
std::vector<uint8_t> encodeRemainingLength(uint32_t len);

// Decode remaining length from buffer; returns decoded value and bytes consumed
std::pair<uint32_t, size_t> decodeRemainingLength(const uint8_t* data, size_t data_len);

// Pack orr unpack user properties helper
void add_user_property(UserPropertyList &list, const std::string &name, const std::string &value);

// Utility: add PQC user property convenience
inline void set_pqc_user_props(UserPropertyList &props, const std::string &alg) {
    add_user_property(props, pqc::USER_PROP_PQC_ALG, alg);
}
}
