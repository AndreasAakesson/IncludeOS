

#include <usb/xhci.hpp>
#include <timers>
#define XHCI_DEBUG

#ifdef XHCI_DEBUG
#define DBG(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define DBG(fmt, ...) /* fmt */
#endif

// CAPABILITIES
inline static constexpr uint32_t CAPLENGTH    = 0x00;
inline static constexpr uint32_t HCIVERSION   = 0x02;
inline static constexpr uint32_t HCSPARAMS1   = 0x04;
inline static constexpr uint32_t HCSPARAMS2   = 0x08;
inline static constexpr uint32_t HCSPARAMS3   = 0x0C;
inline static constexpr uint32_t HCCPARAMS1   = 0x10;
inline static constexpr uint32_t DBOFF        = 0x14;
inline static constexpr uint32_t RTSOFF       = 0x18;
// OPERATIONAL
inline static constexpr uint32_t USBCMD       = 0x00;
inline static constexpr uint32_t USBSTS       = 0x04;
inline static constexpr uint32_t PGSIZE       = 0x08;
inline static constexpr uint32_t DNCTRL       = 0x14;
inline static constexpr uint32_t CRCR         = 0x18;
inline static constexpr uint32_t DCBAAP       = 0x30;
inline static constexpr uint32_t CONFIG       = 0x38;
inline static constexpr uint32_t PORT_REG_START       = 0x400;

#define CMD_RUN       (1 << 0)
#define CMD_HCRST     (1 << 1)  // Host Controller Reset
#define CMD_INTE      (1 << 2)  // IRQ Enable
#define CMD_HSEE      (1 << 3)  // Host System Error En
#define CMD_LHCRST      (1 << 7)  // Light Host Controller Reset
#define CMD_CSS       (1 << 8)  // Controller Save State
#define CMD_CRS       (1 << 9)  // Controller Restore State
#define CMD_EWE       (1 << 10) // Enable Wrap Event

namespace usb {

  XHCI::XHCI(hw::PCI_Device& dev)
    : pci_{dev}, bar0_{0}
  {
    INFO("XHCI", "Initializing with PCI @ %#x: %s %#x",
      pci_.pci_addr(), pci_.to_string().c_str(), pci_.subclass());
    INFO2("Revision ID %#x", pci_.rev_id());
    INFO2("| Probing resources");
    pci_.parse_capabilities();
    pci_.probe_resources();

    bar0_ = pci_.get_bar(0).start;
    op_offs_ = read_cmd(CAPLENGTH);
    uintptr_t opreg = bar0_ + op_offs_;
    INFO2("Operational Register Space start at %p", (void*)opreg);
    /*union version_helper {
      uint16_t ver;
      struct
      {
        uint8_t major;
        uint8_t minor;
      };
    };
    version_helper version{(uint16_t)(read_cmd(0x02) >> 16)};
    INFO2("Version %u.%u %#x", version.major, version.minor, version.ver);*/

    uint16_t version = read_cmd<uint16_t>(HCIVERSION);
    INFO2("Version %hu %#hx", version, version);

    union hcsparams1 {
      uint32_t whole;
      struct {
        uint8_t   max_slots;
        uint16_t  max_intrs;
        uint8_t   max_ports;
      }__attribute__((packed));
    };

    //auto sparams1 = read_cmd(HCSPARAMS1);
    //DBG("structual paramas 1 0x%08x\n", sparams1);
    //uint8_t slots = sparams1 & 0xff;
    //uint8_t ports = (sparams1 >> 24) & 0xff;
    //DBG("slots=%u ports=%u\n", slots, ports);

    hcsparams1 params1{read_cmd(HCSPARAMS1)};
    INFO2("Scanning capabilities");
    max_ports_ = params1.max_ports;
    max_slots_ = params1.max_slots;
    INFO2("|-- Ports:%u Slots:%u",
      max_ports_, max_slots_);

    DBG("structual paramas 2 0x%08x\n", read_cmd(HCSPARAMS2));
    DBG("structual paramas 3 0x%08x\n", read_cmd(HCSPARAMS3));
    DBG("capabilities paramas 1 0x%08x\n", read_cmd(HCCPARAMS1));

    auto db_offs = read_cmd(DBOFF) & ~0x03;
    INFO2("Doorbell offset %u", db_offs);
    auto rts_offs = read_cmd(RTSOFF) & ~0x1F;
    INFO2("Runtime Register Space offset %u", rts_offs);

    INFO2("Operational register");
    DBG("USBCMD 0x%08x\n", read_op(USBCMD));
    DBG("USBSTS 0x%08x\n", read_op(USBSTS));

    write_op(USBCMD, read_op(USBCMD) & ~CMD_RUN);
    DBG("USBCMD 0x%08x\n", read_op(USBCMD));
    DBG("USBSTS 0x%08x\n", read_op(USBSTS));

    write_op(USBCMD, read_op(USBCMD) | CMD_HCRST);

    write_op(USBCMD, read_op(USBCMD) | CMD_RUN);
    Timers::oneshot(std::chrono::seconds(3), [this](auto){
      DBG("USBSTS 0x%08x\n", read_op(USBSTS));
    });

  }

}
