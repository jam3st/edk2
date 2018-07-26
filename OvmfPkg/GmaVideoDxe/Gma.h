/** @file
  QEMU Video Controller Driver

  Copyright (c) 2006 - 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

//
// QEMU Video Controller Driver
//

#ifndef _QEMU_H_
#define _QEMU_H_


#include <Uefi.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/PciIo.h>
#include <Protocol/DriverSupportedEfiVersion.h>
#include <Protocol/DevicePath.h>

#include <Library/DebugLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiLib.h>
#include <Library/PcdLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DevicePathLib.h>
#include <Library/TimerLib.h>
#include <Library/FrameBufferBltLib.h>

#include <IndustryStandard/Pci.h>
#include <IndustryStandard/Acpi.h>

struct lb_framebuffer {
    UINT64 physical_address;
    UINT64 x_resolution;
    UINT64 y_resolution;
    UINT64 bpp;
};

// QEMU Vide Graphical Mode Data
//
typedef struct {
  UINT32  InternalModeIndex; // points into card-specific mode table
  UINT32  HorizontalResolution;
  UINT32  VerticalResolution;
  UINT32  ColorDepth;
} QEMU_VIDEO_MODE_DATA;

#define PIXEL_RED_SHIFT   0
#define PIXEL_GREEN_SHIFT 3
#define PIXEL_BLUE_SHIFT  6

#define PIXEL_RED_MASK    (BIT7 | BIT6 | BIT5)
#define PIXEL_GREEN_MASK  (BIT4 | BIT3 | BIT2)
#define PIXEL_BLUE_MASK   (BIT1 | BIT0)

#define PIXEL_TO_COLOR_BYTE(pixel, mask, shift) ((UINT8) ((pixel & mask) << shift))
#define PIXEL_TO_RED_BYTE(pixel) PIXEL_TO_COLOR_BYTE(pixel, PIXEL_RED_MASK, PIXEL_RED_SHIFT)
#define PIXEL_TO_GREEN_BYTE(pixel) PIXEL_TO_COLOR_BYTE(pixel, PIXEL_GREEN_MASK, PIXEL_GREEN_SHIFT)
#define PIXEL_TO_BLUE_BYTE(pixel) PIXEL_TO_COLOR_BYTE(pixel, PIXEL_BLUE_MASK, PIXEL_BLUE_SHIFT)

#define RGB_BYTES_TO_PIXEL(Red, Green, Blue) \
  (UINT8) ( (((Red) >> PIXEL_RED_SHIFT) & PIXEL_RED_MASK) | \
            (((Green) >> PIXEL_GREEN_SHIFT) & PIXEL_GREEN_MASK) | \
            (((Blue) >> PIXEL_BLUE_SHIFT) & PIXEL_BLUE_MASK) )

#define PIXEL24_RED_MASK    0x00ff0000
#define PIXEL24_GREEN_MASK  0x0000ff00
#define PIXEL24_BLUE_MASK   0x000000ff

typedef enum {
  QEMU_VIDEO_INTEL_HDG = 1
} QEMU_VIDEO_VARIANT;

typedef struct {
  UINT8                                 SubClass;
  UINT16                                VendorId;
  UINT16                                DeviceId;
  QEMU_VIDEO_VARIANT                    Variant;
  CHAR16                                *Name;
} QEMU_VIDEO_CARD;

typedef struct {
  UINT64                                Signature;
  EFI_HANDLE                            Handle;
  EFI_PCI_IO_PROTOCOL                   *PciIo;
  UINT64                                OriginalPciAttributes;
  EFI_GRAPHICS_OUTPUT_PROTOCOL          GraphicsOutput;
  EFI_DEVICE_PATH_PROTOCOL              *GopDevicePath;

  //
  // The next two fields match the client-visible
  // EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE.MaxMode field.
  //
  UINTN                                 MaxMode;
  QEMU_VIDEO_MODE_DATA                  *ModeData;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  *VmwareSvgaModeInfo;

  QEMU_VIDEO_VARIANT                    Variant;
  FRAME_BUFFER_CONFIGURE                *FrameBufferBltConfigure;
  UINTN                                 FrameBufferBltConfigureSize;
  UINT8                                 FrameBufferVramBarIndex;
  UINT16                                VmwareSvgaBasePort;
} QEMU_VIDEO_PRIVATE_DATA;

///
/// Card-specific Video Mode structures
///
typedef struct {
  UINT32  Width;
  UINT32  Height;
  UINT32  ColorDepth;
  UINT8   *CrtcSettings;
  UINT16  *SeqSettings;
  UINT8   MiscSetting;
} QEMU_VIDEO_CIRRUS_MODES;

typedef struct {
  UINT32  Width;
  UINT32  Height;
  UINT32  ColorDepth;
} QEMU_VIDEO_BOCHS_MODES;

#define QEMU_VIDEO_PRIVATE_DATA_FROM_GRAPHICS_OUTPUT_THIS(a) \
  CR(a, QEMU_VIDEO_PRIVATE_DATA, GraphicsOutput, QEMU_VIDEO_PRIVATE_DATA_SIGNATURE)


//
// Global Variables
//
extern UINT8                                      AttributeController[];
extern UINT8                                      GraphicsController[];
extern UINT8                                      Crtc_640_480_256_60[];
extern UINT16                                     Seq_640_480_256_60[];
extern UINT8                                      Crtc_800_600_256_60[];
extern UINT16                                     Seq_800_600_256_60[];
extern UINT8                                      Crtc_1024_768_256_60[];
extern UINT16                                     Seq_1024_768_256_60[];
extern QEMU_VIDEO_CIRRUS_MODES                    QemuVideoCirrusModes[];
extern QEMU_VIDEO_BOCHS_MODES                     QemuVideoBochsModes[];
extern EFI_DRIVER_BINDING_PROTOCOL                gQemuVideoDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL                gQemuVideoComponentName;
extern EFI_COMPONENT_NAME2_PROTOCOL               gQemuVideoComponentName2;
extern EFI_DRIVER_SUPPORTED_EFI_VERSION_PROTOCOL  gQemuVideoDriverSupportedEfiVersion;

// Graphics Output Hardware abstraction internal worker functions
//
EFI_STATUS
QemuVideoGraphicsOutputConstructor (
  QEMU_VIDEO_PRIVATE_DATA  *Private
  );

EFI_STATUS
QemuVideoGraphicsOutputDestructor (
  QEMU_VIDEO_PRIVATE_DATA  *Private
  );


EFI_STATUS
EFIAPI
QemuVideoControllerDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

/**
  TODO: Add function description

  @param  This TODO: add argument description
  @param  Controller TODO: add argument description
  @param  RemainingDevicePath TODO: add argument description

  TODO: add return values

**/
EFI_STATUS
EFIAPI
QemuVideoControllerDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

