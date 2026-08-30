#include <Arduino.h>
#include "ESP_SR.h"  // Ensures srmodels.bin is staged for ESP SR partitions.
#include "esp_heap_caps.h"
#include "esp_partition.h"

constexpr uint32_t REPORT_INTERVAL_MS = 5000;
uint32_t lastReportMs = 0;

float bytesToMiB(size_t bytes) {
  return static_cast<float>(bytes) / (1024.0f * 1024.0f);
}

void printSize(const char *label, size_t bytes) {
  Serial.printf("%-27s %10u bytes  (%7.2f MiB)\n", label,
                static_cast<unsigned>(bytes), bytesToMiB(bytes));
}

void printChipAndFlash() {
  Serial.println("\n========== CHIP / FLASH ==========");
  Serial.printf("Chip model:                 %s\n", ESP.getChipModel());
  Serial.printf("Chip revision:              %u\n", ESP.getChipRevision());
  Serial.printf("CPU cores:                  %u\n", ESP.getChipCores());
  Serial.printf("CPU frequency:              %u MHz\n", ESP.getCpuFreqMHz());
  printSize("Physical flash size:", ESP.getFlashChipSize());
  Serial.printf("Flash speed:                %u MHz\n",
                static_cast<unsigned>(ESP.getFlashChipSpeed() / 1000000));
  printSize("Compiled sketch size:", ESP.getSketchSize());
  printSize("Free application space:", ESP.getFreeSketchSpace());
}

void printRuntimeMemory() {
  Serial.println("\n========== RUNTIME MEMORY ==========");

  const size_t internalTotal = heap_caps_get_total_size(
      MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  const size_t internalFree = heap_caps_get_free_size(
      MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  const size_t internalLargest = heap_caps_get_largest_free_block(
      MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  const size_t internalMinimum = heap_caps_get_minimum_free_size(
      MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);

  printSize("Internal RAM total:", internalTotal);
  printSize("Internal RAM free:", internalFree);
  printSize("Internal largest block:", internalLargest);
  printSize("Internal minimum free:", internalMinimum);

  if (!psramFound()) {
    Serial.println("PSRAM:                      NOT DETECTED");
    Serial.println("Check Tools > PSRAM > OPI PSRAM.");
    return;
  }

  const size_t psramTotal = heap_caps_get_total_size(
      MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  const size_t psramFree = heap_caps_get_free_size(
      MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  const size_t psramLargest = heap_caps_get_largest_free_block(
      MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  const size_t psramMinimum = heap_caps_get_minimum_free_size(
      MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

  printSize("PSRAM total:", psramTotal);
  printSize("PSRAM free:", psramFree);
  printSize("PSRAM largest block:", psramLargest);
  printSize("PSRAM minimum free:", psramMinimum);
}

const char *partitionTypeName(esp_partition_type_t type) {
  switch (type) {
    case ESP_PARTITION_TYPE_APP:
      return "APP";
    case ESP_PARTITION_TYPE_DATA:
      return "DATA";
    default:
      return "OTHER";
  }
}

void printPartitionTable() {
  Serial.println("\n========== PARTITION TABLE ==========");
  Serial.println("Label            Type   Subtype  Address     Size");

  esp_partition_iterator_t iterator = esp_partition_find(
      ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, nullptr);
  while (iterator != nullptr) {
    const esp_partition_t *partition = esp_partition_get(iterator);
    Serial.printf("%-16s %-6s 0x%02x     0x%08x  %7.2f MiB\n",
                  partition->label, partitionTypeName(partition->type),
                  partition->subtype,
                  static_cast<unsigned>(partition->address),
                  bytesToMiB(partition->size));
    iterator = esp_partition_next(iterator);
  }
  esp_partition_iterator_release(iterator);
}

void setup() {
  Serial.begin(115200);
  delay(1500);

  Serial.println("ESP32-S3 flash, RAM, and PSRAM diagnostic");
  printChipAndFlash();
  printRuntimeMemory();
  printPartitionTable();
  Serial.println("\nRuntime memory will be reported every 5 seconds.");
  lastReportMs = millis();
}

void loop() {
  if (millis() - lastReportMs >= REPORT_INTERVAL_MS) {
    lastReportMs = millis();
    printChipAndFlash();
    printRuntimeMemory();
  }
  delay(50);
}
