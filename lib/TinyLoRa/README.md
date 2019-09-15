# Modified Adafruit TinyLoRa Library [![Build Status](https://travis-ci.org/adafruit/TinyLoRa.svg?branch=master)](https://travis-ci.org/adafruit/TinyLoRa)


Library for communicating with [The Things Network](https://www.thethingsnetwork.org/) using a Hope RF RFM95/96/97/98(W) LoRa Transceiver Module.

## Modifications
Allow to use this library on other CPU's:
- Add SPI.endTransaction()
- Add pins as parameters to begin() function

Implemented asynchronous send

## Documentation/Links

The Doxygen documentation is automatically generated from the source files
in this repository, and documents the API exposed by this driver. For
practical details on how to connect and use this sensor, consult the Learning
Guide.

- [Adafruit Learning Guide](https://learn.adafruit.com/the-things-network-for-feather/using-a-feather-32u4)
  (Installation details for Feather 32u4/Feather M0.)
- [API Documentation](https://adafruit.github.io/TinyLoRa/) (automatically generated via doxygen from source)
- [Adafruit Feather 32u4 RFM95 LoRa Radio](https://www.adafruit.com/product/3078)
- [Adafruit Feather M0 with RFM95 LoRa Radio](https://www.adafruit.com/product/3178)

## License

This library was written by [ClemensRiederer](https://github.com/ClemensRiederer/TinyLoRa-BME280). We've modified it to support channel/datarate selections and made small changes so it works with the Adafruit Feather 32u4 LoRa and the Adafruit Feather M0 LoRa. We've added examples for using this library to transmit sensor data over a single channel or multiple channels to The Things Network.

This open source code is licensed under the LGPL license (see [LICENSE](LICENSE)
for details).
