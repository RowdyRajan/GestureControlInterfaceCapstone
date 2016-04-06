library ieee;

use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.VITAL_Primitives.all;
use work.DE2_CONSTANTS.all;
	
entity DE2Component is
	
	port (
		KEY				: in  std_logic_vector (3 downto 0); --Buttons
		SW					: in 	std_logic_vector (7 downto 0); -- Switches
		CLOCK_50			: in  std_logic; -- 50 MHz Clock
		LEDG				: out DE2_LED_GREEN; -- Green LEDs
		--DRAM_ADDR	:	out 	std_logic_vector (11 downto 0);
		DRAM_ADDR		:	out	DE2_SDRAM_ADDR_BUS;
		DRAM_BA_0		: 	out	std_logic;
		DRAM_BA_1		:	out	std_logic;
		DRAM_CAS_N		:	out	std_logic;
		DRAM_CKE			:	out	std_logic;
		DRAM_CLK			:	out	std_logic;
		DRAM_CS_N		:	out	std_logic;
		--DRAM_DQ		:	inout std_logic_vector (15 downto 0);
		DRAM_DQ			:	inout 	DE2_SDRAM_DATA_BUS;
		DRAM_LDQM		: 	out	std_logic;
		DRAM_UDQM		: 	out	std_logic;
		DRAM_RAS_N		: 	out	std_logic;
		DRAM_WE_N		: 	out 	std_logic;
		-- SRAM on board
		SRAM_ADDR		:	out	DE2_SRAM_ADDR_BUS;
		SRAM_DQ			:	inout DE2_SRAM_DATA_BUS;
		SRAM_WE_N		:	out	std_logic;
		SRAM_OE_N		:	out	std_logic;
		SRAM_UB_N		:	out 	std_logic;
		SRAM_LB_N		:	out	std_logic;
		SRAM_CE_N		:	out	std_logic;
		
		--UART Connection
		UART_TXD : out std_logic; --Transmitter
		UART_RXD : in std_logic;  --Receiver
		
		-- USB controller
		OTG_INT0			: in	std_logic;
		OTG_INT1			: in	std_logic;
		OTG_DREQ0		: in	std_logic;
		OTG_DREQ1		: in	std_logic;
		OTG_DACK0_N		: out	std_logic;
		OTG_DACK1_N		: out	std_logic;
		OTG_FSPEED		: out	std_logic;
		OTG_LSPEED		: out	std_logic;
		OTG_ADDR			: out	std_logic_vector(1 downto 0);
		OTG_DATA			: inout	std_logic_vector(15 downto 0);
		OTG_CS_N			: out	std_logic;
		OTG_RD_N			: out	std_logic;
		OTG_WR_N			: out	std_logic;
		OTG_RST_N		: out	std_logic
	);

end DE2Component;

