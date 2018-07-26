#include <IndustryStandard/Acpi.h>
#include "Gma.h"

EFI_DRIVER_BINDING_PROTOCOL gGmaVideoDriverBinding = {
  GmaVideoControllerDriverSupported,
  GmaVideoControllerDriverStart,
  GmaVideoControllerDriverStop,
  0x10,
  NULL,
  NULL
};


GMA_VIDEO_CARD gGmaVideoCardList[] = {
    {
        PCI_CLASS_DISPLAY_VGA,
        0x8086,
        0x0412,
        GMA_VIDEO_INTEL_HDG,
        L"Intel HD Graphics"
    },{
        0 /* end of list */
    }
};

static GMA_VIDEO_CARD*
GmaVideoDetect(
  IN UINT8 SubClass,
  IN UINT16 VendorId,
  IN UINT16 DeviceId
  )
{
  UINTN Index = 0;

  while (gGmaVideoCardList[Index].VendorId != 0) {
    if (gGmaVideoCardList[Index].SubClass == SubClass &&
        gGmaVideoCardList[Index].VendorId == VendorId &&
        gGmaVideoCardList[Index].DeviceId == DeviceId) {
      return gGmaVideoCardList + Index;
    }
    Index++;
  }
  return NULL;
}

static void memcpy(void *dest, const void *src, int n)
{
  unsigned char *d = dest;
  unsigned char const *s = src;

  while (n--)
    *d++ = *s++;
}


/**
  Check if this device is supported.

  @param  This                   The driver binding protocol.
  @param  Controller             The controller handle to check.
  @param  RemainingDevicePath    The remaining device path.

  @retval EFI_SUCCESS            The bus supports this controller.
  @retval EFI_UNSUPPORTED        This device isn't supported.

**/
EFI_STATUS
EFIAPI
GmaVideoControllerDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
{
  EFI_STATUS          Status;
  EFI_PCI_IO_PROTOCOL *PciIo;
  PCI_TYPE00          Pci;
  GMA_VIDEO_CARD     *Card;

  //
  // Open the PCI I/O Protocol
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  (VOID **) &PciIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Read the PCI Configuration Header from the PCI Device
  //
  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint32,
                        0,
                        sizeof (Pci) / sizeof (UINT32),
                        &Pci
                        );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  Status = EFI_UNSUPPORTED;
  if (!IS_PCI_DISPLAY (&Pci)) {
    goto Done;
  }
  Card = GmaVideoDetect(Pci.Hdr.ClassCode[1], Pci.Hdr.VendorId, Pci.Hdr.DeviceId);
  if (Card != NULL) {
    DebugPrint(0, "GmaVideo: %s detected\n", Card->Name);
    Status = EFI_SUCCESS;
  }

Done:
  //
  // Close the PCI I/O Protocol
  //
  gBS->CloseProtocol (
        Controller,
        &gEfiPciIoProtocolGuid,
        This->DriverBindingHandle,
        Controller
        );

  return Status;
}

