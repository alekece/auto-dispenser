cmake_minimum_required(VERSION 3.10)

project(food-dispenser CXX)

set(ARDUINO_BOARD "" CACHE STRING "")
set(ARDUINO_PORT "" CACHE STRING "")
set(BLYNK_TOKEN "" CACHE STRING "")
set(NETWORK_SSID "" CACHE STRING "")
set(NETWORK_PASSWORD "" CACHE STRING "")
set(SERVER_ADDRESS "blynk-cloud.com" CACHE STRING "")
set(SERVER_PORT 80 CACHE STRING "")

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/app/Secrets.h.in
  ${CMAKE_CURRENT_SOURCE_DIR}/app/Secrets.h
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/app/sketch.json.in
  ${CMAKE_CURRENT_SOURCE_DIR}/app/sketch.json
)

add_custom_target(configure
  arduino-cli core update-index --additional-urls https://arduino.esp8266.com/stable/package_esp8266com_index.json
  COMMAND arduino-cli core install esp8266:esp8266
)

add_custom_target(compile
  COMMAND arduino-cli compile --libraries lib app
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(upload
  COMMAND arduino-cli upload app
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(compile-and-upload ALL
  COMMAND arduino-cli compile --libraries lib -u app
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
