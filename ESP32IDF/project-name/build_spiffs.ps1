cd C:\Users\jatan\esp\v5.2.2\esp-idf\components\spiffs
python spiffsgen.py 0x160000 C:\Users\jatan\Documents\GitHub\turn_systems\ESP32Wifi\ESP32IDF\project-name\data C:\Users\jatan\Documents\GitHub\turn_systems\ESP32Wifi\ESP32IDF\project-name\w_spiffs
cd C:\Users\jatan\esp\v5.2.2\esp-idf\components\esptool_py\esptool
cd C:\Users\jatan\esp\v5.2.2\esp-idf\
esptool.py --chip esp32 --port com26 --baud 921600 erase_region 0x290000 0x160000
esptool.py --port com26 write_flash --flash_mode qio --flash_size 4MB 0x290000 C:\Users\jatan\Documents\GitHub\turn_systems\ESP32Wifi\ESP32IDF\project-name\w_spiffs