#include "main.h"
#include "mazogs.h"
#include "button.h"
#include "rampack.pio.h"

#include <hardware/dma.h>
#include <pico/stdlib.h>
#include <stdio.h>
#include <tusb.h>

const uint sm0 = 0;
const uint sm1 = 1;

uint8_t write[4];
uint8_t ram[16384]  __attribute__((aligned(16384)));

Button btn(gpio_btn);

int main() {
  stdio_init_all();
  //while (!tud_cdc_connected()) { sleep_ms(100); }

  uint offset = pio_add_program(pio0, &rampack_wr_program);
  rampack_wr_program_init(pio0, sm0, offset);
  pio_sm_set_enabled(pio0, sm0, true);

  offset = pio_add_program(pio0, &rampack_rd_program);
  rampack_rd_program_init(pio0, sm1, offset);
  pio_sm_set_enabled(pio0, sm1, true);

  int wr_addr_chan = dma_claim_unused_channel(true);
  int wr_data_chan = dma_claim_unused_channel(true);
  int wr_write_chan = dma_claim_unused_channel(true);
  int rd_addr_chan = dma_claim_unused_channel(true);
  int rd_data_chan = dma_claim_unused_channel(true);

  // WR ADDRESS Channel
  dma_channel_config c = dma_channel_get_default_config(wr_addr_chan);
  channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
  channel_config_set_read_increment(&c, false);
  channel_config_set_write_increment(&c, false);
  channel_config_set_dreq(&c, pio_get_dreq(pio0, sm0, false));
  channel_config_set_chain_to(&c, wr_data_chan);
  channel_config_set_high_priority(&c, true);
  dma_channel_set_config(wr_addr_chan, &c, false);
  dma_channel_set_trans_count(wr_addr_chan, 1, false);
  dma_channel_set_read_addr(wr_addr_chan, &pio0->rxf[sm0], false);
  dma_channel_set_write_addr(wr_addr_chan, &dma_hw->ch[wr_write_chan].write_addr, false);

  // WR DATA Channel
  c = dma_channel_get_default_config(wr_data_chan);
  channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
  channel_config_set_read_increment(&c, false);
  channel_config_set_write_increment(&c, false);
  channel_config_set_dreq(&c, pio_get_dreq(pio0, sm0, false));
  channel_config_set_chain_to(&c, wr_write_chan);
  channel_config_set_high_priority(&c, true);
  dma_channel_set_config(wr_data_chan, &c, false);
  dma_channel_set_trans_count(wr_data_chan, 1, false);
  dma_channel_set_read_addr(wr_data_chan, &pio0->rxf[sm0], false);
  dma_channel_set_write_addr(wr_data_chan, &write[0], false);

  // WR WRITE Channel
  c = dma_channel_get_default_config(wr_write_chan);
  channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
  channel_config_set_read_increment(&c, false);
  channel_config_set_write_increment(&c, false);
  channel_config_set_chain_to(&c, wr_addr_chan);
  channel_config_set_high_priority(&c, true);
  dma_channel_set_config(wr_write_chan, &c, false);
  dma_channel_set_trans_count(wr_write_chan, 1, false);
  dma_channel_set_read_addr(wr_write_chan, &write[3], false);

  // RD ADDRESS Channel
  c = dma_channel_get_default_config(rd_addr_chan);
  channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
  channel_config_set_read_increment(&c, false);
  channel_config_set_write_increment(&c, false);
  channel_config_set_dreq(&c, pio_get_dreq(pio0, sm1, false));
  channel_config_set_chain_to(&c, rd_data_chan);
  channel_config_set_high_priority(&c, true);
  dma_channel_set_config(rd_addr_chan, &c, false);
  dma_channel_set_trans_count(rd_addr_chan, 1, false);
  dma_channel_set_read_addr(rd_addr_chan, &pio0->rxf[sm1], false);
  dma_channel_set_write_addr(rd_addr_chan, &dma_hw->ch[rd_data_chan].read_addr, false);

  // RD DATA Channel
  c = dma_channel_get_default_config(rd_data_chan);
  channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
  channel_config_set_read_increment(&c, false);
  channel_config_set_write_increment(&c, false);
  channel_config_set_chain_to(&c, rd_addr_chan);
  channel_config_set_high_priority(&c, true);
  dma_channel_set_config(rd_data_chan, &c, false);
  dma_channel_set_trans_count(rd_data_chan, 1, false);
  dma_channel_set_write_addr(rd_data_chan, &pio0->txf[sm1], false);

  dma_start_channel_mask((1u << wr_addr_chan) | (1u << rd_addr_chan));

  pio_sm_put_blocking(pio0, sm0, ((uint32_t)(ram) >> 14)); // Wr
  pio_sm_put_blocking(pio0, sm1, ((uint32_t)(ram) >> 14)); // Rd

  while (1)
  {
    if(btn.Read() == Button::State::Rising)
    {
      memcpy(&ram[9], MAZOGS, sizeof(MAZOGS));
    }
  
    sleep_us(1);
  }
}