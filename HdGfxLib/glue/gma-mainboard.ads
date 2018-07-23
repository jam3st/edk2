with HW.GFX.GMA;
with HW.GFX.GMA.Display_Probing;

use HW.GFX.GMA;
use HW.GFX.GMA.Display_Probing;

private package GMA.Mainboard is

   ports : constant Port_List :=
     ( -- mainboard DVI port
      HDMI3, -- mainboard HDMI port
       -- mainboard HDMI port
      others => Disabled);

end GMA.Mainboard;
