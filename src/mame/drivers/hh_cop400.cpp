// license:BSD-3-Clause
// copyright-holders:hap, Sean Riddle
/***************************************************************************

  National Semiconductor COP400 MCU handhelds or other simple devices,
  mostly LED electronic games/toys.

  TODO:
  - why does h2hbaskb(and clones) need a workaround on writing L pins?
  - plus1: which sensor position is which colour?
  - vidchal: Add screen and gun cursor with brightness detection callback,
    and softwarelist for the video tapes. We'd also need a VHS player device.
    The emulated lightgun itself appears to be working fine(eg. add a 30hz
    timer to IN3 to score +100)

***************************************************************************/

#include "emu.h"
#include "cpu/cop400/cop400.h"

#include "machine/timer.h"
#include "sound/spkrdev.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
//#include "rendlay.h"
#include "screen.h"
#include "speaker.h"

// internal artwork
#include "bship82.lh" // clickable
#include "ctstein.lh" // clickable
#include "einvaderc.lh"
#include "funjacks.lh" // clickable
#include "funrlgl.lh"
#include "h2hbaskb.lh"
#include "h2hhockey.lh"
#include "h2hsoccer.lh"
#include "lchicken.lh" // clickable
#include "lightfgt.lh" // clickable
#include "mdallas.lh"
#include "qkracer.lh"
#include "unkeinv.lh"
#include "vidchal.lh"

//#include "hh_cop400_test.lh" // common test-layout - use external artwork


class hh_cop400_state : public driver_device
{
public:
	hh_cop400_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_inp_matrix(*this, "IN.%u", 0),
		m_out_x(*this, "%u.%u", 0U, 0U),
		m_out_a(*this, "%u.a", 0U),
		m_out_digit(*this, "digit%u", 0U),
		m_speaker(*this, "speaker"),
		m_display_wait(33),
		m_display_maxy(1),
		m_display_maxx(0)
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	optional_ioport_array<6> m_inp_matrix; // max 6
	output_finder<0x20, 0x20> m_out_x;
	output_finder<0x20> m_out_a;
	output_finder<0x20> m_out_digit;
	optional_device<speaker_sound_device> m_speaker;

	// misc common
	u8 m_l;                         // MCU port L write data
	u8 m_g;                         // MCU port G write data
	u8 m_d;                         // MCU port D write data
	int m_so;                       // MCU SO line state
	int m_sk;                       // MCU SK line state
	u16 m_inp_mux;                  // multiplexed inputs mask

	u16 read_inputs(int columns, u16 colmask = ~0);

	// display common
	int m_display_wait;             // led/lamp off-delay in milliseconds (default 33ms)
	int m_display_maxy;             // display matrix number of rows
	int m_display_maxx;             // display matrix number of columns (max 31 for now)

	u32 m_display_state[0x20];      // display matrix rows data (last bit is used for always-on)
	u16 m_display_segmask[0x20];    // if not 0, display matrix row is a digit, mask indicates connected segments
	u8 m_display_decay[0x20][0x20]; // (internal use)

	TIMER_DEVICE_CALLBACK_MEMBER(display_decay_tick);
	void display_update();
	void set_display_size(int maxx, int maxy);
	void set_display_segmask(u32 digits, u32 mask);
	void display_matrix(int maxx, int maxy, u32 setx, u32 sety, bool update = true);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
};


// machine start/reset

void hh_cop400_state::machine_start()
{
	// resolve handlers
	m_out_x.resolve();
	m_out_a.resolve();
	m_out_digit.resolve();

	// zerofill
	memset(m_display_state, 0, sizeof(m_display_state));
	memset(m_display_decay, 0, sizeof(m_display_decay));
	memset(m_display_segmask, 0, sizeof(m_display_segmask));

	m_l = 0;
	m_g = 0;
	m_d = 0;
	m_so = 0;
	m_sk = 0;
	m_inp_mux = ~0;

	// register for savestates
	save_item(NAME(m_display_maxy));
	save_item(NAME(m_display_maxx));
	save_item(NAME(m_display_wait));

	save_item(NAME(m_display_state));
	save_item(NAME(m_display_decay));
	save_item(NAME(m_display_segmask));

	save_item(NAME(m_l));
	save_item(NAME(m_g));
	save_item(NAME(m_d));
	save_item(NAME(m_so));
	save_item(NAME(m_sk));
	save_item(NAME(m_inp_mux));
}

void hh_cop400_state::machine_reset()
{
}



/***************************************************************************

  Helper Functions

***************************************************************************/

// The device may strobe the outputs very fast, it is unnoticeable to the user.
// To prevent flickering here, we need to simulate a decay.

void hh_cop400_state::display_update()
{
	for (int y = 0; y < m_display_maxy; y++)
	{
		u32 active_state = 0;

		for (int x = 0; x <= m_display_maxx; x++)
		{
			// turn on powered segments
			if (m_display_state[y] >> x & 1)
				m_display_decay[y][x] = m_display_wait;

			// determine active state
			u32 ds = (m_display_decay[y][x] != 0) ? 1 : 0;
			active_state |= (ds << x);

			// output to y.x, or y.a when always-on
			if (x != m_display_maxx)
				m_out_x[y][x] = ds;
			else
				m_out_a[y] = ds;
		}

		// output to digity
		if (m_display_segmask[y] != 0)
			m_out_digit[y] = active_state & m_display_segmask[y];
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(hh_cop400_state::display_decay_tick)
{
	// slowly turn off unpowered segments
	for (int y = 0; y < m_display_maxy; y++)
		for (int x = 0; x <= m_display_maxx; x++)
			if (m_display_decay[y][x] != 0)
				m_display_decay[y][x]--;

	display_update();
}

void hh_cop400_state::set_display_size(int maxx, int maxy)
{
	m_display_maxx = maxx;
	m_display_maxy = maxy;
}

void hh_cop400_state::set_display_segmask(u32 digits, u32 mask)
{
	// set a segment mask per selected digit, but leave unselected ones alone
	for (int i = 0; i < 0x20; i++)
	{
		if (digits & 1)
			m_display_segmask[i] = mask;
		digits >>= 1;
	}
}

void hh_cop400_state::display_matrix(int maxx, int maxy, u32 setx, u32 sety, bool update)
{
	set_display_size(maxx, maxy);

	// update current state
	u32 mask = (1 << maxx) - 1;
	for (int y = 0; y < maxy; y++)
		m_display_state[y] = (sety >> y & 1) ? ((setx & mask) | (1 << maxx)) : 0;

	if (update)
		display_update();
}


// generic input handlers

u16 hh_cop400_state::read_inputs(int columns, u16 colmask)
{
	// active low
	u16 ret = ~0 & colmask;

	// read selected input rows
	for (int i = 0; i < columns; i++)
		if (~m_inp_mux >> i & 1)
			ret &= m_inp_matrix[i]->read();

	return ret;
}



/***************************************************************************

  Minidrivers (subclass, I/O, Inputs, Machine Config)

***************************************************************************/

/***************************************************************************

  Castle Toy Einstein
  * COP421 MCU label ~/927 COP421-NEZ/N
  * 4 lamps, 1-bit sound

  This is a Simon clone, the tones are not harmonic. Two models exist, each
  with a different batteries setup, assume they're same otherwise.

***************************************************************************/

class ctstein_state : public hh_cop400_state
{
public:
	ctstein_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_cop400_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(write_g);
	DECLARE_WRITE8_MEMBER(write_l);
	DECLARE_READ8_MEMBER(read_l);
	void ctstein(machine_config &config);
};

// handlers

WRITE8_MEMBER(ctstein_state::write_g)
{
	// G0-G2: input mux
	m_inp_mux = data & 7;
}

WRITE8_MEMBER(ctstein_state::write_l)
{
	// L0-L3: button lamps
	display_matrix(4, 1, data & 0xf, 1);
}

READ8_MEMBER(ctstein_state::read_l)
{
	// L4-L7: multiplexed inputs
	return read_inputs(3, 0xf) << 4 | 0xf;
}

// config

static INPUT_PORTS_START( ctstein )
	PORT_START("IN.0") // G0 port L
	PORT_CONFNAME( 0x0f, 0x01^0x0f, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x01^0x0f, "1" )
	PORT_CONFSETTING(    0x02^0x0f, "2" )
	PORT_CONFSETTING(    0x04^0x0f, "3" )
	PORT_CONFSETTING(    0x08^0x0f, "4" )

	PORT_START("IN.1") // G1 port L
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SELECT ) PORT_NAME("Best Score")
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN.2") // G2 port L
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Red Button")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Yellow Button")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Green Button")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Blue Button")
INPUT_PORTS_END

