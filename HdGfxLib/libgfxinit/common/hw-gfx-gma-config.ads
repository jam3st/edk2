--
-- Copyright (C) 2015-2018 secunet Security Networks AG
--
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 2 of the License, or
-- (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--

private package HW.GFX.GMA.Config
with
   Initializes => (Valid_Port_GPU, Raw_Clock)
is

   CPU : constant CPU_Type := Haswell;

   CPU_Var : constant CPU_Variant := Normal;

   Internal_Display : constant Internal_Type := DP;

   Analog_I2C_Port : constant PCH_Port := PCH_DAC;

   EDP_Low_Voltage_Swing : constant Boolean := False;

   DDI_HDMI_Buffer_Translation : constant Integer := -1;

   Default_MMIO_Base : constant := 0 ;

   LVDS_Dual_Threshold : constant := 95_000_000;

   ----------------------------------------------------------------------------

   Default_MMIO_Base_Set   : constant Boolean := Default_MMIO_Base /= 0;

   Has_Internal_Display    : constant Boolean := Internal_Display /= None;
   Internal_Is_EDP         : constant Boolean := Internal_Display = DP;
   Have_DVI_I              : constant Boolean := Analog_I2C_Port /= PCH_DAC;
   Has_Presence_Straps     : constant Boolean := CPU /= Broxton;

   ----- CPU pipe: --------
   Disable_Trickle_Feed    : constant Boolean := not
                                                (CPU in Haswell .. Broadwell);
   Pipe_Enabled_Workaround : constant Boolean := CPU = Broadwell;
   Has_EDP_Transcoder      : constant Boolean := CPU >= Haswell;
   Use_PDW_For_EDP_Scaling : constant Boolean := CPU = Haswell;
   Has_Pipe_DDI_Func       : constant Boolean := CPU >= Haswell;
   Has_Trans_Clk_Sel       : constant Boolean := CPU >= Haswell;
   Has_Pipe_MSA_Misc       : constant Boolean := CPU >= Haswell;
   Has_Pipeconf_Misc       : constant Boolean := CPU >= Broadwell;
   Has_Pipeconf_BPC        : constant Boolean := CPU /= Haswell;
   Has_Plane_Control       : constant Boolean := CPU >= Broxton;
   Has_DSP_Linoff          : constant Boolean := CPU <= Ivybridge;
   Has_PF_Pipe_Select      : constant Boolean := CPU in Ivybridge .. Haswell;
   Has_Cursor_FBC_Control  : constant Boolean := CPU >= Ivybridge;
   VGA_Plane_Workaround    : constant Boolean := CPU = Ivybridge;
   Has_GMCH_DP_Transcoder  : constant Boolean := CPU = G45;
   Has_GMCH_VGACNTRL       : constant Boolean := CPU = G45;
   Has_GMCH_PFIT_CONTROL   : constant Boolean := CPU = G45;

   ----- Panel power: -----
   Has_PP_Write_Protection : constant Boolean := CPU <= Ivybridge;
   Has_PP_Port_Select      : constant Boolean := CPU <= Ivybridge;
   Use_PP_VDD_Override     : constant Boolean := CPU <= Ivybridge;
   Has_PCH_Panel_Power     : constant Boolean := CPU >= Ironlake;

   ----- PCH/FDI: ---------
   Has_PCH                 : constant Boolean := CPU /= Broxton and CPU /= G45;
   Has_PCH_DAC             : constant Boolean := CPU in Ironlake .. Ivybridge or
                                                 (CPU in Broadwell .. Haswell
                                                  and CPU_Var = Normal);

   Has_PCH_Aux_Channels    : constant Boolean := CPU in Ironlake .. Broadwell;

   VGA_Has_Sync_Disable    : constant Boolean := CPU <= Ivybridge;

   Has_Trans_Timing_Ovrrde : constant Boolean := CPU >= Sandybridge;

   Has_DPLL_SEL            : constant Boolean := CPU in Ironlake .. Ivybridge;
   Has_FDI_BPC             : constant Boolean := CPU in Ironlake .. Ivybridge;
   Has_FDI_Composite_Sel   : constant Boolean := CPU = Ivybridge;
   Has_Trans_DP_Ctl        : constant Boolean := CPU in
                                                 Sandybridge .. Ivybridge;
   Has_FDI_C               : constant Boolean := CPU = Ivybridge;

   Has_FDI_RX_Power_Down   : constant Boolean := CPU in Haswell .. Broadwell;

   Has_GMCH_RawClk         : constant Boolean := CPU = G45;

   ----- DDI: -------------
   End_EDP_Training_Late   : constant Boolean := CPU in Haswell .. Broadwell;
   Has_Per_DDI_Clock_Sel   : constant Boolean := CPU in Haswell .. Broadwell;
   Has_HOTPLUG_CTL         : constant Boolean := CPU in Haswell .. Broadwell;
   Has_SHOTPLUG_CTL_A      : constant Boolean := (CPU in Haswell .. Broadwell
                                                  and CPU_Var = ULT) or
                                                 CPU >= Skylake;

   Has_DDI_PHYs            : constant Boolean := CPU = Broxton;

   Has_DDI_D               : constant Boolean := CPU >= Haswell and
                                                 CPU_Var = Normal and
                                                 not Has_DDI_PHYs;
   Has_DDI_E               : constant Boolean := -- might be disabled by x4 eDP
                                                 Has_DDI_D;

   Has_DDI_Buffer_Trans    : constant Boolean := CPU >= Haswell and
                                                 CPU /= Broxton;
   Has_Low_Voltage_Swing   : constant Boolean := CPU >= Broxton;
   Has_Iboost_Config       : constant Boolean := CPU >= Skylake;

   Need_DP_Aux_Mutex       : constant Boolean := False; -- Skylake & (PSR | GTC)

   ----- GMBUS: -----------
   Ungate_GMBUS_Unit_Level : constant Boolean := CPU >= Skylake;
   GMBUS_Alternative_Pins  : constant Boolean := CPU = Broxton;
   Has_PCH_GMBUS           : constant Boolean := CPU >= Ironlake;

   ----- Power: -----------
   Has_IPS                 : constant Boolean := (CPU = Haswell and
                                                  CPU_Var = ULT) or
                                                 CPU = Broadwell;
   Has_IPS_CTL_Mailbox     : constant Boolean := CPU = Broadwell;

   Has_Per_Pipe_SRD        : constant Boolean := CPU >= Broadwell;

   ----- GTT: -------------
   Fold_39Bit_GTT_PTE      : constant Boolean := CPU <= Haswell;

   ----------------------------------------------------------------------------

   Max_Pipe : constant Pipe_Index :=
     (if CPU <= Sandybridge
      then Secondary
      else Tertiary);

   type Supported_Pipe_Array is array (Pipe_Index) of Boolean;
   Supported_Pipe : constant Supported_Pipe_Array :=
     (Primary     => Primary     <= Max_Pipe,
      Secondary   => Secondary   <= Max_Pipe,
      Tertiary    => Tertiary    <= Max_Pipe);

   type Valid_Per_Port is array (Port_Type) of Boolean;
   type Valid_Per_GPU is array (CPU_Type) of Valid_Per_Port;
   Valid_Port_GPU : Valid_Per_GPU :=
     (G45         =>
        (Disabled => False,
         Internal => Config.Internal_Display = LVDS,
         HDMI3    => False,
         others   => True),
      Ironlake    =>
        (Disabled => False,
         Internal => Config.Internal_Display = LVDS,
         others   => True),
      Sandybridge =>
        (Disabled => False,
         Internal => Config.Internal_Display = LVDS,
         others   => True),
      Ivybridge   =>
        (Disabled => False,
         Internal => Config.Internal_Display /= None,
         others   => True),
      Haswell     =>
        (Disabled => False,
         Internal => Config.Internal_Display = DP,
         HDMI3    => CPU_Var = Normal,
         DP3      => CPU_Var = Normal,
         Analog   => CPU_Var = Normal,
         others   => True),
      Broadwell   =>
        (Disabled => False,
         Internal => Config.Internal_Display = DP,
         HDMI3    => CPU_Var = Normal,
         DP3      => CPU_Var = Normal,
         Analog   => CPU_Var = Normal,
         others   => True),
      Broxton     =>
        (Internal => Config.Internal_Display = DP,
         DP1      => True,
         DP2      => True,
         HDMI1    => True,
         HDMI2    => True,
         others   => False),
      Skylake     =>
        (Disabled => False,
         Internal => Config.Internal_Display = DP,
         Analog   => False,
         others   => True))
   with
      Part_Of => GMA.Config_State;
   Valid_Port : Valid_Per_Port renames Valid_Port_GPU (CPU);

   Last_Digital_Port : constant Digital_Port :=
     (if Has_DDI_E then DIGI_E else DIGI_C);

   ----------------------------------------------------------------------------

   type FDI_Per_Port is array (Port_Type) of Boolean;
   Is_FDI_Port : constant FDI_Per_Port :=
     (case CPU is
         when Ironlake .. Ivybridge => FDI_Per_Port'
           (Internal => Internal_Display = LVDS,
            others   => True),
         when Haswell .. Broadwell => FDI_Per_Port'
           (Analog   => Has_PCH_DAC,
            others   => False),
         when others => FDI_Per_Port'
           (others   => False));

   type FDI_Lanes_Per_Port is array (GPU_Port) of DP_Lane_Count;
   FDI_Lane_Count : constant FDI_Lanes_Per_Port :=
     (DIGI_D => DP_Lane_Count_2,
      others =>
        (if CPU in Ironlake .. Ivybridge then
            DP_Lane_Count_4
         else
            DP_Lane_Count_2));

   FDI_Training : constant FDI_Training_Type :=
     (case CPU is
         when Ironlake     => Simple_Training,
         when Sandybridge  => Full_Training,
         when others       => Auto_Training);

   ----------------------------------------------------------------------------

   Default_DDI_HDMI_Buffer_Translation : constant DDI_HDMI_Buf_Trans_Range :=
     (case CPU is
         when Haswell   => 6,
         when Broadwell => 7,
         when Broxton   => 8,
         when Skylake   => 8,
         when others    => 0);

   ----------------------------------------------------------------------------

   Default_CDClk_Freq : constant Frequency_Type :=
     (case CPU is
         when G45          => 320_000_000, -- unused
         when Ironlake     |
              Haswell      |
              Broadwell    => 450_000_000,
         when Sandybridge  |
              Ivybridge    => 400_000_000,
         when Broxton      => 288_000_000,
         when Skylake      => 337_500_000);

   Default_RawClk_Freq : constant Frequency_Type :=
     (case CPU is
         when G45          => 100_000_000, -- unused, depends on FSB
         when Ironlake     |
              Sandybridge  |
              Ivybridge    => 125_000_000,
         when Haswell      |
              Broadwell    => (if CPU_Var = Normal then
                                 125_000_000
                               else
                                 24_000_000),
         when Broxton      => Frequency_Type'First,   -- none needed
         when Skylake      => 24_000_000);

    Raw_Clock : Frequency_Type := Default_RawClk_Freq
    with Part_Of => GMA.Config_State;

   ----------------------------------------------------------------------------

   -- Maximum source width with enabled scaler. This only accounts
   -- for simple 1:1 pipe:scaler mappings.

   type Width_Per_Pipe is array (Pipe_Index) of Pos16;

   Maximum_Scalable_Width : constant Width_Per_Pipe :=
     (case CPU is
         when G45 => -- TODO: Is this true?
           (Primary     => 4096,
            Secondary   => 2048,
            Tertiary    => Pos16'First),
         when Ironlake..Haswell =>
           (Primary     => 4096,
            Secondary   => 2048,
            Tertiary    => 2048),
         when Broadwell..Skylake =>
           (Primary     => 4096,
            Secondary   => 4096,
            Tertiary    => 4096));

   -- Maximum X position of hardware cursors
   Maximum_Cursor_X : constant := (case CPU is
                                    when G45 .. Ivybridge      => 4095,
                                    when Haswell .. Skylake    => 8191);

   Maximum_Cursor_Y : constant := 4095;

   ----------------------------------------------------------------------------

   -- FIXME: Unknown for Broxton, Linux' i915 contains a fixme too :-D
   HDMI_Max_Clock_24bpp : constant Frequency_Type :=
     (if CPU >= Haswell then 300_000_000 else 225_000_000);

   ----------------------------------------------------------------------------

   GTT_Offset  : constant := (case CPU is
                                 when G45 .. Haswell   => 16#0020_0000#,
                                 when Broadwell .. Skylake  => 16#0080_0000#);

   GTT_Size    : constant := (case CPU is
                                 when G45 .. Haswell   => 16#0020_0000#,
                                 -- Limit Broadwell to 4MiB to have a stable
                                 -- interface (i.e. same number of entries):
                                 when Broadwell .. Skylake  => 16#0040_0000#);

   GTT_PTE_Size   : constant := (case CPU is
                                    when G45 .. Haswell       => 4,
                                    when Broadwell .. Skylake => 8);

   Fence_Base : constant := (case CPU is
                              when G45 .. Ironlake          => 16#0000_3000#,
                              when Sandybridge .. Skylake   => 16#0010_0000#);

   Fence_Count : constant := (case CPU is
                                 when G45 .. Sandybridge       => 16,
                                 when Ivybridge .. Skylake     => 32);

   ----------------------------------------------------------------------------

   use type HW.Word16;

   function Is_Broadwell_H (Device_Id : Word16) return Boolean is
     (Device_Id = 16#1612# or Device_Id = 16#1622# or Device_Id = 16#162a#);

   function Is_Skylake_U (Device_Id : Word16) return Boolean is
     (Device_Id = 16#1906# or Device_Id = 16#1916# or Device_Id = 16#1923# or
      Device_Id = 16#1926# or Device_Id = 16#1927#);

   -- Rather catch too much here than too little,
   -- it's only used to distinguish generations.
   function Is_GPU (Device_Id : Word16; CPU : CPU_Type; CPU_Var : CPU_Variant)
      return Boolean is
     (case CPU is
         when G45          => (Device_Id and 16#ff02#) = 16#2e02# or
                              (Device_Id and 16#fffe#) = 16#2a42#,
         when Ironlake     => (Device_Id and 16#fff3#) = 16#0042#,
         when Sandybridge  => (Device_Id and 16#ffc2#) = 16#0102#,
         when Ivybridge    => (Device_Id and 16#ffc3#) = 16#0142#,
         when Haswell =>
           (case CPU_Var is
               when Normal => (Device_Id and 16#ffc3#) = 16#0402# or
                              (Device_Id and 16#ffc3#) = 16#0d02#,
               when ULT    => (Device_Id and 16#ffc3#) = 16#0a02#),
         when Broadwell    => ((Device_Id and 16#ffc3#) = 16#1602# or
                               (Device_Id and 16#ffcf#) = 16#160b# or
                               (Device_Id and 16#ffcf#) = 16#160d#) and
                             (case CPU_Var is
                                 when Normal =>     Is_Broadwell_H (Device_Id),
                                 when ULT    => not Is_Broadwell_H (Device_Id)),
         when Broxton      => (Device_Id and 16#fffe#) = 16#5a84#,
         when Skylake      => ((Device_Id and 16#ffc3#) = 16#1902# or
                               (Device_Id and 16#ffcf#) = 16#190b# or
                               (Device_Id and 16#ffcf#) = 16#190d# or
                               (Device_Id and 16#fff9#) = 16#1921#) and
                             (case CPU_Var is
                                 when Normal => not Is_Skylake_U (Device_Id),
                                 when ULT    =>     Is_Skylake_U (Device_Id)));

   function Compatible_GPU (Device_Id : Word16) return Boolean is
     (Is_GPU (Device_Id, CPU, CPU_Var));

end HW.GFX.GMA.Config;