/**
  Start to process the controller.

  @param  This                   The USB bus driver binding instance.
  @param  Controller             The controller to check.
  @param  RemainingDevicePath    The remaining device patch.

  @retval EFI_SUCCESS            The controller is controlled by the usb bus.
  @retval EFI_ALREADY_STARTED    The controller is already controlled by the usb
                                 bus.
  @retval EFI_OUT_OF_RESOURCES   Failed to allocate resources.

**/
EFI_STATUS
EFIAPI
GmaVideoControllerDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
{
  EFI_TPL                           OldTpl;
  EFI_STATUS                        Status;
  GMA_VIDEO_PRIVATE_DATA           *Private;
  EFI_DEVICE_PATH_PROTOCOL          *ParentDevicePath;
  ACPI_ADR_DEVICE_PATH              AcpiDeviceNode;
  PCI_TYPE00                        Pci;
  GMA_VIDEO_CARD                   *Card;
  EFI_PCI_IO_PROTOCOL               *ChildPciIo;
unsigned char x[256];

int i;

  OldTpl = gBS->RaiseTPL (TPL_CALLBACK);
DebugPrint(0, "GmaVideoControllerDriverStart\n");

  //
  // Allocate Private context data for GOP inteface.
  //
  Private = AllocateZeroPool (sizeof (GMA_VIDEO_PRIVATE_DATA));
  if (Private == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto RestoreTpl;
  }

  //
  // Set up context record
  //
  Private->Signature  = SIGNATURE_32 ('I', 'G', 'F', 'X') ;

  //
  // Open PCI I/O Protocol
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  (VOID **) &Private->PciIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
DebugPrint(0, "OpenProtocol %d\n", Status);
  if (EFI_ERROR (Status)) {
    goto FreePrivate;
  }

  //
  // Read the PCI Configuration Header from the PCI Device
  //
  Status = Private->PciIo->Pci.Read (
                        Private->PciIo,
                        EfiPciIoWidthUint32,
                        0,
                        64,
                        &x[0]
                        );
DebugPrint(0, "Read configuration %d\n", Status);
memcpy(&Pci, &x[0], sizeof(Pci));
  for(i = 0; i < sizeof(x); ++i) {
     DebugPrint(0, "%02x ", x[i]);
     if(((i + 1) % 16) == 0)
        DebugPrint(0, "\n");
  }
      DebugPrint(0, "\n");
  if (EFI_ERROR (Status)) {
    goto ClosePciIo;
  }

  //
  // Determine card variant.
  //
  Card = GmaVideoDetect(Pci.Hdr.ClassCode[1], Pci.Hdr.VendorId, Pci.Hdr.DeviceId);
DebugPrint(0, "Video Detect %p\n", Card);
  if (Card == NULL) {
    Status = EFI_DEVICE_ERROR;
    goto ClosePciIo;
  }
  Private->Variant = Card->Variant;
  Private->FrameBufferVramBarIndex = PCI_BAR_IDX0;

  //
  // Save original PCI attributes
  //
  Status = Private->PciIo->Attributes (
                    Private->PciIo,
                    EfiPciIoAttributeOperationGet,
                    0,
                    &Private->OriginalPciAttributes
                    );

DebugPrint(0, "Read Attributes %d\n", Status);
  if (EFI_ERROR (Status)) {
    goto ClosePciIo;
  }

  //
  // Set new PCI attributes
  //
#define  PCI_COMMAND_MASTER 0x4
#define  PCI_COMMAND_MEMORY 0x2
#define  PCI_COMMAND_IO     0x1


  Status = Private->PciIo->Attributes (
                            Private->PciIo,
                            EfiPciIoAttributeOperationEnable,
                            EFI_PCI_DEVICE_ENABLE |PCI_COMMAND_MASTER |PCI_COMMAND_MEMORY |PCI_COMMAND_IO ,
                            NULL
                            );
DebugPrint(0, "Set Attributes %d\n", Status);
  if (EFI_ERROR (Status)) {
    goto ClosePciIo;
  }

  //
  // Get ParentDevicePath
  //
  Status = gBS->HandleProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &ParentDevicePath
                  );
  if (EFI_ERROR (Status)) {
DebugPrint(0, "HandleProtocol %s\n", Status);
    goto RestoreAttributes;
  }
Private->FrameBufferVramBarIndex = PCI_BAR_IDX1;

  //
  // Set Gop Device Path
  //
  ZeroMem (&AcpiDeviceNode, sizeof (ACPI_ADR_DEVICE_PATH));
  AcpiDeviceNode.Header.Type = ACPI_DEVICE_PATH;
  AcpiDeviceNode.Header.SubType = ACPI_ADR_DP;
  AcpiDeviceNode.ADR = ACPI_DISPLAY_ADR (1, 0, 0, 1, 0, ACPI_ADR_DISPLAY_TYPE_VGA, 0, 0);
  SetDevicePathNodeLength (&AcpiDeviceNode.Header, sizeof (ACPI_ADR_DEVICE_PATH));

  Private->GopDevicePath = AppendDevicePathNode (
                                      ParentDevicePath,
                                      (EFI_DEVICE_PATH_PROTOCOL *) &AcpiDeviceNode
                                      );
  if (Private->GopDevicePath == NULL) {
DebugPrint(0, "&AcpiDeviceNode, %s\n", Status);
    Status = EFI_OUT_OF_RESOURCES;
    goto RestoreAttributes;
  }

  //
  // Create new child handle and install the device path protocol on it.
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Private->Handle,
                  &gEfiDevicePathProtocolGuid,
                  Private->GopDevicePath,
                  NULL
                  );
DebugPrint(0, "Protocol installed %d\n", Status);
  if (EFI_ERROR (Status)) {
    goto FreeGopDevicePath;
  }

  Status = GmaVideoHdGfxModeSetup(Private);
  if (EFI_ERROR (Status)) {
DebugPrint(0, "Intel HD ERROR %d\n", Status);
    goto UninstallGopDevicePath;
  }

  //
  // Start the GOP software stack.
  //
  Status = GmaVideoGraphicsOutputConstructor (Private);
  if (EFI_ERROR (Status)) {
DebugPrint(0, "GmaVideoGraphicsOutputConstructor %d\n", Status);
    goto FreeModeData;
  }

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Private->Handle,
                  &gEfiGraphicsOutputProtocolGuid,
                  &Private->GraphicsOutput,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
DebugPrint(0, "GmaVideoGraphicsOutputConstructor failed %d\n", Status);
    goto DestructGmaVideoGraphics;
  }

DebugPrint(0, "GmaVideoGraphicsOutputConstructor passed %d\n", Status);
  //
  // Reference parent handle from child handle.
  //
  Status = gBS->OpenProtocol (
                Controller,
                &gEfiPciIoProtocolGuid,
                (VOID **) &ChildPciIo,
                This->DriverBindingHandle,
                Private->Handle,
                EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                );
  if (EFI_ERROR (Status)) {
    goto UninstallGop;
  }

  gBS->RestoreTPL (OldTpl);
  return EFI_SUCCESS;