architecture structure of DE2Component is
	
	component niosII_system is
			port (
            clk_clk                                : in    	std_logic := 'X'; -- clk
            reset_reset_n                          : in    	std_logic := 'X'; -- reset_n
				green_leds_external_connection_export  : out   	DE2_LED_GREEN;                  	   	-- export
				switches_external_connection_export    : in    	std_logic := 'X';  							-- export
            sdram_0_wire_addr                      : out   	DE2_SDRAM_ADDR_BUS;                    -- addr
            sdram_0_wire_ba                        : out   	std_logic_vector(1 downto 0);          -- ba
            sdram_0_wire_cas_n                     : out   	std_logic;                             -- cas_n
            sdram_0_wire_cke                       : out   	std_logic;                             -- cke
            sdram_0_wire_cs_n                      : out   	std_logic;                             -- cs_n
            sdram_0_wire_dq                        : inout 	DE2_SDRAM_DATA_BUS := (others => 'X'); -- dq
            sdram_0_wire_dqm                       : out   	std_logic_vector(1 downto 0);          -- dqm
            sdram_0_wire_ras_n                     : out   	std_logic;                             -- ras_n
            sdram_0_wire_we_n                      : out   	std_logic;                             -- we_n
				sram_0_external_interface_DQ           : inout 	DE2_SRAM_DATA_BUS := (others => 'X'); 	-- DQ
            sram_0_external_interface_ADDR         : out   	DE2_SRAM_ADDR_BUS;                    	-- ADDR
            sram_0_external_interface_LB_N         : out   	std_logic;                             -- LB_N
            sram_0_external_interface_UB_N         : out   	std_logic;                            	-- UB_N
            sram_0_external_interface_CE_N         : out   	std_logic;                             -- CE_N
            sram_0_external_interface_OE_N         : out   	std_logic;                             -- OE_N
            sram_0_external_interface_WE_N         : out		std_logic;                             -- WE_N
				altpll_0_c0_clk                        : out   	std_logic;                             -- clk
				usb_0_external_interface_INT1				: in		std_logic	:= 'X'; -- Interrupt1
				usb_0_external_interface_INT0				: in 		std_logic	:= 'X'; -- Interrupt2
				usb_0_external_interface_DATA				: inout	std_logic_vector(15 downto 0) := (others => 'X'); -- DATA
				usb_0_external_interface_RST_N			: out		std_logic; -- reset_n
            usb_0_external_interface_ADDR				: out		std_logic_vector(1 downto 0); -- ADDR
				usb_0_external_interface_CS_N				: out		std_logic; -- CS_N
				usb_0_external_interface_RD_N				: out		std_logic; -- RD_N
				usb_0_external_interface_WR_N				: out		std_logic; -- WR_N
				rs232_0_external_interface_RXD        	: in    	std_logic;   --:= 'X';             -- RXD
				rs232_0_external_interface_TXD        	: out   	std_logic                        -- TXD
				
        );
	end component niosII_system;

	--	These signals are for matching the provided IP core to
	-- The specific SDRAM chip in our system	 
	signal BA	: std_logic_vector (1 downto 0);
	signal DQM	:	std_logic_vector (1 downto 0);
	 
	begin
	-- DACK not used
	OTG_DACK0_N <= 'Z';
	OTG_DACK1_N <= 'Z';
	
	-- Speed set to Full-speed
	OTG_FSPEED <= 'Z';
	OTG_LSPEED <= '0';
	
	--Memory related signal settings. Appears to be used for combining two signals into a singal interface.
	DRAM_BA_1 <= BA(1);
	DRAM_BA_0 <= BA(0);
	
	DRAM_UDQM <= DQM(1);
	DRAM_LDQM <= DQM(0);
	
	-- Component Instantiation Statement (optional)
	u0 : component niosII_system
      port map (
         clk_clk                                => CLOCK_50,                                
         reset_reset_n                          => KEY(0),                          
         sdram_0_wire_addr                      => DRAM_ADDR,                      
         sdram_0_wire_ba                        => BA,                        
         sdram_0_wire_cas_n                     => DRAM_CAS_N,                      
         sdram_0_wire_cke                       => DRAM_CKE,                       
         sdram_0_wire_cs_n                      => DRAM_CS_N,                      
         sdram_0_wire_dq                        => DRAM_DQ,                         
         sdram_0_wire_dqm                       => DQM,                        
         sdram_0_wire_ras_n                     => DRAM_RAS_N,                     
         sdram_0_wire_we_n                      => DRAM_WE_N,                       
          
			altpll_0_c0_clk                        => DRAM_CLK,                        
          
			green_leds_external_connection_export  => LEDG,  
         switches_external_connection_export    => SW(0),       
          
			sram_0_external_interface_DQ           => SRAM_DQ,           
         sram_0_external_interface_ADDR         => SRAM_ADDR,          
         sram_0_external_interface_LB_N         => SRAM_LB_N,         
         sram_0_external_interface_UB_N         => SRAM_UB_N,          
         sram_0_external_interface_CE_N         => SRAM_CE_N,         
         sram_0_external_interface_OE_N         => SRAM_OE_N,         
         sram_0_external_interface_WE_N         => SRAM_WE_N,
			 
			usb_0_external_interface_INT1				=> OTG_INT1,
			usb_0_external_interface_INT0				=> OTG_INT0,
			usb_0_external_interface_DATA				=> OTG_DATA,
			usb_0_external_interface_RST_N			=> OTG_RST_N,
			usb_0_external_interface_ADDR				=> OTG_ADDR,
			usb_0_external_interface_CS_N				=> OTG_CS_N,
			usb_0_external_interface_RD_N				=> OTG_RD_N,
			usb_0_external_interface_WR_N				=> OTG_WR_N,
			
			rs232_0_external_interface_RXD			=> UART_RXD,
			rs232_0_external_interface_TXD			=> UART_TXD
      );

end structure;

library ieee;
	use ieee.std_logic_1164.all;
	
package DE2_CONSTANTS is
	
	type DE2_SDRAM_ADDR_BUS is array(11 downto 0) of std_logic;
	type DE2_SDRAM_DATA_BUS is array(15 downto 0) of std_logic;
	
	type DE2_LCD_DATA_BUS	is array(7 downto 0) of std_logic;
	
	type DE2_LED_GREEN		is array(7 downto 0)  of std_logic;
	
	type DE2_SRAM_ADDR_BUS	is array(17 downto 0) of std_logic;
	type DE2_SRAM_DATA_BUS  is array(15 downto 0) of std_logic;
	
end DE2_CONSTANTS;