/**
  TODO: Add function description

  @param  This TODO: add argument description
  @param  Controller TODO: add argument description
  @param  NumberOfChildren TODO: add argument description
  @param  ChildHandleBuffer TODO: add argument description

  TODO: add return values

**/
EFI_STATUS
EFIAPI
QemuVideoControllerDriverStop (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN UINTN                        NumberOfChildren,
  IN EFI_HANDLE                   *ChildHandleBuffer
  );

//
// EFI Component Name Functions
//
/**
  Retrieves a Unicode string that is the user readable name of the driver.

  This function retrieves the user readable name of a driver in the form of a
  Unicode string. If the driver specified by This has a user readable name in
  the language specified by Language, then a pointer to the driver name is
  returned in DriverName, and EFI_SUCCESS is returned. If the driver specified
  by This does not support the language specified by Language,
  then EFI_UNSUPPORTED is returned.

  @param  This[in]              A pointer to the EFI_COMPONENT_NAME2_PROTOCOL or
                                EFI_COMPONENT_NAME_PROTOCOL instance.

  @param  Language[in]          A pointer to a Null-terminated ASCII string
                                array indicating the language. This is the
                                language of the driver name that the caller is
                                requesting, and it must match one of the
                                languages specified in SupportedLanguages. The
                                number of languages supported by a driver is up
                                to the driver writer. Language is specified
                                in RFC 4646 or ISO 639-2 language code format.

  @param  DriverName[out]       A pointer to the Unicode string to return.
                                This Unicode string is the name of the
                                driver specified by This in the language
                                specified by Language.

  @retval EFI_SUCCESS           The Unicode string for the Driver specified by
                                This and the language specified by Language was
                                returned in DriverName.

  @retval EFI_INVALID_PARAMETER Language is NULL.

  @retval EFI_INVALID_PARAMETER DriverName is NULL.

  @retval EFI_UNSUPPORTED       The driver specified by This does not support
                                the language specified by Language.

**/
EFI_STATUS
EFIAPI
QemuVideoComponentNameGetDriverName (
  IN  EFI_COMPONENT_NAME_PROTOCOL  *This,
  IN  CHAR8                        *Language,
  OUT CHAR16                       **DriverName
  );