MACHINE_CONFIG_START(ctstein_state::ctstein)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", COP421, 850000) // approximation - RC osc. R=12K, C=100pF
	MCFG_COP400_CONFIG(COP400_CKI_DIVISOR_4, COP400_CKO_OSCILLATOR_OUTPUT, false) // guessed
	MCFG_COP400_WRITE_G_CB(WRITE8(ctstein_state, write_g))
	MCFG_COP400_WRITE_L_CB(WRITE8(ctstein_state, write_l))
	MCFG_COP400_WRITE_SK_CB(DEVWRITELINE("speaker", speaker_sound_device, level_w))
	MCFG_COP400_READ_L_CB(READ8(ctstein_state, read_l))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_cop400_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_ctstein)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Coleco Head to Head: Electronic Basketball/Hockey/Soccer (model 2150/2160/2170)
  * COP420 MCU label COP420L-NEZ/N
  * 2-digit 7seg display, 41 other leds, 1-bit sound

  3 Head to Head games were released using this MCU/ROM. They play very much
  the same, only differing on game time. The PCB is pre-configured on G1+IN2
  and IN3 to select the game.

  An earlier revision of this runs on TMS1000. Model numbers are the same.
  From the outside, an easy way to spot the difference is the Start/Display
  button: TMS1000 version button label is D, COP420 label is a *.

***************************************************************************/

class h2hbaskb_state : public hh_cop400_state
{
public:
	h2hbaskb_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_cop400_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(write_d);
	DECLARE_WRITE8_MEMBER(write_g);
	DECLARE_WRITE8_MEMBER(write_l);
	DECLARE_READ8_MEMBER(read_in);
	void h2hsoccer(machine_config &config);
	void h2hbaskb(machine_config &config);
	void h2hhockey(machine_config &config);
};

// handlers

WRITE8_MEMBER(h2hbaskb_state::write_d)
{
	// D: led select
	m_d = data & 0xf;
}

WRITE8_MEMBER(h2hbaskb_state::write_g)
{
	// G: led select, input mux
	m_inp_mux = data;
	m_g = data & 0xf;
}

WRITE8_MEMBER(h2hbaskb_state::write_l)
{
	// D2,D3 double as multiplexer
	u16 mask = ((m_d >> 2 & 1) * 0x00ff) | ((m_d >> 3 & 1) * 0xff00);
	u16 sel = (m_g | m_d << 4 | m_g << 8 | m_d << 12) & mask;

	// D2+G0,G1 are 7segs
	set_display_segmask(3, 0x7f);

	// L0-L6: digit segments A-G, L0-L4: led data
	// strobe display
	display_matrix(7, 16, data, sel);
	display_matrix(7, 16, 0, 0);
}

READ8_MEMBER(h2hbaskb_state::read_in)
{
	// IN: multiplexed inputs
	return read_inputs(4, 7) | (m_inp_matrix[4]->read() & 8);
}

// config

static INPUT_PORTS_START( h2hbaskb )
	PORT_START("IN.0") // G0 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_NAME("P1 Pass CW") // clockwise
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_NAME("P1 Pass CCW") // counter-clockwise
	PORT_CONFNAME( 0x04, 0x04, DEF_STR( Players ) )
	PORT_CONFSETTING(    0x04, "1" )
	PORT_CONFSETTING(    0x00, "2" )

	PORT_START("IN.1") // G1 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Shoot")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START ) PORT_NAME("Start/Display")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL ) // factory set

	PORT_START("IN.2") // G2 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL PORT_16WAY PORT_NAME("P2 Defense Right")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL PORT_16WAY PORT_NAME("P2 Defense Left")
	PORT_CONFNAME( 0x04, 0x04, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x04, "1" )
	PORT_CONFSETTING(    0x00, "2" )

	PORT_START("IN.3") // G3 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_CONFNAME( 0x04, 0x04, "Factory Test" )
	PORT_CONFSETTING(    0x04, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN.4") // IN3 (factory set)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL )
INPUT_PORTS_END

static INPUT_PORTS_START( h2hhockey )
	PORT_INCLUDE( h2hbaskb )

	PORT_MODIFY("IN.3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Goalie Right")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Goalie Left")

	PORT_MODIFY("IN.4")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SPECIAL )
INPUT_PORTS_END

