#include <IndustryStandard/VmwareSvga.h>
#include "Gma.h"

EFI_STATUS
QemuVideoHdGfxModeSetup (
  QEMU_VIDEO_PRIVATE_DATA *Private
  )
{
  struct lb_framebuffer*fb = getFb();

  Private->ModeData = AllocatePool (
                        sizeof (Private->ModeData[0]) 
                        );
  if (Private->ModeData == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  DebugPrint(0, "X Res %d\n", fb->x_resolution);
  Private->ModeData->InternalModeIndex = 0;
  Private->ModeData->HorizontalResolution = fb->x_resolution;
  Private->ModeData->VerticalResolution = fb->y_resolution;
  Private->ModeData->ColorDepth = fb->bpp;
  Private->MaxMode = 1;

  gma_func0_init(Private);
  return EFI_SUCCESS;
}