/**
  Retrieves a Unicode string that is the user readable name of the controller
  that is being managed by a driver.

  This function retrieves the user readable name of the controller specified by
  ControllerHandle and ChildHandle in the form of a Unicode string. If the
  driver specified by This has a user readable name in the language specified by
  Language, then a pointer to the controller name is returned in ControllerName,
  and EFI_SUCCESS is returned.  If the driver specified by This is not currently
  managing the controller specified by ControllerHandle and ChildHandle,
  then EFI_UNSUPPORTED is returned.  If the driver specified by This does not
  support the language specified by Language, then EFI_UNSUPPORTED is returned.

  @param  This[in]              A pointer to the EFI_COMPONENT_NAME2_PROTOCOL or
                                EFI_COMPONENT_NAME_PROTOCOL instance.

  @param  ControllerHandle[in]  The handle of a controller that the driver
                                specified by This is managing.  This handle
                                specifies the controller whose name is to be
                                returned.

  @param  ChildHandle[in]       The handle of the child controller to retrieve
                                the name of.  This is an optional parameter that
                                may be NULL.  It will be NULL for device
                                drivers.  It will also be NULL for a bus drivers
                                that wish to retrieve the name of the bus
                                controller.  It will not be NULL for a bus
                                driver that wishes to retrieve the name of a
                                child controller.

  @param  Language[in]          A pointer to a Null-terminated ASCII string
                                array indicating the language.  This is the
                                language of the driver name that the caller is
                                requesting, and it must match one of the
                                languages specified in SupportedLanguages. The
                                number of languages supported by a driver is up
                                to the driver writer. Language is specified in
                                RFC 4646 or ISO 639-2 language code format.

  @param  ControllerName[out]   A pointer to the Unicode string to return.
                                This Unicode string is the name of the
                                controller specified by ControllerHandle and
                                ChildHandle in the language specified by
                                Language from the point of view of the driver
                                specified by This.

  @retval EFI_SUCCESS           The Unicode string for the user readable name in
                                the language specified by Language for the
                                driver specified by This was returned in
                                DriverName.

  @retval EFI_INVALID_PARAMETER ControllerHandle is not a valid EFI_HANDLE.

  @retval EFI_INVALID_PARAMETER ChildHandle is not NULL and it is not a valid
                                EFI_HANDLE.

  @retval EFI_INVALID_PARAMETER Language is NULL.

  @retval EFI_INVALID_PARAMETER ControllerName is NULL.

  @retval EFI_UNSUPPORTED       The driver specified by This is not currently
                                managing the controller specified by
                                ControllerHandle and ChildHandle.

  @retval EFI_UNSUPPORTED       The driver specified by This does not support
                                the language specified by Language.

**/
EFI_STATUS
EFIAPI
QemuVideoComponentNameGetControllerName (
  IN  EFI_COMPONENT_NAME_PROTOCOL                     *This,
  IN  EFI_HANDLE                                      ControllerHandle,
  IN  EFI_HANDLE                                      ChildHandle        OPTIONAL,
  IN  CHAR8                                           *Language,
  OUT CHAR16                                          **ControllerName
  );

EFI_STATUS
QemuVideoHdGfxModeSetup (
  QEMU_VIDEO_PRIVATE_DATA *Private
  );

void hdgfx_adainit();
void gma_test_debugprint();
void gma_gfxinit(int* ok);
void gma_func0_init(QEMU_VIDEO_PRIVATE_DATA* dev);
int fill_lb_framebuffer(struct lb_framebuffer *framebuffer);
struct lb_framebuffer* getFb();


#endif
