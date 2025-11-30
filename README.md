# XMQ 

A lightweight, MQTT-compatible messaging protocol with Post-Quantum Key Establishment.

# Overview 

XMQ provides:

- Custom Implementation of MQTT with modular components
- Hybrid Encryption Model - Both PQC via liboqs and Classical encryption algorithms via OpenSSL
- Delay Tolerant Networking (DTN) adapter via libdtn 
- Hardware optimized communication protocol for Industrial IoT or Military 

# Features 

- MQTT with quantum safe End-to-End encryption (E2EE) 
- Support for DTN methods 
- Modularity 

# Documentation 

Documentation for Build options, Design, PQC algorithms and Protocol can be dound in `docs/` 

# Acknowledgements 

This project is licensed under MIT, this project does not include ARM specific libraries for legal reasons. If you wish to use any consider licensing details.

This project depends on: 
- [liboqs](https://github.com/open-quantum-safe/liboqs) Supported by Post Quantum Cryptography Alliance as a part of Linux Foundation. 
- [OpenSSL](https://github.com/openssl/openssl) by OpenSSL org.

