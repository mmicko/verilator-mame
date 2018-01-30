// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Poly-88 driver by Miodrag Milanovic

        18/05/2009 Initial implementation

        Poly-88 ToDo:
        MT 06231: Cassette saving hangs. Also, the system has different
        formats depending on a switch. Only one format is currently
        emulated.


        Poly-8813 is a disk-based computer with 3 mini-floppy drives.
        Booting is done by pressing the "Load" button, mounted on the
        front panel near the power switch. Although user manuals are easy
        to obtain, technical information and drive controller schematics
        are not. The disk format is known to be 256 bytes per sector, 10
        sectors per track, 35 tracks, single sided, for a total of 89600
        bytes.

        The Poly-8813 BIOS makes use of undocumented instructions which we
        do not currently emulate. These are at 006A (print a character
        routine - ED ED 05); another is at 0100 (move memory routine -
        ED ED 03); the last is at 087B (disk I/O routine - ED ED 01). The
        code at 0100 can be replaced by 7E 12 13 23 03 79 B0 C2 00 01 C9,
        which exactly fits into the available space. The routine at 006A is
        likewise could be exactly replaced with F5 C5 D5 E5, which enters
        a display routine that appears in other assembly listings but seems
        to have no entry point here. Since the ED ED opcode is defined as
        for CALLN in the NEC V20/V30's 8080 mode, it might be the case that
        these are actually hooks patched into the original code for
        emulation purposes. (There is also a slim possibility that this
        opcode invokes an undocumented feature of the NEC uPD8080AF, which
        at least some models of the Poly-88 are known to have used.)

****************************************************************************/

#include "emu.h"
#include "includes/poly88.h"

#include "cpu/i8085/i8085.h"
//#include "bus/s100/s100.h"
#include "imagedev/cassette.h"
#include "sound/wave.h"
#include "screen.h"
#include "speaker.h"


