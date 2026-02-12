----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 02/12/2026 04:14:30 PM
-- Design Name: 
-- Module Name: LED_COUNTER - Behavioral
-- Project Name: 
-- Target Devices: 
-- Tool Versions: 
-- Description: 
-- 
-- Dependencies: 
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
-- 
----------------------------------------------------------------------------------


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity LED_COUNTER is
    Port ( i_clk      : in STD_LOGIC;
           i_rst      : in STD_LOGIC;
           i_cnt_up_n : in STD_LOGIC;
           o_led      : out STD_LOGIC_VECTOR (1 downto 0));
end LED_COUNTER;

architecture Behavioral of LED_COUNTER is
  signal ro_led : std_logic_vector(1 downto 0) := "00";
  signal i_cnt_up          : std_logic;
  signal i_cnt_up_sync_0   : std_logic := '0';
  signal i_cnt_up_sync_1   : std_logic := '0';
  signal i_cnt_up_db       : std_logic := '0';
  signal cnt_up_d : std_logic := '0';
  signal db_count : integer range 0 to 50000 := 0;
begin

  i_cnt_up <= not i_cnt_up_n;

process (i_clk, i_rst)
begin
  if i_rst = '0' then
    ro_led <= "00";
    cnt_up_d <= '0';
    i_cnt_up_sync_0 <= '0';
    i_cnt_up_sync_1 <= '0';
    i_cnt_up_db <= '0';
    db_count <= 0;
  elsif rising_edge(i_clk) then
    -- Synchronize async input into i_clk domain first.
    i_cnt_up_sync_0 <= i_cnt_up;
    i_cnt_up_sync_1 <= i_cnt_up_sync_0;

    -- Debounce: accept new level only after it stays unchanged long enough.
    if i_cnt_up_sync_1 = i_cnt_up_db then
      db_count <= 0;
    else
      if db_count = 50000 then
        i_cnt_up_db <= i_cnt_up_sync_1;
        db_count <= 0;
      else
        db_count <= db_count + 1;
      end if;
    end if;

    cnt_up_d <= i_cnt_up_db;
    if (cnt_up_d = '0') and (i_cnt_up_db = '1') then
      case ro_led is
        when "00" => ro_led <= "01";
        when "01" => ro_led <= "10";
        when "10" => ro_led <= "11";
        when others => ro_led <= "00";
      end case;
    end if;
  end if;
end process;

o_led <= ro_led;

end Behavioral;