static INPUT_PORTS_START( h2hsoccer )
	PORT_INCLUDE( h2hhockey )

	PORT_MODIFY("IN.1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SPECIAL )
INPUT_PORTS_END

MACHINE_CONFIG_START(h2hbaskb_state::h2hbaskb)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", COP420, 850000) // approximation - RC osc. R=43K, C=101pF
	MCFG_COP400_CONFIG(COP400_CKI_DIVISOR_8, COP400_CKO_OSCILLATOR_OUTPUT, false) // guessed
	MCFG_COP400_WRITE_D_CB(WRITE8(h2hbaskb_state, write_d))
	MCFG_COP400_WRITE_G_CB(WRITE8(h2hbaskb_state, write_g))
	MCFG_COP400_WRITE_L_CB(WRITE8(h2hbaskb_state, write_l))
	MCFG_COP400_READ_IN_CB(READ8(h2hbaskb_state, read_in))
	MCFG_COP400_WRITE_SO_CB(DEVWRITELINE("speaker", speaker_sound_device, level_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_cop400_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_h2hbaskb)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED(h2hbaskb_state::h2hhockey, h2hbaskb)

	/* basic machine hardware */
	MCFG_DEFAULT_LAYOUT(layout_h2hhockey)
MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED(h2hbaskb_state::h2hsoccer, h2hbaskb)

	/* basic machine hardware */
	MCFG_DEFAULT_LAYOUT(layout_h2hsoccer)
MACHINE_CONFIG_END





/***************************************************************************

  Entex Space Invader
  * COP444L MCU label /B138 COPL444-HRZ/N INV II (die label HRZ COP 444L/A)
  * 3 7seg LEDs, LED matrix and overlay mask, 1-bit sound

  The first version was on TMS1100 (see hh_tms1k.c), this is the reprogrammed
  second release with a gray case instead of black.

***************************************************************************/

class einvaderc_state : public hh_cop400_state
{
public:
	einvaderc_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_cop400_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(write_d);
	DECLARE_WRITE8_MEMBER(write_g);
	DECLARE_WRITE_LINE_MEMBER(write_sk);
	DECLARE_WRITE_LINE_MEMBER(write_so);
	DECLARE_WRITE8_MEMBER(write_l);
	void einvaderc(machine_config &config);
};

// handlers

void einvaderc_state::prepare_display()
{
	// D0-D2 are 7segs
	set_display_segmask(7, 0x7f);

	// update display
	u8 l = bitswap<8>(m_l,7,6,0,1,2,3,4,5);
	u16 grid = (m_d | m_g << 4 | m_sk << 8 | m_so << 9) ^ 0x0ff;
	display_matrix(8, 10, l, grid);
}

WRITE8_MEMBER(einvaderc_state::write_d)
{
	// D: led grid 0-3
	m_d = data;
	prepare_display();
}

WRITE8_MEMBER(einvaderc_state::write_g)
{
	// G: led grid 4-7
	m_g = data;
	prepare_display();
}

WRITE_LINE_MEMBER(einvaderc_state::write_sk)
{
	// SK: speaker out + led grid 8
	m_speaker->level_w(state);
	m_sk = state;
	prepare_display();
}

WRITE_LINE_MEMBER(einvaderc_state::write_so)
{
	// SO: led grid 9
	m_so = state;
	prepare_display();
}

WRITE8_MEMBER(einvaderc_state::write_l)
{
	// L: led state/segment
	m_l = data;
	prepare_display();
}

// config

static INPUT_PORTS_START( einvaderc )
	PORT_START("IN.0") // port IN
	PORT_CONFNAME( 0x01, 0x01, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x01, "Amateur" )
	PORT_CONFSETTING(    0x00, "Professional" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
INPUT_PORTS_END

MACHINE_CONFIG_START(einvaderc_state::einvaderc)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", COP444L, 850000) // approximation - RC osc. R=47K, C=100pF
	MCFG_COP400_CONFIG(COP400_CKI_DIVISOR_16, COP400_CKO_OSCILLATOR_OUTPUT, false) // guessed
	MCFG_COP400_READ_IN_CB(IOPORT("IN.0"))
	MCFG_COP400_WRITE_D_CB(WRITE8(einvaderc_state, write_d))
	MCFG_COP400_WRITE_G_CB(WRITE8(einvaderc_state, write_g))
	MCFG_COP400_WRITE_SK_CB(WRITELINE(einvaderc_state, write_sk))
	MCFG_COP400_WRITE_SO_CB(WRITELINE(einvaderc_state, write_so))
	MCFG_COP400_WRITE_L_CB(WRITE8(einvaderc_state, write_l))

	/* video hardware */
	MCFG_SCREEN_SVG_ADD("screen", "svg")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(913, 1080)
	MCFG_SCREEN_VISIBLE_AREA(0, 913-1, 0, 1080-1)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_cop400_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_einvaderc)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Gordon Barlow Design electronic Space Invaders game (unreleased, from patent US4345764)
  * COP421 (likely a development chip)
  * 36+9 LEDs, 1-bit sound

  This game is presumedly unreleased. The title is unknown, the patent simply names
  it "Hand-held electronic game". There is no mass-manufacture company assigned
  to it either. The game seems unfinished(no scorekeeping, some bugs), and the design
  is very complex. Player ship and bullets are on a moving "wand", a 2-way mirror
  makes it appear on the same plane as the enemies and barriers.

***************************************************************************/

class unkeinv_state : public hh_cop400_state
{
public:
	unkeinv_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_cop400_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(write_g);
	DECLARE_WRITE8_MEMBER(write_d);
	DECLARE_WRITE8_MEMBER(write_l);
	DECLARE_READ8_MEMBER(read_l);

	DECLARE_INPUT_CHANGED_MEMBER(position_changed);
	void unkeinv(machine_config &config);
};

// handlers

void unkeinv_state::prepare_display()
{
	display_matrix(8+8, 8+12, m_g << 4 | m_d, m_l, false);

	// positional led row is on L6,L7
	u16 wand = m_display_state[7] << 8 | m_display_state[6];
	m_display_state[8 + m_inp_matrix[1]->read()] = wand;
	display_update();
}

WRITE8_MEMBER(unkeinv_state::write_g)
{
	// G0-G3: led select part
	// G2,G3: input mux
	m_g = ~data & 0xf;
	prepare_display();
}

WRITE8_MEMBER(unkeinv_state::write_d)
{
	// D0-D3: led select part
	m_d = ~data & 0xf;
	prepare_display();
}

WRITE8_MEMBER(unkeinv_state::write_l)
{
	// L0-L7: led data
	m_l = ~data & 0xff;
	prepare_display();
}

READ8_MEMBER(unkeinv_state::read_l)
{
	u8 ret = 0xff;

	// L0-L5+G2: positional odd
	// L0-L5+G3: positional even
	u8 pos = m_inp_matrix[1]->read();
	if (m_g & 4 && pos & 1)
		ret ^= (1 << (pos >> 1));
	if (m_g & 8 && ~pos & 1)
		ret ^= (1 << (pos >> 1));

	// L7+G3: fire button
	if (m_g & 8 && m_inp_matrix[0]->read())
		ret ^= 0x80;

	return ret & ~m_l;
}

// config

INPUT_CHANGED_MEMBER(unkeinv_state::position_changed)
{
	prepare_display();
}

static INPUT_PORTS_START( unkeinv )
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START("IN.1")
	PORT_BIT( 0x0f, 0x00, IPT_POSITIONAL ) PORT_POSITIONS(12) PORT_SENSITIVITY(10) PORT_KEYDELTA(1) PORT_CENTERDELTA(0) PORT_CHANGED_MEMBER(DEVICE_SELF, unkeinv_state, position_changed, nullptr)
INPUT_PORTS_END

MACHINE_CONFIG_START(unkeinv_state::unkeinv)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", COP421, 850000) // frequency guessed
	MCFG_COP400_CONFIG(COP400_CKI_DIVISOR_4, COP400_CKO_OSCILLATOR_OUTPUT, false) // guessed
	MCFG_COP400_WRITE_G_CB(WRITE8(unkeinv_state, write_g))
	MCFG_COP400_WRITE_D_CB(WRITE8(unkeinv_state, write_d))
	MCFG_COP400_WRITE_L_CB(WRITE8(unkeinv_state, write_l))
	MCFG_COP400_READ_L_CB(READ8(unkeinv_state, read_l))
	MCFG_COP400_READ_L_TRISTATE_CB(CONSTANT(0xff))
	MCFG_COP400_WRITE_SO_CB(DEVWRITELINE("speaker", speaker_sound_device, level_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_cop400_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_unkeinv)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  LJN I Took a Lickin' From a Chicken
  * COP421 MCU label ~/005 COP421-NJC/N
  * 11 leds, 1-bit sound, motor to a chicken on a spring

  This toy includes 4 games: Tic Tac Toe, Chicken Sez, and Total Recall I/II.

  known releases:
  - USA: I Took a Lickin' From a Chicken
  - Japan: Professor Chicken's Genius Classroom 「にわとり博士の天才教室」, published by Bandai
    (not sure if it's the same ROM, or just licensed the outer shell)

***************************************************************************/

class lchicken_state : public hh_cop400_state
{
public:
	lchicken_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_cop400_state(mconfig, type, tag)
	{ }

	u8 m_motor_pos;
	TIMER_DEVICE_CALLBACK_MEMBER(motor_sim_tick);
	DECLARE_CUSTOM_INPUT_MEMBER(motor_switch);

	DECLARE_WRITE8_MEMBER(write_l);
	DECLARE_WRITE8_MEMBER(write_d);
	DECLARE_WRITE8_MEMBER(write_g);
	DECLARE_READ8_MEMBER(read_g);
	DECLARE_WRITE_LINE_MEMBER(write_so);
	DECLARE_READ_LINE_MEMBER(read_si);
	void lchicken(machine_config &config);

protected:
	virtual void machine_start() override;
};

// handlers

TIMER_DEVICE_CALLBACK_MEMBER(lchicken_state::motor_sim_tick)
{
	if (~m_inp_mux & 8)
	{
		m_motor_pos++;
		output().set_value("motor_pos", 100 * (m_motor_pos / (float)0x100));
	}
}

WRITE8_MEMBER(lchicken_state::write_l)
{
	// L0-L3: led data
	// L4-L6: led select
	// L7: N/C
	display_matrix(4, 3, ~data & 0xf, data >> 4 & 7);
}

WRITE8_MEMBER(lchicken_state::write_d)
{
	// D0-D3: input mux
	// D3: motor on
	m_inp_mux = data & 0xf;
	output().set_value("motor_on", ~data >> 3 & 1);
}

WRITE8_MEMBER(lchicken_state::write_g)
{
	m_g = data;
}

READ8_MEMBER(lchicken_state::read_g)
{
	// G0-G3: multiplexed inputs
	return read_inputs(4, m_g);
}

WRITE_LINE_MEMBER(lchicken_state::write_so)
{
	// SO: speaker out
	m_speaker->level_w(state);
	m_so = state;
}

READ_LINE_MEMBER(lchicken_state::read_si)
{
	// SI: SO
	return m_so;
}

// config

static INPUT_PORTS_START( lchicken )
	PORT_START("IN.0") // D0 port G
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN.1") // D1 port G
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN.2") // D2 port G
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN.3") // D3 port G
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, lchicken_state, motor_switch, nullptr)
INPUT_PORTS_END

CUSTOM_INPUT_MEMBER(lchicken_state::motor_switch)
{
	return m_motor_pos > 0xe8; // approximation
}

void lchicken_state::machine_start()
{
	hh_cop400_state::machine_start();

	// zerofill, register for savestates
	m_motor_pos = 0;
	save_item(NAME(m_motor_pos));
}

MACHINE_CONFIG_START(lchicken_state::lchicken)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", COP421, 850000) // approximation - RC osc. R=12K, C=100pF
	MCFG_COP400_CONFIG(COP400_CKI_DIVISOR_4, COP400_CKO_OSCILLATOR_OUTPUT, false) // guessed
	MCFG_COP400_WRITE_L_CB(WRITE8(lchicken_state, write_l))
	MCFG_COP400_WRITE_D_CB(WRITE8(lchicken_state, write_d))
	MCFG_COP400_WRITE_G_CB(WRITE8(lchicken_state, write_g))
	MCFG_COP400_READ_G_CB(READ8(lchicken_state, read_g))
	MCFG_COP400_WRITE_SO_CB(WRITELINE(lchicken_state, write_so))
	MCFG_COP400_READ_SI_CB(READLINE(lchicken_state, read_si))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("chicken_motor", lchicken_state, motor_sim_tick, attotime::from_msec(6000/0x100)) // ~6sec for a full rotation
	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_cop400_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_lchicken)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Mattel Funtronics: Jacks
  * COP410L MCU bonded directly to PCB (die label COP410L/B NGS)
  * 8 LEDs, 1-bit sound

***************************************************************************/

class funjacks_state : public hh_cop400_state
{
public:
	funjacks_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_cop400_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(write_d);
	DECLARE_WRITE8_MEMBER(write_l);
	DECLARE_WRITE8_MEMBER(write_g);
	DECLARE_READ8_MEMBER(read_l);
	DECLARE_READ8_MEMBER(read_g);
	void funjacks(machine_config &config);
};

// handlers

WRITE8_MEMBER(funjacks_state::write_d)
{
	// D: led grid + input mux
	m_inp_mux = data;
	m_d = ~data & 0xf;
	display_matrix(2, 4, m_l, m_d);
}

WRITE8_MEMBER(funjacks_state::write_l)
{
	// L0,L1: led state
	m_l = data & 3;
	display_matrix(2, 4, m_l, m_d);
}

WRITE8_MEMBER(funjacks_state::write_g)
{
	// G1: speaker out
	m_speaker->level_w(data >> 1 & 1);
	m_g = data;
}

READ8_MEMBER(funjacks_state::read_l)
{
	// L4,L5: multiplexed inputs
	return read_inputs(3, 0x30) | m_l;
}

READ8_MEMBER(funjacks_state::read_g)
{
	// G1: speaker out state
	// G2,G3: inputs
	return m_inp_matrix[3]->read() | (m_g & 2);
}

// config

static INPUT_PORTS_START( funjacks )
	PORT_START("IN.0") // D0 port G
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_START("IN.1") // D1 port G
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 )

	PORT_START("IN.2") // D2 port G
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON6 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) // positioned at 1 o'clock on panel, increment clockwise

	PORT_START("IN.3") // port G
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) // speaker
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START )
	PORT_CONFNAME( 0x08, 0x00, DEF_STR( Players ) )
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x08, "2" )
INPUT_PORTS_END

