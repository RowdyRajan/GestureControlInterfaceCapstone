library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity usb is
	port(

		--Clock and Reset
		clk : in std_logic;
		reset_n : in std_logic;

		-- USB Conduit interface to DE2 (Export)
		USB_DATA	: inout std_logic_vector(15 downto 0);
		USB_ADDR	: out std_logic_vector(1 downto 0);
		USB_WR_N	: out std_logic := '1';
		USB_RD_N	: out std_logic := '1';
		USB_RST_N	: out std_logic := '1';
		USB_CS_N	: out std_logic := '1';
		USB_INT0	: in std_logic; 	-- Irq 0 DC
		USB_INT1	: in std_logic; 	-- Irq 1 HC
	
		-- Avalon Memory-Mapped-Slave interface Device Controller (DC)
		avs_dc_address		: in std_logic;
		avs_dc_writedata	: in std_logic_vector(15 downto 0);
		avs_dc_write_n		: in std_logic;
		avs_dc_read_n		: in std_logic;
		avs_dc_CS_n		: in std_logic;
		avs_dc_readdata		: out std_logic_vector(15 downto 0);
		avs_dc_irq		: out std_logic;
		
	
		-- Avalon Memory-Mapped-Slave interface Host Controller (HC)
		-- Probably will not use this interface. 
		avs_hc_address		: in std_logic;
		avs_hc_writedata	: in std_logic_vector(15 downto 0);
		avs_hc_write_n		: in std_logic;
		avs_hc_read_n		: in std_logic;
		avs_hc_CS_n		: in std_logic;
		avs_hc_readdata		: out std_logic_vector(15 downto 0);
		avs_hc_irq		: out std_logic
	
	);
end usb;
	
architecture connections of usb is

	begin
	-- Send interrupt from DE2 connection to proper controller
	avs_dc_irq <= USB_INT0;
	avs_hc_irq <= USB_INT1;
	
	-- Two cases possible, using the host controller or the device controller. 
	-- Currently this does not full function for the Host Controller (HC) but we 
	-- do not need it for our project. I do intend to make the architecture generalized later
	
	--Device controller signals
	USB_DATA <= avs_dc_writedata when avs_dc_write_n = '0' else (others => 'Z'); -- Only does device controller
	avs_dc_readdata <= USB_DATA when avs_dc_read_n = '0' else (others => 'Z');
	avs_hc_readdata <= USB_DATA when avs_hc_read_n = '0' else (others => 'Z');
	USB_CS_N <= '1' when avs_dc_CS_n = '0' and avs_hc_CS_n = '0' else '0';
	USB_ADDR(0) <= '1';
	USB_ADDR(1) <= avs_dc_address;
	USB_RD_N <= avs_dc_read_n; --Just Ignoring the HC controller right now.
	USB_WR_N <= avs_dc_write_n;
	USB_RST_N <= reset_n;
	
end architecture connections;
	

	
	-- If chip_select_n == 1
		-- I could probably have processes for chip select for toggling between HC and DC
		-- but for now i'm less than interested when I havent gotten DC working