UninstallGop:
  gBS->UninstallProtocolInterface (Private->Handle,
         &gEfiGraphicsOutputProtocolGuid, &Private->GraphicsOutput);

DestructGmaVideoGraphics:
  GmaVideoGraphicsOutputDestructor (Private);

FreeModeData:
  FreePool (Private->ModeData);
  if (Private->VmwareSvgaModeInfo != NULL) {
    FreePool (Private->VmwareSvgaModeInfo);
  }

UninstallGopDevicePath:
  gBS->UninstallProtocolInterface (Private->Handle,
         &gEfiDevicePathProtocolGuid, Private->GopDevicePath);

FreeGopDevicePath:
  FreePool (Private->GopDevicePath);

RestoreAttributes:
  DebugPrint(0, "RestoreAttributes AFTER FAIL \n");
  Private->PciIo->Attributes (Private->PciIo, EfiPciIoAttributeOperationSet,
                    Private->OriginalPciAttributes, NULL);

ClosePciIo:
  gBS->CloseProtocol (Controller, &gEfiPciIoProtocolGuid,
         This->DriverBindingHandle, Controller);

FreePrivate:
  FreePool (Private);

RestoreTpl:
  gBS->RestoreTPL (OldTpl);

  return Status;
}

/**
  Stop this device

  @param  This                   The USB bus driver binding protocol.
  @param  Controller             The controller to release.
  @param  NumberOfChildren       The number of children of this device that
                                 opened the controller BY_CHILD.
  @param  ChildHandleBuffer      The array of child handle.

  @retval EFI_SUCCESS            The controller or children are stopped.
  @retval EFI_DEVICE_ERROR       Failed to stop the driver.

**/
EFI_STATUS
EFIAPI
GmaVideoControllerDriverStop (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN UINTN                          NumberOfChildren,
  IN EFI_HANDLE                     *ChildHandleBuffer
  )
{
  EFI_GRAPHICS_OUTPUT_PROTOCOL    *GraphicsOutput;

  EFI_STATUS                      Status;
  GMA_VIDEO_PRIVATE_DATA  *Private;

  if (NumberOfChildren == 0) {
    //
    // Close the PCI I/O Protocol
    //
    gBS->CloseProtocol (
          Controller,
          &gEfiPciIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );
    return EFI_SUCCESS;
  }

  //
  // free all resources for whose access we need the child handle, because the
  // child handle is going away
  //
  ASSERT (NumberOfChildren == 1);
  Status = gBS->OpenProtocol (
                  ChildHandleBuffer[0],
                  &gEfiGraphicsOutputProtocolGuid,
                  (VOID **) &GraphicsOutput,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Get our private context information
  //
  Private = GMA_VIDEO_PRIVATE_DATA_FROM_GRAPHICS_OUTPUT_THIS (GraphicsOutput);
  ASSERT (Private->Handle == ChildHandleBuffer[0]);

  GmaVideoGraphicsOutputDestructor (Private);
  //
  // Remove the GOP protocol interface from the system
  //
  Status = gBS->UninstallMultipleProtocolInterfaces (
                  Private->Handle,
                  &gEfiGraphicsOutputProtocolGuid,
                  &Private->GraphicsOutput,
                  NULL
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Restore original PCI attributes
  //
  Private->PciIo->Attributes (
                  Private->PciIo,
                  EfiPciIoAttributeOperationSet,
                  Private->OriginalPciAttributes,
                  NULL
                  );

  gBS->CloseProtocol (
        Controller,
        &gEfiPciIoProtocolGuid,
        This->DriverBindingHandle,
        Private->Handle
        );

  FreePool (Private->ModeData);
  if (Private->VmwareSvgaModeInfo != NULL) {
    FreePool (Private->VmwareSvgaModeInfo);
  }
  gBS->UninstallProtocolInterface (Private->Handle,
         &gEfiDevicePathProtocolGuid, Private->GopDevicePath);
  FreePool (Private->GopDevicePath);

  //
  // Free our instance data
  //
  gBS->FreePool (Private);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
InitializeGmaVideo (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS              Status;

  Status = EfiLibInstallDriverBindingComponentName2 (
             ImageHandle,
             SystemTable,
             &gGmaVideoDriverBinding,
             ImageHandle,
             &gGmaVideoComponentName,
             &gGmaVideoComponentName2
             );
  ASSERT_EFI_ERROR (Status);

  //
  // Install EFI Driver Supported EFI Version Protocol required for
  // EFI drivers that are on PCI and other plug in cards.
  //
  gGmaVideoDriverSupportedEfiVersion.FirmwareVersion = PcdGet32 (PcdDriverSupportedEfiVersion);
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &ImageHandle,
                  &gEfiDriverSupportedEfiVersionProtocolGuid,
                  &gGmaVideoDriverSupportedEfiVersion,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  return Status;
}
