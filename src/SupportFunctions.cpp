/*#include "MainHeader.h"

//prend un nombre d'octets en entrées, et retourne sa taille, formaté en Ko, Mo, et Go (pour SPIFFS)
String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "o";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "Ko";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "Mo";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "Go";
  }
}
*/