MACHINE_CONFIG_START(funjacks_state::funjacks)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", COP410, 1000000) // approximation - RC osc. R=47K, C=56pF
	MCFG_COP400_CONFIG(COP400_CKI_DIVISOR_8, COP400_CKO_OSCILLATOR_OUTPUT, false) // guessed
	MCFG_COP400_WRITE_D_CB(WRITE8(funjacks_state, write_d))
	MCFG_COP400_WRITE_L_CB(WRITE8(funjacks_state, write_l))
	MCFG_COP400_WRITE_G_CB(WRITE8(funjacks_state, write_g))
	MCFG_COP400_READ_L_CB(READ8(funjacks_state, read_l))
	MCFG_COP400_READ_G_CB(READ8(funjacks_state, read_g))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_cop400_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_funjacks)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Mattel Funtronics: Red Light Green Light
  * COP410L MCU bonded directly to PCB (die label COP410L/B NHZ)
  * 14 LEDs, 1-bit sound

  known releases:
  - USA: Funtronics: Red Light Green Light
  - USA(rerelease): Funtronics: Hot Wheels Drag Race

***************************************************************************/

class funrlgl_state : public hh_cop400_state
{
public:
	funrlgl_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_cop400_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(write_d);
	DECLARE_WRITE8_MEMBER(write_l);
	DECLARE_WRITE8_MEMBER(write_g);

