#include "Gma.h"

EFI_DRIVER_SUPPORTED_EFI_VERSION_PROTOCOL gQemuVideoDriverSupportedEfiVersion = {
  sizeof (EFI_DRIVER_SUPPORTED_EFI_VERSION_PROTOCOL), // Size of Protocol structure.
  0                                                   // Version number to be filled at start up.
};

