
#pragma once

#include <hw/pci_device.hpp>

namespace usb {

  class XHCI {
  public:

    XHCI(hw::PCI_Device& dev);

  private:
    hw::PCI_Device& pci_;
    uintptr_t bar0_ = 0;
    uint32_t  op_offs_ = 0;
    // capabilities
    uint8_t max_ports_;
    uint8_t max_slots_;

    template <typename type = uint32_t>
    inline type read_cmd(uint32_t offset);
    template <typename type = uint32_t>
    inline void write_cmd(uint32_t offset, type value);

    inline uint32_t read_op(uint32_t offset);
    inline void     write_op(uint32_t offset, uint32_t val);
  };

  template <typename type>
  inline type XHCI::read_cmd(uint32_t offset)
  {
    return *(type volatile*)(this->bar0_ + offset);
  }

  template <typename type>
  inline void XHCI::write_cmd(uint32_t offset, type val)
  {
    *(type volatile*)(this->bar0_ + offset) = val;
  }

  inline uint32_t XHCI::read_op(uint32_t offset)
  {
    return read_cmd(op_offs_ + offset);
  }

  inline void XHCI::write_op(uint32_t offset, uint32_t val)
  {
    write_cmd(op_offs_ + offset, val);
  }


}