	DECLARE_INPUT_CHANGED_MEMBER(reset_button);
	void funrlgl(machine_config &config);
};

// handlers

WRITE8_MEMBER(funrlgl_state::write_d)
{
	// D: led grid
	m_d = ~data & 0xf;
	display_matrix(4, 4, m_l, m_d);
}

WRITE8_MEMBER(funrlgl_state::write_l)
{
	// L0-L3: led state
	// L4-L7: N/C
	m_l = ~data & 0xf;
	display_matrix(4, 4, m_l, m_d);
}

WRITE8_MEMBER(funrlgl_state::write_g)
{
	// G3: speaker out
	m_speaker->level_w(data >> 3 & 1);
}

// config

static INPUT_PORTS_START( funrlgl )
	PORT_START("IN.0") // port G
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_CONFNAME( 0x04, 0x04, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x04, "1" )
	PORT_CONFSETTING(    0x00, "2" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_MEMBER(DEVICE_SELF, funrlgl_state, reset_button, nullptr)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(funrlgl_state::reset_button)
{
	// middle button is directly tied to MCU reset pin
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
}

MACHINE_CONFIG_START(funrlgl_state::funrlgl)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", COP410, 1000000) // approximation - RC osc. R=51K, C=91pF
	MCFG_COP400_CONFIG(COP400_CKI_DIVISOR_8, COP400_CKO_OSCILLATOR_OUTPUT, false) // guessed
	MCFG_COP400_WRITE_D_CB(WRITE8(funrlgl_state, write_d))
	MCFG_COP400_WRITE_L_CB(WRITE8(funrlgl_state, write_l))
	MCFG_COP400_READ_L_TRISTATE_CB(CONSTANT(0xff))
	MCFG_COP400_WRITE_G_CB(WRITE8(funrlgl_state, write_g))
	MCFG_COP400_READ_G_CB(IOPORT("IN.0"))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_cop400_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_funrlgl)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Mattel Dalla$ (J.R. handheld)
  * COP444 MCU label COP444L-HYN/N
  * 8-digit 7seg display, 1-bit sound

  This is a board game, only the handheld device is emulated here.

***************************************************************************/

class mdallas_state : public hh_cop400_state
{
public:
	mdallas_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_cop400_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(write_l);
	DECLARE_WRITE8_MEMBER(write_d);
	DECLARE_WRITE8_MEMBER(write_g);
	DECLARE_READ8_MEMBER(read_in);
	void mdallas(machine_config &config);
};

// handlers

void mdallas_state::prepare_display()
{
	set_display_segmask(0xff, 0xff);
	display_matrix(8, 8, m_l, ~(m_d << 4 | m_g));
}

WRITE8_MEMBER(mdallas_state::write_l)
{
	// L: digit segment data
	m_l = data;
	prepare_display();
}

WRITE8_MEMBER(mdallas_state::write_d)
{
	// D: select digit, input mux high
	m_inp_mux = (m_inp_mux & 0xf) | (data << 4 & 3);
	m_d = data & 0xf;
	prepare_display();
}

WRITE8_MEMBER(mdallas_state::write_g)
{
	// G: select digit, input mux low
	m_inp_mux = (m_inp_mux & 0x30) | (data & 0xf);
	m_g = data & 0xf;
	prepare_display();
}

READ8_MEMBER(mdallas_state::read_in)
{
	// IN: multiplexed inputs
	return read_inputs(6, 0xf);
}

// config

/* physical button layout and labels is like this:

    <  ON>  [YES]   [NO]   [NEXT]
    [W]     [N]     [S]    [E]
    [7]     [8]     [9]    [STATUS]
    [4]     [5]     [6]    [ASSETS]
    [1]     [2]     [3]    [START]
    [CLEAR] [0]     [MOVE] [ENTER]
*/

static INPUT_PORTS_START( mdallas )
	PORT_START("IN.0") // G0 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_DEL) PORT_NAME("Clear")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("Move")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter")

	PORT_START("IN.1") // G1 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("Start")

	PORT_START("IN.2") // G2 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("Assets")

	PORT_START("IN.3") // G3 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("Status")

	PORT_START("IN.4") // D0 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("West") // W
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Next")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("East") // E

	PORT_START("IN.5") // D1 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("No")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("Yes")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("South") // S
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("North") // N
INPUT_PORTS_END

MACHINE_CONFIG_START(mdallas_state::mdallas)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", COP444L, 1000000) // approximation - RC osc. R=57K, C=101pF
	MCFG_COP400_CONFIG(COP400_CKI_DIVISOR_16, COP400_CKO_OSCILLATOR_OUTPUT, false) // guessed
	MCFG_COP400_WRITE_L_CB(WRITE8(mdallas_state, write_l))
	MCFG_COP400_WRITE_D_CB(WRITE8(mdallas_state, write_d))
	MCFG_COP400_WRITE_G_CB(WRITE8(mdallas_state, write_g))
	MCFG_COP400_READ_IN_CB(READ8(mdallas_state, read_in))
	MCFG_COP400_WRITE_SO_CB(DEVWRITELINE("speaker", speaker_sound_device, level_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_cop400_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_mdallas)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Milton Bradley Plus One
  * COP410L MCU in 8-pin DIP, label ~/029 MM 57405 (die label COP410L/B NNE)
  * orientation sensor(4 directions), 1-bit sound

  This is a board game, each player needs to rotate a triangular pyramid
  shaped piece the same as the previous player, plus 1.

***************************************************************************/

class plus1_state : public hh_cop400_state
{
public:
	plus1_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_cop400_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(write_d);
	DECLARE_WRITE8_MEMBER(write_l);
	DECLARE_READ8_MEMBER(read_l);
	void plus1(machine_config &config);
};

// handlers

WRITE8_MEMBER(plus1_state::write_d)
{
	// D0?: speaker out
	m_speaker->level_w(data & 1);
}

WRITE8_MEMBER(plus1_state::write_l)
{
	m_l = data;
}

READ8_MEMBER(plus1_state::read_l)
{
	// L: IN.1, mask with output
	return m_inp_matrix[1]->read() & m_l;
}

// config

static INPUT_PORTS_START( plus1 )
	PORT_START("IN.0") // port G
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Sensor Position 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Sensor Position 1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN.1") // port L
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Sensor Position 4")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Sensor Position 2")
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

MACHINE_CONFIG_START(plus1_state::plus1)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", COP410, 1000000) // approximation - RC osc. R=51K, C=100pF
	MCFG_COP400_CONFIG(COP400_CKI_DIVISOR_16, COP400_CKO_OSCILLATOR_OUTPUT, false) // guessed
	MCFG_COP400_WRITE_D_CB(WRITE8(plus1_state, write_d))
	MCFG_COP400_READ_G_CB(IOPORT("IN.0"))
	MCFG_COP400_WRITE_L_CB(WRITE8(plus1_state, write_l))
	MCFG_COP400_READ_L_CB(READ8(plus1_state, read_l))

	/* no visual feedback! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Milton Bradley Electronic Lightfight
  * COP421L MCU label /B119 COP421L-HLA/N
  * LED matrix, 1-bit sound

  Xbox-shaped electronic game for 2 or more players, with long diagonal buttons
  next to each outer LED. The main object of the game is to pinpoint a light
  by pressing 2 buttons. To start, press a skill-level button(P2 button 7/8/9)
  after selecting a game mode(P1 button 6-10).

  The game variations are:
  1: LightFight
  2: NightFight
  3: RiteSite
  4: QuiteBrite
  5: RightLight

***************************************************************************/

