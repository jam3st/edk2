/** @file
  Graphics Output Protocol functions for the QEMU video controller.

  Copyright (c) 2007 - 2010, Intel Corporation. All rights reserved.<BR>

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution. The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <IndustryStandard/VmwareSvga.h>
#include "Qemu.h"

///
/// Generic Attribute Controller Register Settings
///
UINT8  AttributeController[21] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
  0x41, 0x00, 0x0F, 0x00, 0x00
};

///
/// Generic Graphics Controller Register Settings
///
UINT8 GraphicsController[9] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF
};

//
// 640 x 480 x 256 color @ 60 Hertz
//
UINT8 Crtc_640_480_256_60[28] = {
  0x5d, 0x4f, 0x50, 0x82, 0x53, 0x9f, 0x00, 0x3e,
  0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xe1, 0x83, 0xdf, 0x50, 0x00, 0xe7, 0x04, 0xe3,
  0xff, 0x00, 0x00, 0x22
};

UINT8 Crtc_640_480_32bpp_60[28] = {
  0x5d, 0x4f, 0x50, 0x82, 0x53, 0x9f, 0x00, 0x3e,
  0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xe1, 0x83, 0xdf, 0x40, 0x00, 0xe7, 0x04, 0xe3,
  0xff, 0x00, 0x00, 0x32
};

UINT16 Seq_640_480_256_60[15] = {
  0x0100, 0x0101, 0x0f02, 0x0003, 0x0e04, 0x1107, 0x0008, 0x4a0b,
  0x5b0c, 0x450d, 0x7e0e, 0x2b1b, 0x2f1c, 0x301d, 0x331e
};

UINT16 Seq_640_480_32bpp_60[15] = {
  0x0100, 0x0101, 0x0f02, 0x0003, 0x0e04, 0x1907, 0x0008, 0x4a0b,
  0x5b0c, 0x450d, 0x7e0e, 0x2b1b, 0x2f1c, 0x301d, 0x331e
};

//
// 800 x 600 x 256 color @ 60 Hertz
//
UINT8 Crtc_800_600_256_60[28] = {
  0x7F, 0x63, 0x64, 0x80, 0x6B, 0x1B, 0x72, 0xF0,
  0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x58, 0x8C, 0x57, 0x64, 0x00, 0x5F, 0x91, 0xE3,
  0xFF, 0x00, 0x00, 0x22
};

UINT8 Crtc_800_600_32bpp_60[28] = {
  0x7F, 0x63, 0x64, 0x80, 0x6B, 0x1B, 0x72, 0xF0,
  0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x58, 0x8C, 0x57, 0x90, 0x00, 0x5F, 0x91, 0xE3,
  0xFF, 0x00, 0x00, 0x32
};

UINT16 Seq_800_600_256_60[15] = {
  0x0100, 0x0101, 0x0f02, 0x0003, 0x0e04, 0x1107, 0x0008, 0x4a0b,
  0x5b0c, 0x450d, 0x510e, 0x2b1b, 0x2f1c, 0x301d, 0x3a1e
};

UINT16 Seq_800_600_32bpp_60[15] = {
  0x0100, 0x0101, 0x0f02, 0x0003, 0x0e04, 0x1907, 0x0008, 0x4a0b,
  0x5b0c, 0x450d, 0x510e, 0x2b1b, 0x2f1c, 0x301d, 0x3a1e
};

UINT8 Crtc_960_720_32bpp_60[28] = {
  0xA3, 0x77, 0x80, 0x86, 0x85, 0x96, 0x24, 0xFD,
  0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x02, 0x88, 0xCF, 0xe0, 0x00, 0x00, 0x64, 0xE3,
  0xFF, 0x4A, 0x00, 0x32
};

UINT16 Seq_960_720_32bpp_60[15] = {
  0x0100, 0x0101, 0x0f02, 0x0003, 0x0e04, 0x1907, 0x0008, 0x4a0b,
  0x5b0c, 0x450d, 0x760e, 0x2b1b, 0x2f1c, 0x301d, 0x341e
};

//
// 1024 x 768 x 256 color @ 60 Hertz
//
UINT8 Crtc_1024_768_256_60[28] = {
  0xA3, 0x7F, 0x80, 0x86, 0x85, 0x96, 0x24, 0xFD,
  0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x02, 0x88, 0xFF, 0x80, 0x00, 0x00, 0x24, 0xE3,
  0xFF, 0x4A, 0x00, 0x22
};

UINT16 Seq_1024_768_256_60[15] = {
  0x0100, 0x0101, 0x0f02, 0x0003, 0x0e04, 0x1107, 0x0008, 0x4a0b,
  0x5b0c, 0x450d, 0x760e, 0x2b1b, 0x2f1c, 0x301d, 0x341e
};

//
// 1024 x 768 x 24-bit color @ 60 Hertz
//
UINT8 Crtc_1024_768_24bpp_60[28] = {
  0xA3, 0x7F, 0x80, 0x86, 0x85, 0x96, 0x24, 0xFD,
  0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x02, 0x88, 0xFF, 0x80, 0x00, 0x00, 0x24, 0xE3,
  0xFF, 0x4A, 0x00, 0x32
};

UINT16 Seq_1024_768_24bpp_60[15] = {
  0x0100, 0x0101, 0x0f02, 0x0003, 0x0e04, 0x1507, 0x0008, 0x4a0b,
  0x5b0c, 0x450d, 0x760e, 0x2b1b, 0x2f1c, 0x301d, 0x341e
};

UINT8 Crtc_1024_768_32bpp_60[28] = {
  0xA3, 0x7F, 0x80, 0x86, 0x85, 0x96, 0x24, 0xFD,
  0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x02, 0x88, 0xFF, 0xe0, 0x00, 0x00, 0x64, 0xE3,
  0xFF, 0x4A, 0x00, 0x32
};

UINT16 Seq_1024_768_32bpp_60[15] = {
  0x0100, 0x0101, 0x0f02, 0x0003, 0x0e04, 0x1907, 0x0008, 0x4a0b,
  0x5b0c, 0x450d, 0x760e, 0x2b1b, 0x2f1c, 0x301d, 0x341e
};

///
/// Table of supported video modes
///
QEMU_VIDEO_CIRRUS_MODES  QemuVideoCirrusModes[] = {
//  {  640, 480, 8, Crtc_640_480_256_60,  Seq_640_480_256_60,  0xe3 },
//  {  800, 600, 8, Crtc_800_600_256_60,  Seq_800_600_256_60,  0xef },
  {  640, 480, 32, Crtc_640_480_32bpp_60,  Seq_640_480_32bpp_60,  0xef },
  {  800, 600, 32, Crtc_800_600_32bpp_60,  Seq_800_600_32bpp_60,  0xef },
//  { 1024, 768, 8, Crtc_1024_768_256_60, Seq_1024_768_256_60, 0xef }
  { 1024, 768, 24, Crtc_1024_768_24bpp_60, Seq_1024_768_24bpp_60, 0xef }
//  { 1024, 768, 32, Crtc_1024_768_32bpp_60, Seq_1024_768_32bpp_60, 0xef }
//  { 960, 720, 32, Crtc_960_720_32bpp_60, Seq_1024_768_32bpp_60, 0xef }
};

#define QEMU_VIDEO_CIRRUS_MODE_COUNT \
  (ARRAY_SIZE (QemuVideoCirrusModes))

/**
  Construct the valid video modes for QemuVideo.

**/
EFI_STATUS
QemuVideoCirrusModeSetup (
  QEMU_VIDEO_PRIVATE_DATA  *Private
  )
{
  UINT32                                 Index;
  QEMU_VIDEO_MODE_DATA                   *ModeData;
  QEMU_VIDEO_CIRRUS_MODES                *VideoMode;

  //
  // Setup Video Modes
  //
  Private->ModeData = AllocatePool (
                        sizeof (Private->ModeData[0]) * QEMU_VIDEO_CIRRUS_MODE_COUNT
                        );
  if (Private->ModeData == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  ModeData = Private->ModeData;
  VideoMode = &QemuVideoCirrusModes[0];
  for (Index = 0; Index < QEMU_VIDEO_CIRRUS_MODE_COUNT; Index ++) {
    ModeData->InternalModeIndex = Index;
    ModeData->HorizontalResolution          = VideoMode->Width;
    ModeData->VerticalResolution            = VideoMode->Height;
    ModeData->ColorDepth                    = VideoMode->ColorDepth;
    DEBUG ((EFI_D_INFO,
      "Adding Mode %d as Cirrus Internal Mode %d: %dx%d, %d-bit\n",
      (INT32) (ModeData - Private->ModeData),
      ModeData->InternalModeIndex,
      ModeData->HorizontalResolution,
      ModeData->VerticalResolution,
      ModeData->ColorDepth
      ));

    ModeData ++ ;
    VideoMode ++;
  }
  Private->MaxMode = ModeData - Private->ModeData;

  return EFI_SUCCESS;
}

///
/// Table of supported video modes
///
QEMU_VIDEO_BOCHS_MODES  QemuVideoBochsModes[] = {
  { 1600,  900, 32 }
};

#define QEMU_VIDEO_BOCHS_MODE_COUNT \
  (ARRAY_SIZE (QemuVideoBochsModes))

EFI_STATUS
QemuVideoBochsModeSetup (
  QEMU_VIDEO_PRIVATE_DATA  *Private,
  BOOLEAN                  IsQxl
  )
{
  UINT32                                 AvailableFbSize;
  UINT32                                 Index;
  QEMU_VIDEO_MODE_DATA                   *ModeData;
  QEMU_VIDEO_BOCHS_MODES                 *VideoMode;

  //
  // Fetch the available framebuffer size.
  //
  // VBE_DISPI_INDEX_VIDEO_MEMORY_64K is expected to return the size of the
  // drawable framebuffer. Up to and including qemu-2.1 however it used to
  // return the size of PCI BAR 0 (ie. the full video RAM size).
  //
  // On stdvga the two concepts coincide with each other; the full memory size
  // is usable for drawing.
  //
  // On QXL however, only a leading segment, "surface 0", can be used for
  // drawing; the rest of the video memory is used for the QXL guest-host
  // protocol. VBE_DISPI_INDEX_VIDEO_MEMORY_64K should report the size of
  // "surface 0", but since it doesn't (up to and including qemu-2.1), we
  // retrieve the size of the drawable portion from a field in the QXL ROM BAR,
  // where it is also available.
  //
  if (IsQxl) {
    UINT32 Signature;
    UINT32 DrawStart;

    Signature = 0;
    DrawStart = 0xFFFFFFFF;
    AvailableFbSize = 0;
    if (EFI_ERROR (
          Private->PciIo->Mem.Read (Private->PciIo, EfiPciIoWidthUint32,
                                PCI_BAR_IDX2, 0, 1, &Signature)) ||
        Signature != SIGNATURE_32 ('Q', 'X', 'R', 'O') ||
        EFI_ERROR (
          Private->PciIo->Mem.Read (Private->PciIo, EfiPciIoWidthUint32,
                                PCI_BAR_IDX2, 36, 1, &DrawStart)) ||
        DrawStart != 0 ||
        EFI_ERROR (
          Private->PciIo->Mem.Read (Private->PciIo, EfiPciIoWidthUint32,
                                PCI_BAR_IDX2, 40, 1, &AvailableFbSize))) {
      DEBUG ((EFI_D_ERROR, "%a: can't read size of drawable buffer from QXL "
        "ROM\n", __FUNCTION__));
      return EFI_NOT_FOUND;
    }
  } else {
    AvailableFbSize  = BochsRead (Private, VBE_DISPI_INDEX_VIDEO_MEMORY_64K);
    AvailableFbSize *= SIZE_64KB;
  }
  DEBUG ((EFI_D_INFO, "%a: AvailableFbSize=0x%x\n", __FUNCTION__,
    AvailableFbSize));

  //
  // Setup Video Modes
  //
  Private->ModeData = AllocatePool (
                        sizeof (Private->ModeData[0]) * QEMU_VIDEO_BOCHS_MODE_COUNT
                        );
  if (Private->ModeData == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  ModeData = Private->ModeData;
  VideoMode = &QemuVideoBochsModes[0];
  for (Index = 0; Index < QEMU_VIDEO_BOCHS_MODE_COUNT; Index ++) {
    UINTN RequiredFbSize;

    ASSERT (VideoMode->ColorDepth % 8 == 0);
    RequiredFbSize = (UINTN) VideoMode->Width * VideoMode->Height *
                     (VideoMode->ColorDepth / 8);
    if (RequiredFbSize <= AvailableFbSize) {
      ModeData->InternalModeIndex    = Index;
      ModeData->HorizontalResolution = VideoMode->Width;
      ModeData->VerticalResolution   = VideoMode->Height;
      ModeData->ColorDepth           = VideoMode->ColorDepth;
      DEBUG ((EFI_D_INFO,
        "Adding Mode %d as Bochs Internal Mode %d: %dx%d, %d-bit\n",
        (INT32) (ModeData - Private->ModeData),
        ModeData->InternalModeIndex,
        ModeData->HorizontalResolution,
        ModeData->VerticalResolution,
        ModeData->ColorDepth
        ));

      ModeData ++ ;
    }
    VideoMode ++;
  }
  Private->MaxMode = ModeData - Private->ModeData;

  return EFI_SUCCESS;
}

EFI_STATUS
QemuVideoVmwareSvgaModeSetup (
  QEMU_VIDEO_PRIVATE_DATA *Private
  )
{
  EFI_STATUS                            Status;
  UINT32                                FbSize;
  UINT32                                MaxWidth, MaxHeight;
  UINT32                                Capabilities;
  UINT32                                BitsPerPixel;
  UINT32                                Index;
  QEMU_VIDEO_MODE_DATA                  *ModeData;
  QEMU_VIDEO_BOCHS_MODES                *VideoMode;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  *ModeInfo;

  VmwareSvgaWrite (Private, VmwareSvgaRegEnable, 0);

  Private->ModeData =
    AllocatePool (sizeof (Private->ModeData[0]) * QEMU_VIDEO_BOCHS_MODE_COUNT);
  if (Private->ModeData == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ModeDataAllocError;
  }

  Private->VmwareSvgaModeInfo =
    AllocatePool (
      sizeof (Private->VmwareSvgaModeInfo[0]) * QEMU_VIDEO_BOCHS_MODE_COUNT
      );
  if (Private->VmwareSvgaModeInfo == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ModeInfoAllocError;
  }

  FbSize =       VmwareSvgaRead (Private, VmwareSvgaRegFbSize);
  MaxWidth =     VmwareSvgaRead (Private, VmwareSvgaRegMaxWidth);
  MaxHeight =    VmwareSvgaRead (Private, VmwareSvgaRegMaxHeight);
  Capabilities = VmwareSvgaRead (Private, VmwareSvgaRegCapabilities);
  if ((Capabilities & VMWARE_SVGA_CAP_8BIT_EMULATION) != 0) {
    BitsPerPixel = VmwareSvgaRead (
                     Private,
                     VmwareSvgaRegHostBitsPerPixel
                     );
    VmwareSvgaWrite (
      Private,
      VmwareSvgaRegBitsPerPixel,
      BitsPerPixel
      );
  } else {
    BitsPerPixel = VmwareSvgaRead (
                     Private,
                     VmwareSvgaRegBitsPerPixel
                     );
  }

  if (FbSize == 0       ||
      MaxWidth == 0     ||
      MaxHeight == 0    ||
      BitsPerPixel == 0 ||
      BitsPerPixel % 8 != 0) {
    Status = EFI_DEVICE_ERROR;
    goto Rollback;
  }

  ModeData = Private->ModeData;
  ModeInfo = Private->VmwareSvgaModeInfo;
  VideoMode = &QemuVideoBochsModes[0];
  for (Index = 0; Index < QEMU_VIDEO_BOCHS_MODE_COUNT; Index++) {
    UINTN RequiredFbSize;

    RequiredFbSize = (UINTN) VideoMode->Width * VideoMode->Height *
                     (BitsPerPixel / 8);
    if (RequiredFbSize <= FbSize     &&
        VideoMode->Width <= MaxWidth &&
        VideoMode->Height <= MaxHeight) {
      UINT32  BytesPerLine;
      UINT32  RedMask, GreenMask, BlueMask, PixelMask;

      VmwareSvgaWrite (
        Private,
        VmwareSvgaRegWidth,
        VideoMode->Width
        );
      VmwareSvgaWrite (
        Private,
        VmwareSvgaRegHeight,
        VideoMode->Height
        );

      ModeData->InternalModeIndex    = Index;
      ModeData->HorizontalResolution = VideoMode->Width;
      ModeData->VerticalResolution   = VideoMode->Height;
      ModeData->ColorDepth           = BitsPerPixel;

      //
      // Setting VmwareSvgaRegWidth/VmwareSvgaRegHeight actually changes
      // the device's display mode, so we save all properties of each mode up
      // front to avoid inadvertent mode changes later.
      //
      ModeInfo->Version = 0;
      ModeInfo->HorizontalResolution = ModeData->HorizontalResolution;
      ModeInfo->VerticalResolution   = ModeData->VerticalResolution;

      ModeInfo->PixelFormat = PixelBitMask;

      RedMask   = VmwareSvgaRead (Private, VmwareSvgaRegRedMask);
      ModeInfo->PixelInformation.RedMask = RedMask;

      GreenMask = VmwareSvgaRead (Private, VmwareSvgaRegGreenMask);
      ModeInfo->PixelInformation.GreenMask = GreenMask;

      BlueMask  = VmwareSvgaRead (Private, VmwareSvgaRegBlueMask);
      ModeInfo->PixelInformation.BlueMask = BlueMask;

      //
      // Reserved mask is whatever bits in the pixel not containing RGB data,
      // so start with binary 1s for every bit in the pixel, then mask off
      // bits already used for RGB. Special case 32 to avoid undefined
      // behaviour in the shift.
      //
      if (BitsPerPixel == 32) {
        if (BlueMask == 0xff && GreenMask == 0xff00 && RedMask == 0xff0000) {
          ModeInfo->PixelFormat = PixelBlueGreenRedReserved8BitPerColor;
        } else if (BlueMask == 0xff0000 &&
                   GreenMask == 0xff00 &&
                   RedMask == 0xff) {
          ModeInfo->PixelFormat = PixelRedGreenBlueReserved8BitPerColor;
        }
        PixelMask = MAX_UINT32;
      } else {
        PixelMask = (1u << BitsPerPixel) - 1;
      }
      ModeInfo->PixelInformation.ReservedMask =
        PixelMask & ~(RedMask | GreenMask | BlueMask);

      BytesPerLine = VmwareSvgaRead (Private, VmwareSvgaRegBytesPerLine);
      ModeInfo->PixelsPerScanLine = BytesPerLine / (BitsPerPixel / 8);

      ModeData++;
      ModeInfo++;
    }
    VideoMode++;
  }
  Private->MaxMode = ModeData - Private->ModeData;
  return EFI_SUCCESS;

Rollback:
  FreePool (Private->VmwareSvgaModeInfo);
  Private->VmwareSvgaModeInfo = NULL;

ModeInfoAllocError:
  FreePool (Private->ModeData);
  Private->ModeData = NULL;

ModeDataAllocError:
  return Status;
}
