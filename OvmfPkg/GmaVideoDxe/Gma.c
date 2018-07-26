#include "haswell.h"
#include "i915_reg.h"
#include "Gma.h"
#include <Library/PciLib.h>
#include <Protocol/Cpu.h>


#define LP_GPE0_EN_4	0x9c
#define LP_GPE0_STS_4	0x8c	/* Standard GPE */
#define TCO1_STS	0x64
#define   DMISCI_STS	(1 << 9)
#define   TCOSCI_STS	(1 << 6)
#define   TCOSCI_EN	(1 << 6)


// Used by the HD Graphics Ada stuff
struct mono_time {
    long microseconds;
};

static struct monotonic_counter {
    int initialized;
    struct mono_time time;
    UINT32 last_value;
} mono_counter;

static inline void mono_time_add_usecs(struct mono_time *mt, long us)
{
    mt->microseconds += us;
}


void timer_monotonic_get(struct mono_time *mt)
{
    MicroSecondDelay(2);
    mono_time_add_usecs(&mono_counter.time, 2);
    *mt = mono_counter.time;
}



static inline __attribute__((always_inline)) u32 read32( const volatile u32* addr)
{
    u32 val;
    asm("movl (%eax), %rax" : "=a" (val): "a"(addr): "memory");
    return *addr;
}

static inline __attribute__((always_inline)) void write32(volatile u32 *addr, u32 val)
{
    asm("movl %edx, (%rax)" : "=d" (val): "a"(addr): "memory");
}

static inline u32* res2mmio(u64 base, unsigned long offset)
{
    return (u32 *)(u64)((base + offset));
}

static inline u32 inl(u16 port)
{
    u32 value;
    __asm__ __volatile__  ( "inl %w1, %k0" : "=a" (value) : "d" (port) );

    return value;
}

static inline u16 inw(u16 port)
{
    u16 value;
    __asm__ __volatile__ ("inw %w1, %w0" : "=a"(value) : "d" (port));
    return value;

}

static inline void outw(u16 port, u16 value)
{
    __asm__ __volatile__ ("outw %w0, %w1" : : "a" (value), "d" (port));
}

static inline void outl( u16 port, u32 value)
{
    __asm__ __volatile__ ("outl %k0, %w1" : : "a" (value), "d" (port));
}


struct gt_reg {
    u32 reg;
    u32 andmask;
    u32 ormask;
};

static const struct gt_reg haswell_gt_setup[] = { 
    /* Enable Counters */
    { 0x0a248, 0x00000000, 0x00000016 },
    { 0x0a000, 0x00000000, 0x00070020 },
    { 0x0a180, 0xff3fffff, 0x15000000 },
    /* Enable DOP Clock Gating */
    { 0x09424, 0x00000000, 0x000003fd },
    /* Enable Unit Level Clock Gating */
    { 0x09400, 0x00000000, 0x00000080 },
    { 0x09404, 0x00000000, 0x40401000 },
    { 0x09408, 0x00000000, 0x00000000 },
    { 0x0940c, 0x00000000, 0x02000001 },
    { 0x0a008, 0x00000000, 0x08000000 },
    /* Wake Rate Limits */
    { 0x0a090, 0xffffffff, 0x00000000 },
    { 0x0a098, 0xffffffff, 0x03e80000 },
    { 0x0a09c, 0xffffffff, 0x00280000 },
    { 0x0a0a8, 0xffffffff, 0x0001e848 },
    { 0x0a0ac, 0xffffffff, 0x00000019 },
    /* Render/Video/Blitter Idle Max Count */
    { 0x02054, 0x00000000, 0x0000000a },
    { 0x12054, 0x00000000, 0x0000000a },
    { 0x22054, 0x00000000, 0x0000000a },
    /* RC Sleep / RCx Thresholds */
    { 0x0a0b0, 0xffffffff, 0x00000000 },
    { 0x0a0b4, 0xffffffff, 0x000003e8 },
    { 0x0a0b8, 0xffffffff, 0x0000c350 },
    /* RP Settings */
    { 0x0a010, 0xffffffff, 0x000f4240 },
    { 0x0a014, 0xffffffff, 0x12060000 },
    { 0x0a02c, 0xffffffff, 0x0000e808 },
    { 0x0a030, 0xffffffff, 0x0003bd08 },
    { 0x0a068, 0xffffffff, 0x000101d0 },
    { 0x0a06c, 0xffffffff, 0x00055730 },
    { 0x0a070, 0xffffffff, 0x0000000a },
    /* RP Control */
    { 0x0a024, 0x00000000, 0x00000b92 },
    /* HW RC6 Control */
    { 0x0a090, 0x00000000, 0x88040000 },
    /* Video Frequency Request */
    { 0x0a00c, 0x00000000, 0x08000000 },
    { 0 },
};