class lightfgt_state : public hh_cop400_state
{
public:
	lightfgt_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_cop400_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE_LINE_MEMBER(write_so);
	DECLARE_WRITE8_MEMBER(write_d);
	DECLARE_WRITE8_MEMBER(write_l);
	DECLARE_READ8_MEMBER(read_g);
	void lightfgt(machine_config &config);
};

// handlers

void lightfgt_state::prepare_display()
{
	u8 grid = (m_so | m_d << 1) ^ 0x1f;
	display_matrix(5, 5, m_l, grid);
}

WRITE_LINE_MEMBER(lightfgt_state::write_so)
{
	// SO: led grid 0 (and input mux)
	m_so = state;
	prepare_display();
}

WRITE8_MEMBER(lightfgt_state::write_d)
{
	// D: led grid 1-4 (and input mux)
	m_d = data;
	prepare_display();
}

WRITE8_MEMBER(lightfgt_state::write_l)
{
	// L0-L4: led state
	// L5-L7: N/C
	m_l = data & 0x1f;
	prepare_display();
}

READ8_MEMBER(lightfgt_state::read_g)
{
	// G: multiplexed inputs
	m_inp_mux = m_d << 1 | m_so;
	return read_inputs(5, 0xf);
}

// config

static INPUT_PORTS_START( lightfgt )
	PORT_START("IN.0") // SO port G
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) // note: button 1 is on the left side from player perspective
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_COCKTAIL

	PORT_START("IN.1") // D0 port G
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON7 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_COCKTAIL

	PORT_START("IN.2") // D1 port G
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON8 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_COCKTAIL

	PORT_START("IN.3") // D2 port G
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON9 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_COCKTAIL

	PORT_START("IN.4") // D3 port G
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON10 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_COCKTAIL
INPUT_PORTS_END

MACHINE_CONFIG_START(lightfgt_state::lightfgt)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", COP421, 950000) // approximation - RC osc. R=82K, C=56pF
	MCFG_COP400_CONFIG(COP400_CKI_DIVISOR_16, COP400_CKO_OSCILLATOR_OUTPUT, false) // guessed
	MCFG_COP400_WRITE_SO_CB(WRITELINE(lightfgt_state, write_so))
	MCFG_COP400_WRITE_D_CB(WRITE8(lightfgt_state, write_d))
	MCFG_COP400_WRITE_L_CB(WRITE8(lightfgt_state, write_l))
	MCFG_COP400_WRITE_SK_CB(DEVWRITELINE("speaker", speaker_sound_device, level_w))
	MCFG_COP400_READ_G_CB(READ8(lightfgt_state, read_g))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_cop400_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_lightfgt)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Milton Bradley Electronic Battleship (1982 version)
  * COP420 MCU label COP420-JWE/N

  see hh_tms1k.cpp bship driver for more information

***************************************************************************/

class bship82_state : public hh_cop400_state
{
public:
	bship82_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_cop400_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(write_d);
	DECLARE_READ8_MEMBER(read_l);
	DECLARE_READ8_MEMBER(read_in);
	DECLARE_WRITE_LINE_MEMBER(write_so);
	void bship82(machine_config &config);
};

// handlers

WRITE8_MEMBER(bship82_state::write_d)
{
	// D: input mux
	m_inp_mux = data;
}

READ8_MEMBER(bship82_state::read_l)
{
	// L: multiplexed inputs
	return read_inputs(4, 0xff);
}

READ8_MEMBER(bship82_state::read_in)
{
	// IN: multiplexed inputs
	return read_inputs(4, 0xf00) >> 8;
}

WRITE_LINE_MEMBER(bship82_state::write_so)
{
	// SO: led
	display_matrix(1, 1, state, 1);
}

// config

