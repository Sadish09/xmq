
# XMQ Protocol Specification

A lightweight, MQTT-compatible messaging protocol with Post-Quantum Key Establishment.

---

# ## 1. Introduction

XMQ is a Post-Quantum–secure extension of MQTT v5, designed to add:

* PQC KEM-based session establishment
* Application-layer payload encryption
* Backward compatibility with standard MQTT v5
* Pluggable crypto backend (liboqs + OpenSSL)
* Transport-agnostic design 

XMQ reuses the MQTT v5 wire format and extends it using:

* AUTH packets - for KEM exchange
* User Properties - for small metadata
* Auth Data - for binary PQC key material

MQTT clients or brokers unaware of this extension simply fall back to normal MQTT.

---

# 2. High-Level Architecture

[TBD]

---