static const struct gt_reg haswell_gt_lock[] = { 
    { 0x0a248, 0xffffffff, 0x80000000 },
    { 0x0a004, 0xffffffff, 0x00000010 },
    { 0x0a080, 0xffffffff, 0x00000004 },
    { 0x0a180, 0xffffffff, 0x80000000 },
    { 0 },
};


static u64 gtt_res_base = 0ull;

u32 gtt_read(u32 reg, int print)
{
    u32 val;
    void* xreg = res2mmio(gtt_res_base, reg); 
    val = read32(xreg);
    if(print)
        DebugPrint(0,  "GTT R %p %x\n", xreg, val);
    return val;

}

void gtt_write(u32 reg, u32 data)
{
    void* xreg = res2mmio(gtt_res_base, reg); 
    write32(xreg, data);
    DebugPrint(0,  "GTT W %p %x\n", xreg, data);
}

static inline void gtt_rmw(u32 reg, u32 andmask, u32 ormask)
{
    u32 val = gtt_read(reg, 1);
    val &= andmask;
    val |= ormask;
    gtt_write(reg, val);
}

static inline void gtt_write_regs(const struct gt_reg *gt)
{
    for (; gt && gt->reg; gt++) {
        if (gt->andmask)
            gtt_rmw(gt->reg, gt->andmask, gt->ormask);
        else
            gtt_write(gt->reg, gt->ormask);
    }
}

#define GTT_RETRY 10000
int gtt_poll(u32 reg, u32 mask, u32 value)
{
    unsigned try = GTT_RETRY;
    u32 data;

    while (try--) {
        data = gtt_read(reg, 0);
        if ((data & mask) == value) {
            DebugPrint(0, "GT poll ok %x %x %x\n", reg, mask, value);
            return 1;
            }
        MicroSecondDelay(100);
    }

    DebugPrint(0, "GT init timeout on %x %x %x got %x\n", reg, mask, value, data);
    return 0;
}

static void power_well_enable(void)
{
    gtt_write(HSW_PWR_WELL_CTL1, 0);
    MicroSecondDelay(50000);
    gtt_write(HSW_PWR_WELL_CTL1, HSW_PWR_WELL_ENABLE);
    gtt_read(HSW_PWR_WELL_CTL1,1);
    gtt_poll(HSW_PWR_WELL_CTL1, HSW_PWR_WELL_STATE, HSW_PWR_WELL_STATE);
    gtt_read(HSW_PWR_WELL_CTL1,1);
}


static void gma_pm_init_pre_vbios()
{
	DebugPrint(0, "GT Power Management Init %llx\n", gtt_res_base);

	power_well_enable();

	/*
	 * Enable RC6
	 */

	/* Enable Force Wake */
	gtt_write(0x0a180, 1 << 5);
	gtt_write(0x0a188, 0x00010001);
	gtt_poll(FORCEWAKE_ACK_HSW, 1u << 0, 1u << 0);

	/* GT Settings */
	gtt_write_regs(haswell_gt_setup);

	/* Wait for Mailbox Ready */
	gtt_poll(0x138124, (1UL << 31), (0UL << 31));
	/* Mailbox Data - RC6 VIDS */
	gtt_write(0x138128, 0x00000000);
	/* Mailbox Command */
	gtt_write(0x138124, 0x80000004);
	/* Wait for Mailbox Ready */
	gtt_poll(0x138124, (1UL << 31), (0UL << 31));

	/* Enable PM Interrupts */
	gtt_write(GEN6_PMIER, GEN6_PM_MBOX_EVENT | GEN6_PM_THERMAL_EVENT |
		  GEN6_PM_RP_DOWN_TIMEOUT | GEN6_PM_RP_UP_THRESHOLD |
		  GEN6_PM_RP_DOWN_THRESHOLD | GEN6_PM_RP_UP_EI_EXPIRED |
		  GEN6_PM_RP_DOWN_EI_EXPIRED);

	/* Enable RC6 in idle */
	gtt_write(0x0a094, 0x00040000);

	/* PM Lock Settings */
	gtt_write_regs(haswell_gt_lock);
}