static INPUT_PORTS_START( bship82 )
	PORT_START("IN.0") // D0 ports L,IN
	PORT_BIT( 0x001, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("P1 Clear Last Entry") // CLE
	PORT_BIT( 0x002, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("P1 A")
	PORT_BIT( 0x004, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("P1 B")
	PORT_BIT( 0x008, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("P1 C")
	PORT_BIT( 0x010, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("P1 D")
	PORT_BIT( 0x020, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("P1 E")
	PORT_BIT( 0x040, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("P1 F")
	PORT_BIT( 0x080, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("P1 G")
	PORT_BIT( 0x100, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("P1 H")
	PORT_BIT( 0x200, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("P1 I")
	PORT_BIT( 0x400, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("P1 J")
	PORT_BIT( 0x800, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN.1") // D1 ports L,IN
	PORT_BIT( 0x001, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_NAME("P1 Clear Memory") // CM
	PORT_BIT( 0x002, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("P1 1")
	PORT_BIT( 0x004, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("P1 2")
	PORT_BIT( 0x008, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("P1 3")
	PORT_BIT( 0x010, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("P1 4")
	PORT_BIT( 0x020, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("P1 5")
	PORT_BIT( 0x040, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("P1 6")
	PORT_BIT( 0x080, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("P1 7")
	PORT_BIT( 0x100, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("P1 8")
	PORT_BIT( 0x200, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("P1 9")
	PORT_BIT( 0x400, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("P1 10")
	PORT_BIT( 0x800, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("P1 Fire")

	PORT_START("IN.2") // D2 ports L,IN
	PORT_BIT( 0x001, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 Clear Last Entry") // CLE
	PORT_BIT( 0x002, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 A")
	PORT_BIT( 0x004, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 B")
	PORT_BIT( 0x008, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 C")
	PORT_BIT( 0x010, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 D")
	PORT_BIT( 0x020, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 E")
	PORT_BIT( 0x040, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 F")
	PORT_BIT( 0x080, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 G")
	PORT_BIT( 0x100, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 H")
	PORT_BIT( 0x200, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 I")
	PORT_BIT( 0x400, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 J")
	PORT_BIT( 0x800, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN.3") // D3 ports L,IN
	PORT_BIT( 0x001, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 Clear Memory") // CM
	PORT_BIT( 0x002, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 1")
	PORT_BIT( 0x004, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 2")
	PORT_BIT( 0x008, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 3")
	PORT_BIT( 0x010, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 4")
	PORT_BIT( 0x020, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 5")
	PORT_BIT( 0x040, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 6")
	PORT_BIT( 0x080, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 7")
	PORT_BIT( 0x100, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 8")
	PORT_BIT( 0x200, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 9")
	PORT_BIT( 0x400, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 10")
	PORT_BIT( 0x800, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("P2 Fire")

	PORT_START("IN.4") // SI
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_TOGGLE PORT_CODE(KEYCODE_F1) PORT_NAME("Load/Go") // switch
INPUT_PORTS_END

/*

http://www.seanriddle.com/bship82.txt

21 G0     3.9K resistor to speaker transistor base
22 G1     2.2K resistor to speaker transistor base
23 G2     1.0K resistor to speaker transistor base
24 G3     speaker transistor base tied high 4.7K

speaker connection
2N3904 transistor:
emitter to 10ohm resistor to ground
collector to 68ohm resistor to speaker (other speaker terminal to VCC)
base pulled high with 4.7K resistor, connects directly to G3, 1K resistor to G2,
2.2K resistor to G1, 3.9K resistor to G0

*/

MACHINE_CONFIG_START(bship82_state::bship82)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", COP420, 750000) // approximation - RC osc. R=14K, C=100pF
	MCFG_COP400_CONFIG(COP400_CKI_DIVISOR_4, COP400_CKO_OSCILLATOR_OUTPUT, false) // guessed
	MCFG_COP400_WRITE_D_CB(WRITE8(bship82_state, write_d))
	MCFG_COP400_WRITE_G_CB(DEVWRITE8("dac", dac_byte_interface, write))
	MCFG_COP400_READ_L_CB(READ8(bship82_state, read_l))
	MCFG_COP400_READ_IN_CB(READ8(bship82_state, read_in))
	MCFG_COP400_WRITE_SO_CB(WRITELINE(bship82_state, write_so))
	MCFG_COP400_READ_SI_CB(IOPORT("IN.4"))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_cop400_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_bship82)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC_4BIT_BINARY_WEIGHTED_SIGN_MAGNITUDE, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.125) // see above
	MCFG_DEVICE_ADD("vref", VOLTAGE_REGULATOR, 0) MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE_EX(0, "dac", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE_EX(0, "dac", -1.0, DAC_VREF_NEG_INPUT)
MACHINE_CONFIG_END





/***************************************************************************

  National Semiconductor QuizKid Racer (COP420 version)
  * COP420 MCU label COP420-NPG/N
  * 8-digit 7seg led display(1 custom digit), 1 green led, no sound

  This is the COP420 version, the first release was on a MM5799 MCU.

***************************************************************************/

class qkracer_state : public hh_cop400_state
{
public:
	qkracer_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_cop400_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(write_d);
	DECLARE_WRITE8_MEMBER(write_g);
	DECLARE_WRITE8_MEMBER(write_l);
	DECLARE_READ8_MEMBER(read_in);
	DECLARE_WRITE_LINE_MEMBER(write_sk);
	void qkracer(machine_config &config);
};

// handlers

void qkracer_state::prepare_display()
{
	set_display_segmask(0xdf, 0x7f);
	set_display_segmask(0x20, 0x41); // equals sign

	display_matrix(7, 9, m_l, ~(m_d | m_g << 4 | m_sk << 8));
}

WRITE8_MEMBER(qkracer_state::write_d)
{
	// D: select digit, D3: input mux high bit
	m_inp_mux = (m_inp_mux & 0xf) | (data << 1 & 0x10);
	m_d = data & 0xf;
	prepare_display();
}

WRITE8_MEMBER(qkracer_state::write_g)
{
	// G: select digit, input mux
	m_inp_mux = (m_inp_mux & 0x10) | (data & 0xf);
	m_g = data & 0xf;
	prepare_display();
}

WRITE8_MEMBER(qkracer_state::write_l)
{
	// L0-L6: digit segment data
	m_l = data & 0x7f;
	prepare_display();
}

READ8_MEMBER(qkracer_state::read_in)
{
	// IN: multiplexed inputs
	return read_inputs(5, 0xf);
}

WRITE_LINE_MEMBER(qkracer_state::write_sk)
{
	// SK: green led
	m_sk = state;
	prepare_display();
}

// config

static INPUT_PORTS_START( qkracer )
	PORT_START("IN.0") // G0 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME(UTF8_DIVIDE)

	PORT_START("IN.1") // G1 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME(UTF8_MULTIPLY)

	PORT_START("IN.2") // G2 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")

	PORT_START("IN.3") // G3 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Slow")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Fast")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")

	PORT_START("IN.4") // D3 port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("Amateur")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("Pro")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("Complex")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("Tables")
INPUT_PORTS_END

MACHINE_CONFIG_START(qkracer_state::qkracer)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", COP420, 1000000) // approximation - RC osc. R=47K, C=100pF
	MCFG_COP400_CONFIG(COP400_CKI_DIVISOR_32, COP400_CKO_OSCILLATOR_OUTPUT, false) // guessed
	MCFG_COP400_WRITE_D_CB(WRITE8(qkracer_state, write_d))
	MCFG_COP400_WRITE_G_CB(WRITE8(qkracer_state, write_g))
	MCFG_COP400_WRITE_L_CB(WRITE8(qkracer_state, write_l))
	MCFG_COP400_READ_IN_CB(READ8(qkracer_state, read_in))
	MCFG_COP400_WRITE_SK_CB(WRITELINE(qkracer_state, write_sk))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_cop400_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_qkracer)

	/* no sound! */
MACHINE_CONFIG_END





/***************************************************************************

  Select Merchandise Video Challenger
  * COP420 MCU label COP420-TDX/N
  * 6-digit 7seg led display, 3 other leds, 4-bit sound

  This is a lightgun with scorekeeping. The "games" themselves were released
  on VHS tapes. To determine scoring, the lightgun detects strobe lighting
  from objects in the video.

  known releases:
  - Japan: Video Challenger, published by Takara
  - UK: Video Challenger, published by Bandai
  - Canada: Video Challenger, published by Irwin

***************************************************************************/

class vidchal_state : public hh_cop400_state
{
public:
	vidchal_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_cop400_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE8_MEMBER(write_d);
	DECLARE_WRITE8_MEMBER(write_l);
	DECLARE_WRITE_LINE_MEMBER(write_sk);
	void vidchal(machine_config &config);
};

// handlers

void vidchal_state::prepare_display()
{
	set_display_segmask(0x3f, 0xff);
	display_matrix(8, 7, m_l, m_d | m_sk << 6);
}

WRITE8_MEMBER(vidchal_state::write_d)
{
	// D: CD4028BE to digit select
	m_d = 1 << data & 0x3f;
	prepare_display();
}

WRITE8_MEMBER(vidchal_state::write_l)
{
	// L: digit segment data
	m_l = bitswap<8>(data,0,3,1,5,4,7,2,6);
	prepare_display();
}

WRITE_LINE_MEMBER(vidchal_state::write_sk)
{
	// SK: hit led
	m_sk = state;
	prepare_display();
}

// config

static INPUT_PORTS_START( vidchal )
	PORT_START("IN.0") // port IN
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) // TODO: light sensor
INPUT_PORTS_END

MACHINE_CONFIG_START(vidchal_state::vidchal)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", COP420, 900000) // approximation
	MCFG_COP400_CONFIG(COP400_CKI_DIVISOR_4, COP400_CKO_OSCILLATOR_OUTPUT, false) // guessed
	MCFG_COP400_WRITE_D_CB(WRITE8(vidchal_state, write_d))
	MCFG_COP400_WRITE_G_CB(DEVWRITE8("dac", dac_byte_interface, write))
	MCFG_COP400_WRITE_L_CB(WRITE8(vidchal_state, write_l))
	MCFG_COP400_READ_IN_CB(IOPORT("IN.0"))
	MCFG_COP400_WRITE_SK_CB(WRITELINE(vidchal_state, write_sk))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_cop400_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_vidchal)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC_4BIT_BINARY_WEIGHTED_SIGN_MAGNITUDE, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.125) // unknown DAC
	MCFG_DEVICE_ADD("vref", VOLTAGE_REGULATOR, 0) MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE_EX(0, "dac", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE_EX(0, "dac", -1.0, DAC_VREF_NEG_INPUT)
MACHINE_CONFIG_END





/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( ctstein )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "cop421-nez_n", 0x0000, 0x0400, CRC(16148e03) SHA1(b2b74891d36813d9a1eefd56a925054997c4b7f7) ) // 2nd half empty
ROM_END


ROM_START( h2hbaskb )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "cop420l-nmy", 0x0000, 0x0400, CRC(87152509) SHA1(acdb869b65d49b3b9855a557ed671cbbb0f61e2c) )
ROM_END

ROM_START( h2hhockey ) // dumped from Basketball
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "cop420l-nmy", 0x0000, 0x0400, CRC(87152509) SHA1(acdb869b65d49b3b9855a557ed671cbbb0f61e2c) )
ROM_END

ROM_START( h2hsoccer ) // dumped from Basketball
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "cop420l-nmy", 0x0000, 0x0400, CRC(87152509) SHA1(acdb869b65d49b3b9855a557ed671cbbb0f61e2c) )
ROM_END


ROM_START( einvaderc )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "copl444-hrz_n_inv_ii", 0x0000, 0x0800, CRC(76400f38) SHA1(0e92ab0517f7b7687293b189d30d57110df20fe0) )

	ROM_REGION( 80636, "svg", 0)
	ROM_LOAD( "einvaderc.svg", 0, 80636, CRC(a52d0166) SHA1(f69397ebcc518701f30a47b4d62e5a700825375a) )
ROM_END


ROM_START( unkeinv )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "cop421_us4345764", 0x0000, 0x0400, CRC(0068c3a3) SHA1(4e5fd566a5a26c066cc14623a9bd01e109ebf797) ) // typed in from patent US4345764, good print quality
ROM_END


ROM_START( lchicken )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "cop421-njc_n", 0x0000, 0x0400, CRC(319e7985) SHA1(9714327518f65ebefe38ac7911bed2b9b9c77307) )
ROM_END


ROM_START( funjacks )
	ROM_REGION( 0x0200, "maincpu", 0 )
	ROM_LOAD( "cop410l_b_ngs", 0x0000, 0x0200, CRC(863368ea) SHA1(f116cc27ae721b3a3e178fa13765808bdc275663) )
ROM_END


ROM_START( funrlgl )
	ROM_REGION( 0x0200, "maincpu", 0 )
	ROM_LOAD( "cop410l_b_nhz", 0x0000, 0x0200, CRC(4065c3ce) SHA1(f0bc8125d922949e0d7ab1ba89c805a836d20e09) )
ROM_END


ROM_START( mdallas )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "copl444l-hyn_n", 0x0000, 0x0800, CRC(7848b78c) SHA1(778d24512180892f58c49df3c72ca77b2618d63b) )
ROM_END


ROM_START( plus1 )
	ROM_REGION( 0x0200, "maincpu", 0 )
	ROM_LOAD( "cop410l_b_nne", 0x0000, 0x0200, CRC(d861b80c) SHA1(4652f8ee0dd4c3c48b625285bb4f094d96434071) )
ROM_END


ROM_START( lightfgt )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "cop421l-hla_n", 0x0000, 0x0400, CRC(aceb2d65) SHA1(2328cbb195faf93c575f3afa3a1fe0079180edd7) )
ROM_END


ROM_START( bship82 )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "cop420-jwe_n", 0x0000, 0x0400, CRC(5ea8111a) SHA1(34931463b806b48dce4f8ae2361512510bae0ebf) )
ROM_END


ROM_START( qkracer )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "cop420-npg_n", 0x0000, 0x0400, CRC(17f8e538) SHA1(23d1a1819e6ba552d8da83da2948af1cf5b13d5b) )
ROM_END


ROM_START( vidchal )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "cop420-tdx_n", 0x0000, 0x0400, CRC(c9bd041c) SHA1(ab0dcaf4741620fa4c28ab75337a23d646af7626) )
ROM_END



//    YEAR  NAME       PARENT   CMP MACHINE    INPUT      STATE          INIT COMPANY, FULLNAME, FLAGS
CONS( 1979, ctstein,   0,        0, ctstein,   ctstein,   ctstein_state,   0, "Castle Toy", "Einstein (Castle Toy)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

CONS( 1980, h2hbaskb,  0,        0, h2hbaskb,  h2hbaskb,  h2hbaskb_state,  0, "Coleco", "Head to Head: Electronic Basketball (COP420L version)", MACHINE_SUPPORTS_SAVE )
CONS( 1980, h2hhockey, h2hbaskb, 0, h2hhockey, h2hhockey, h2hbaskb_state,  0, "Coleco", "Head to Head: Electronic Hockey (COP420L version)", MACHINE_SUPPORTS_SAVE )
CONS( 1980, h2hsoccer, h2hbaskb, 0, h2hsoccer, h2hsoccer, h2hbaskb_state,  0, "Coleco", "Head to Head: Electronic Soccer (COP420L version)", MACHINE_SUPPORTS_SAVE )

CONS( 1981, einvaderc, einvader, 0, einvaderc, einvaderc, einvaderc_state, 0, "Entex", "Space Invader (Entex, COP444L version)", MACHINE_SUPPORTS_SAVE )

CONS( 1980, unkeinv,   0,        0, unkeinv,   unkeinv,   unkeinv_state,   0, "Gordon Barlow Design", "unknown electronic Space Invaders game (patent)", MACHINE_SUPPORTS_SAVE )

CONS( 1980, lchicken,  0,        0, lchicken,  lchicken,  lchicken_state,  0, "LJN", "I Took a Lickin' From a Chicken", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_MECHANICAL )

CONS( 1979, funjacks,  0,        0, funjacks,  funjacks,  funjacks_state,  0, "Mattel", "Funtronics: Jacks", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1979, funrlgl,   0,        0, funrlgl,   funrlgl,   funrlgl_state,   0, "Mattel", "Funtronics: Red Light Green Light", MACHINE_SUPPORTS_SAVE )
CONS( 1981, mdallas,   0,        0, mdallas,   mdallas,   mdallas_state,   0, "Mattel", "Dalla$ (J.R. handheld)", MACHINE_SUPPORTS_SAVE ) // ***

CONS( 1980, plus1,     0,        0, plus1,     plus1,     plus1_state,     0, "Milton Bradley", "Plus One", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_CONTROLS ) // ***
CONS( 1981, lightfgt,  0,        0, lightfgt,  lightfgt,  lightfgt_state,  0, "Milton Bradley", "Electronic Lightfight - The Games of Dueling Lights", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1982, bship82,   bship,    0, bship82,   bship82,   bship82_state,   0, "Milton Bradley", "Electronic Battleship (1982 version)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK ) // ***

CONS( 1978, qkracer,   0,        0, qkracer,   qkracer,   qkracer_state,   0, "National Semiconductor", "QuizKid Racer (COP420 version)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )

CONS( 1987, vidchal,   0,        0, vidchal,   vidchal,   vidchal_state,   0, "Select Merchandise", "Video Challenger", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )

// ***: As far as MAME is concerned, the game is emulated fine. But for it to be playable, it requires interaction
// with other, unemulatable, things eg. game board/pieces, playing cards, pen & paper, etc.