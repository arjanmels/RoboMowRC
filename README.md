# RoboMowRC
**This is a very early version not ready for use!**

Project to monitor and control a RoboMow lawn mower via Bluetooth via an ESP32.
Currently targeted to TTGO T-Beam board, which includes GPS and LoRaWan functionality.

###L
###Requirements
Currently a board with PSRAM and 4GB of Flash is required. To use OTA you need 8GB of flash.
###ToDo
 - [ ] General: Reduce Heap usage

 - [ ] Check if bluetooth works otherwise revert to cpp_utils/BLE classes

 - [ ] Lots of things...
 - [ ] Homepage with RoboMow status view and controls
 - [ ] RS and RC series status
 - [ ] Lora: add frequency plan to UI
 - [ ] Lora: add framecounter reset to UI
 - [ ] Lora: change to multi channel

- [ ] Webpage: only auto start portal when no credentials configured
- [ ] Webpage: enable start portal on button press

- [ ] General: fix partion table to allow OTA
- [ ] General: shrink code size to allow ota
- [ ] General: shrink memory usage to be able to run without PSRAM
- [ ] General: power saving

- [ ] GPS: Store data for faster acquisition
- [ ] GPS: upload time for faster acquisition

###Done
- [x] Lora: send old GPS coordinate over different Fport (to avoid polluting TTN mapper)
- [x] Lora: send SF12 message every 6 hours and immediatly after theft?,
- [x] Lora: send regular SF7 message max every 5 minutes, normally every 15 minutes
- [x] Lora: add keys to UI
- [x] Lora: store frame counter in nvm
- [x] General: remove spiffs replace with NVM
 