struct lb_framebuffer* getFb() {
   static struct lb_framebuffer fb;
   return &fb;
}

void Write8(UINT8     Data) {
     __asm__ __volatile__ ("outb %b0,%w1" : : "a" (Data), "d" ((UINT16)0x402));
}
#define PCI_COMMAND  0x04
#define  PCI_COMMAND_MASTER 0x4
#define  PCI_COMMAND_MEMORY 0x2 
#define  PCI_COMMAND_IO     0x1

static void mset(u8* dst, u8 pat, u64 size) {
    while(size-- != 0) {
        dst[size] = pat;
    }
}

void gma_func0_init(GMA_VIDEO_PRIVATE_DATA* dev) {
  EFI_STATUS s;
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *BarDesc;
  UINT16 deviceId;
  EFI_CPU_ARCH_PROTOCOL     *cpu;

  static int once = 0;
  static int lightup_ok = 0;
  if(once++ != 0) {
    return;
  }
  s = dev->PciIo->GetBarAttributes (dev->PciIo, PCI_BAR_IDX0, NULL, (VOID**) &BarDesc);
  /* Get a Memory address for mapping the Grant Table. */
  DebugPrint(0, "BAR0: BAR at %LX len %x %x\n", BarDesc->AddrRangeMin, BarDesc->AddrLen, BarDesc->AddrTranslationOffset);
  gtt_res_base = BarDesc->AddrRangeMin;
  s = dev->PciIo->GetBarAttributes (dev->PciIo, PCI_BAR_IDX1, NULL, (VOID**) &BarDesc);
  /* Get a Memory address for mapping the Grant Table. */
  DebugPrint(0, "BAR2: BAR at %LX len %x %x\n", BarDesc->AddrRangeMin, BarDesc->AddrLen, BarDesc->AddrTranslationOffset);
  DebugPrint(0, "BAR2: set WC %s\n", s);

  DebugPrint(0, "GTT BASE is %d %16llx\n", s, gtt_res_base);
  dev->PciIo->Pci.Read (dev->PciIo, EfiPciIoWidthUint16, PCI_DEVICE_ID_OFFSET, 1, &deviceId);
  DebugPrint(0, "GTT BASE devid %4x\n", deviceId);
  
  s = gBS->LocateProtocol (&gEfiCpuArchProtocolGuid, NULL, (VOID **) &cpu);
  DebugPrint(0, "CPU Protocol %d\n", s);
  s = cpu->SetMemoryAttributes(cpu, BarDesc->AddrRangeMin, BarDesc->AddrLen, EFI_MEMORY_WC | EFI_MEMORY_WB );
  DebugPrint(0, "Attributes is %d\n", s);
  hdgfx_adainit();
  gma_pm_init_pre_vbios();
  gma_gfxinit(&lightup_ok);
  DebugPrint(0, "Lightup ok is %d\n", lightup_ok);
      /* Post panel init */

  if(fill_lb_framebuffer(getFb()) == 0) {
      struct lb_framebuffer* fb = getFb();
      u8* fbAddr = (u8 *)(0x800000000);
      u64 fbSize = fb->x_resolution * fb->y_resolution * 4;
      DebugPrint(0, "fb is at %p of %d x %d size 0x%llx\n", fbAddr, fb->x_resolution, fb->y_resolution, fb->bpp);
      DebugPrint(0, "fbsize %lld\n", fbSize);
      mset(fbAddr, 0x55, fbSize);
      DebugPrint(0, "DONE %lld\n", fbSize);
    }
}

