with Interfaces.C;

with HW;
use HW;

package GMA
is

   procedure test_debugprint;
   pragma Export (C, test_debugprint, "gma_test_debugprint");

   procedure gfxinit (lightup_ok : out Interfaces.C.int);
   pragma Export (C, gfxinit, "gma_gfxinit");

   ----------------------------------------------------------------------------

   type lb_framebuffer is record
      physical_address     : word64;
      x_resolution         : word64;
      y_resolution         : word64;
      bpp                  : word64;
   end record;

   function fill_lb_framebuffer
     (framebuffer : in out lb_framebuffer)
      return Interfaces.C.int;
   pragma Export (C, fill_lb_framebuffer, "fill_lb_framebuffer");

end GMA;
