#include <cstdint>

void kernel_main()
{
    uint32_t c_addr = get_arg_val<uint32_t>(0);
    uint32_t n_tiles = get_arg_val<uint32_t>(1);
    
    // The circular buffer that we are going to read from and write to DRAM
    constexpr uint32_t cb_out0 = tt::CB::c_out0;
    const uint32_t tile_size_bytes = get_tile_size(cb_out0);

    // Address generator for the output buffer. This is faster than doing plain DRAM writes.
    const InterleavedAddrGenFast<true> b = {
        .bank_base_address = c_addr,
        .page_size = tile_size_bytes,
        .data_format = DataFormat::Float16_b,
    };

    // Loop over all the tiles and write them to the output buffer
    for(uint32_t i = 0; i < n_tiles; i++)
    {
        // Make sure there is a tile in the circular buffer
        cb_wait_front(cb_out0, 1);
        uint32_t cb_out0_addr = get_read_ptr(cb_out0);
        // write the tile to DRAM
        noc_async_write_tile(i, b, cb_out0_addr);
        noc_async_write_barrier();
        // Mark the tile as consumed
        cb_pop_front(cb_out0, 1);
    }

}