static ADDRESS_MAP_START(poly88_mem, AS_PROGRAM, 8, poly88_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x03ff) AM_ROM // Monitor ROM
	AM_RANGE(0x0400, 0x0bff) AM_ROM // ROM Expansion
	AM_RANGE(0x0c00, 0x0dff) AM_RAM AM_MIRROR(0x200) // System RAM (mirrored)
	AM_RANGE(0x1000, 0x1fff) AM_ROM // System Expansion area
	AM_RANGE(0x2000, 0x3fff) AM_RAM // Minimal user RAM area
	AM_RANGE(0x4000, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xfbff) AM_RAM AM_SHARE("video_ram") // Video RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( poly88_io, AS_IO, 8, poly88_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVREADWRITE("uart", i8251_device, data_r, data_w)
	AM_RANGE(0x01, 0x01) AM_DEVREADWRITE("uart", i8251_device, status_r, control_w)
	AM_RANGE(0x04, 0x04) AM_WRITE(poly88_baud_rate_w)
	AM_RANGE(0x08, 0x08) AM_WRITE(poly88_intr_w)
	AM_RANGE(0xf8, 0xf8) AM_READ(poly88_keyboard_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START(poly8813_mem, AS_PROGRAM, 8, poly88_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x03ff) AM_ROM // Monitor ROM
	AM_RANGE(0x0400, 0x0bff) AM_ROM // Disk System ROM
	AM_RANGE(0x0c00, 0x0fff) AM_RAM // System RAM
	AM_RANGE(0x1800, 0x1bff) AM_RAM AM_SHARE("video_ram") // Video RAM
	AM_RANGE(0x2000, 0xffff) AM_RAM // RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( poly8813_io, AS_IO, 8, poly88_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( poly88 )
	PORT_START("LINEC")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("LINE0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)

	PORT_START("LINE1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(":") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(";") PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH)

	PORT_START("LINE2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_END)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)

	PORT_START("LINE3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)

	PORT_START("LINE4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)

	PORT_START("LINE5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\\") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("]")  PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("~")  PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DEL")PORT_CODE(KEYCODE_BACKSPACE)

	PORT_START("LINE6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LF") PORT_CODE(KEYCODE_RALT)
INPUT_PORTS_END

static const struct CassetteOptions poly88_cassette_options =
{
	1,      /* channels */
	16,     /* bits per sample */
	7200    /* sample frequency */
};

/* F4 Character Displayer */
static const gfx_layout poly88_charlayout =
{
	8, 16,                  /* text = 7 x 9 */
	128,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( poly88 )
	GFXDECODE_ENTRY( "chargen", 0x0000, poly88_charlayout, 0, 1 )
GFXDECODE_END

MACHINE_CONFIG_START(poly88_state::poly88)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8080A, XTAL(16'588'800) / 9) // uses 8224 clock generator
	MCFG_CPU_PROGRAM_MAP(poly88_mem)
	MCFG_CPU_IO_MAP(poly88_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", poly88_state,  poly88_interrupt)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(poly88_state,poly88_irq_callback)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(64*10, 16*15)
	MCFG_SCREEN_VISIBLE_AREA(0, 64*10-1, 0, 16*15-1)
	MCFG_SCREEN_UPDATE_DRIVER(poly88_state, screen_update_poly88)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", poly88)
	MCFG_PALETTE_ADD_MONOCHROME("palette")


	/* audio hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* cassette */
	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_CREATE_OPTS(&poly88_cassette_options)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED)

	/* uart */
	MCFG_DEVICE_ADD("uart", I8251, XTAL(16'588'800) / 9)
	MCFG_I8251_TXD_HANDLER(WRITELINE(poly88_state,write_cas_tx))
	MCFG_I8251_RXRDY_HANDLER(WRITELINE(poly88_state,poly88_usart_rxready))

	/* snapshot */
	MCFG_SNAPSHOT_ADD("snapshot", poly88_state, poly88, "img", 2)
MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED(poly88_state::poly8813, poly88)
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(poly8813_mem)
	MCFG_CPU_IO_MAP(poly8813_io)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( poly88 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "polymon4.bin", 0x0000, 0x0400, CRC(0baa1a4c) SHA1(c6cf4b89bdde200813d34aab08150d5f3025ce33))
	ROM_LOAD( "tbasic_1.rom", 0x0400, 0x0400, CRC(ec22740e) SHA1(bc606c58ef5f046200bdf402eda66ec070464306))
	ROM_LOAD( "tbasic_2.rom", 0x0800, 0x0400, CRC(f2619232) SHA1(eb6fb0356d2fb153111cfddf39eab10253cb4c53))

	ROM_REGION( 0x800, "chargen", 0 )
	ROM_LOAD( "6571.bin", 0x0000, 0x0800, CRC(5a25144b) SHA1(7b9fee0c8ef2605b85d12b6d9fe8feb82418c63a) )
ROM_END

ROM_START( poly8813 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "poly8813-1.bin", 0x0000, 0x0400, CRC(7fd980a0) SHA1(a71d5999deb4323a11db1c0ea0dcb1dacfaf47ef))
	ROM_LOAD( "poly8813-2.rom", 0x0400, 0x0400, CRC(1ad7c06c) SHA1(c96b8f03c184de58dbdcee18d297dbccf2d77176))
	ROM_LOAD( "poly8813-3.rom", 0x0800, 0x0400, CRC(3df57e5b) SHA1(5b0c4febfc7515fc07e63dcb21d0ab32bc6a2e46))

	ROM_REGION( 0x800, "chargen", 0 )
	ROM_LOAD( "6571.bin", 0x0000, 0x0800, CRC(5a25144b) SHA1(7b9fee0c8ef2605b85d12b6d9fe8feb82418c63a) )
ROM_END
/* Driver */

//    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT   STATE          INIT    COMPANY                  FULLNAME     FLAGS
COMP( 1976, poly88,  0,     0,       poly88,    poly88, poly88_state,  poly88, "PolyMorphic Systems",   "Poly-88",   0 )
COMP( 1977, poly8813,poly88,0,       poly8813,  poly88, poly88_state,  poly88, "PolyMorphic Systems",   "Poly-8813", MACHINE_NOT_WORKING )