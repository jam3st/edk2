#include "Qemu.h"
#include "haswell.h"
#include "i915_reg.h"
#include <Library/PciLib.h>

#define LP_GPE0_EN_4	0x9c
#define LP_GPE0_STS_4	0x8c	/* Standard GPE */
#define TCO1_STS	0x64
#define   DMISCI_STS	(1 << 9)
#define   TCOSCI_STS	(1 << 6)
#define   TCOSCI_EN	(1 << 6)



static inline __attribute__((always_inline)) u32 read32( const volatile void *addr)
{
    return *((volatile u32 *)(addr));
}

static inline __attribute__((always_inline)) void write32(volatile void *addr,
    u32 value)
{
    *((volatile u32 *)(addr)) = value;
}

static inline void *res2mmio(u64 base, unsigned long offset)
{
    return (void *)((base + (u64)offset));
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

u16 get_pmbase(void)
{
    static u16 pmbase = 0x8c00;
    return pmbase;
}

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

#define GTT_PTE_BASE (2 << 20)

static u64 gtt_res_base = 0ull;

u32 gtt_read(u32 reg)
{
    u32 val;
    val = read32(res2mmio(gtt_res_base, reg));
    return val;

}

void gtt_write(u32 reg, u32 data)
{
    write32(res2mmio(gtt_res_base, reg), data);
}

static inline void gtt_rmw(u32 reg, u32 andmask, u32 ormask)
{
    u32 val = gtt_read(reg);
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

/* Enable a standard GPE */
void enable_gpe(u32 mask)
{
    u32 gpe0_reg =  LP_GPE0_EN_4;
    u32 gpe0_en = inl(get_pmbase() + gpe0_reg);
    gpe0_en |= mask;
    outl(gpe0_en, get_pmbase() + gpe0_reg);
}

#define GTT_RETRY 1000
int gtt_poll(u32 reg, u32 mask, u32 value)
{
    unsigned try = GTT_RETRY;
    u32 data;

    while (try--) {
        data = gtt_read(reg);
        if ((data & mask) == value)
            return 1;
        MicroSecondDelay(10);
    }

    DebugPrint(0, "GT init timeout\n");
    return 0;
}

static void power_well_enable(void)
{
    gtt_write(HSW_PWR_WELL_CTL1, HSW_PWR_WELL_ENABLE);
    gtt_poll(HSW_PWR_WELL_CTL1, HSW_PWR_WELL_STATE, HSW_PWR_WELL_STATE);
}


static void gma_pm_init_pre_vbios()
{
	DebugPrint(0, "GT Power Management Init\n");

	power_well_enable();

	/*
	 * Enable RC6
	 */

	/* Enable Force Wake */
	gtt_write(0x0a180, 1 << 5);
	gtt_write(0x0a188, 0x00010001);
	gtt_poll(FORCEWAKE_ACK_HSW, 1 << 0, 1 << 0);

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


static void gma_pm_init_post_vbios(u16 devId)
{
	int cdclk = 0;
	int devid = devId;
	int gpu_is_ulx = 0;

	gtt_write(CPU_VGACNTRL, CPU_VGA_DISABLE);

	if (devid == 0x0a0e || devid == 0x0a1e)
		gpu_is_ulx = 1;

	/* CD Frequency */
	if ((gtt_read(0x42014) & 0x1000000) || gpu_is_ulx)
		cdclk = 0; /* fixed frequency */
	else
		cdclk = 2; /* variable frequency */

	if (gpu_is_ulx || cdclk != 0)
		gtt_rmw(0x130040, 0xf7ffffff, 0x04000000);
	else
		gtt_rmw(0x130040, 0xf3ffffff, 0x00000000);

	/* More magic */
	if( gpu_is_ulx) {
		if (!gpu_is_ulx)
			gtt_write(0x138128, 0x00000000);
		else
			gtt_write(0x138128, 0x00000001);
		gtt_write(0x13812c, 0x00000000);
		gtt_write(0x138124, 0x80000017);
	}

	/* Disable Force Wake */
	gtt_write(0x0a188, 0x00010000);
	gtt_poll(FORCEWAKE_ACK_HSW, 1 << 0, 0 << 0);
	gtt_write(0x0a188, 0x00000001);
}
 
/* Enable TCO SCI */
void enable_tco_sci(void)
{
    u16 gpe0_sts = LP_GPE0_STS_4;

    /* Clear pending events */
    outl(get_pmbase() + gpe0_sts, TCOSCI_STS);

    /* Enable TCO SCI events */
    enable_gpe(TCOSCI_EN);
}


/* Enable SCI to ACPI _GPE._L06 */
static void gma_enable_swsci(void)
{
    u16 reg16;

    /* clear DMISCI status */
    reg16 = inw(get_pmbase() + TCO1_STS);
    reg16 &= DMISCI_STS;
    outw(get_pmbase() + TCO1_STS, reg16);

    /* clear and enable ACPI TCO SCI */
    enable_tco_sci();
}

void hdgfx_adainit(void);
void gma_test_debugprint(void);
void gma_gfxinit(int* ok);
void Write8(UINT8     Data) {
     __asm__ __volatile__ ("outb %b0,%w1" : : "a" (Data), "d" ((UINT16)0x402));
}
#define PCI_COMMAND  0x04
#define  PCI_COMMAND_MASTER 0x4
#define  PCI_COMMAND_MEMORY 0x2 
#define  PCI_COMMAND_IO     0x1


void gma_func0_init(QEMU_VIDEO_PRIVATE_DATA* dev) {
  EFI_STATUS s;
  static int once = 0;
  if(once++ != 0) {
    return;
  }
  static int lightup_ok = 0;
  u16 deviceId = 0xffffu;
  u32 reg32 = 0x0u;
  s = dev->PciIo->Pci.Read (dev->PciIo, EfiPciIoWidthUint32, PCI_COMMAND, 1, &reg32);
  DebugPrint(0, "PCI READ is %d %8lx\n", s, reg32);
  reg32 |= PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY | PCI_COMMAND_IO;
  DebugPrint(0, "PCI about to write is %8lx\n", reg32);
  s = dev->PciIo->Pci.Write (dev->PciIo, EfiPciIoWidthUint32, PCI_COMMAND, 1, &reg32);
  DebugPrint(0, "PCI WRITE is %d %8lx\n", s, reg32);

  gtt_res_base = 0xffffffffffffffffull;
  s = dev->PciIo->Pci.Read (dev->PciIo, EfiPciIoWidthUint32, PCI_BASE_ADDRESSREG_OFFSET, 1, &gtt_res_base);
  gtt_res_base &= 0x00000000ffffffffull;
  DebugPrint(0, "GTT BASE is %d %16llx\n", s, gtt_res_base);
  dev->PciIo->Pci.Read (dev->PciIo, EfiPciIoWidthUint16, PCI_DEVICE_ID_OFFSET, 1, &deviceId);
  DebugPrint(0, "GTT BASE devid %4x\n", deviceId);

  gma_pm_init_pre_vbios();
  hdgfx_adainit();
  gma_gfxinit(&lightup_ok);
  DebugPrint(0, "Lightup ok is %d\n", lightup_ok);
      /* Post panel init */
  gma_pm_init_post_vbios(deviceId);

  gma_enable_swsci();

}

