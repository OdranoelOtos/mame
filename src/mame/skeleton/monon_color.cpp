// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************
   Monon Color - Chinese handheld

   see
   https://gbatemp.net/threads/the-monon-color-a-new-video-game-console-from-china.467788/
   https://twitter.com/Splatoon2weird/status/1072182093206052864

   uses AX208 CPU (custom 8051 @ 96Mhz with single cycle instructions, extended '16 bit' opcodes + integrated video, jpeg decoder etc.)
   https://docplayer.net/52724058-Ax208-product-specification.html

***************************************************************************/

#include "emu.h"
#include "cpu/axc51/axc51.h"
#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

class monon_color_state : public driver_device
{
public:
	monon_color_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_cart(*this, "cartslot"),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_mainram(*this, "mainram")
	{ }

	void monon_color(machine_config &config);
protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
private:
	required_device<generic_slot_device> m_cart;
	required_device<ax208_cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_mainram;

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	void monon_color_map(address_map &map);
	void monon_color_io_map(address_map &map);
};

void monon_color_state::machine_start()
{
}



void monon_color_state::machine_reset()
{
	m_maincpu->set_spi_ptr(memregion("flash")->base(), memregion("flash")->bytes() );


	/*  block starting at e000 in flash is not code? (or encrypted?)
	    no code to map at 0x9000 in address space (possible BIOS?)
	    no code in flash ROM past the first 64kb(?) which is basically the same on all games, must be some kind of script interpreter? J2ME maybe?

	    there are 5 different 'versions' of the code in the dumped ROMs, where the code is the same the roms match up until 0x50000 after which the game specific data starts

	    by game number:

	    103alt                           (earliest? doesn't have bank9)
	    101,102,103,104,105              (1st revision)
	    106,107                          (2nd revision)
	    201                              (3rd revision)
	    202,203,204,205,301,302,303,304  (4th revision)
	*/
}

void monon_color_state::video_start()
{
}


uint32_t monon_color_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static INPUT_PORTS_START( monon_color )
INPUT_PORTS_END

void monon_color_state::monon_color_map(address_map &map)
{
	map(0x4000, 0x6fff).ram().share("mainram");
//	map(0x9000, 0x9fff).rom() // internal to CPU
}


void monon_color_state::monon_color_io_map(address_map& map)
{
	map(0x0000, 0x00ff).ram();

	map(0x4000, 0x6fff).ram().share("mainram");
}

void monon_color_state::monon_color(machine_config &config)
{
	/* basic machine hardware */
	AX208(config, m_maincpu, 96000000); // (8051 / MCS51 derived) incomplete core!
	m_maincpu->set_addrmap(AS_PROGRAM, &monon_color_state::monon_color_map);
	m_maincpu->set_addrmap(AS_IO, &monon_color_state::monon_color_io_map);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_screen_update(FUNC(monon_color_state::screen_update));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_entries(256);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	generic_cartslot_device &cartslot(GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "monon_color_cart", "bin"));
	cartslot.set_width(GENERIC_ROM8_WIDTH);
	cartslot.set_device_load(FUNC(monon_color_state::cart_load));
	cartslot.set_must_be_loaded(true);

	/* software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("monon_color");
}

DEVICE_IMAGE_LOAD_MEMBER( monon_color_state::cart_load )
{
	uint32_t size = m_cart->common_get_size("rom");
	std::vector<uint8_t> temp;
	temp.resize(size);
	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(&temp[0], size, "rom");

	memcpy(memregion("flash")->base(), &temp[0], size);

	return image_init_result::PASS;
}

ROM_START( mononcol )
	ROM_REGION( 0x1000000, "flash", ROMREGION_ERASE00 )
ROM_END

CONS( 2011, mononcol,    0,          0,  monon_color,  monon_color,    monon_color_state, empty_init,    "M&D",   "Monon Color", MACHINE_IS_SKELETON )
