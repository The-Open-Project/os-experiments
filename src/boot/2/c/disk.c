// for disk-related stuff in stdio.h
#include "include/stdio.h"
#include <logging.h>
#include <stdbool.h>
#include <stdint.h>

extern uint8_t _inb(uint16_t port);
extern void _outb(uint16_t port, uint8_t data);

extern bool _get_drive_info(uint8_t drive, uint8_t *type, uint16_t *cylinders,
                            uint16_t *sectors, uint16_t *heads);

extern bool _read_drive_sectors(uint8_t drive, uint16_t cylinder,
                                uint16_t sector, uint16_t head, uint16_t count,
                                uint8_t *buffer);

extern bool _reset_drive(uint8_t drive);

// TODO: implement correct floppy disk driver
// #define _FLOPPY_CTLR_BASE_ADDR 0x3F0
// #define _FLOPPY_IRQ 6
//
// static const char *_drive_types[8] = {
//     "none",         "360kB 5.25\"", "1.2MB 5.25\"", "720kB 3.5\"",
//
//     "2.88MB 3.5\"", "1.44MB 3.5\"", "unknown type", "unknown type"};
//
// typedef enum _FLOPPY_REGS { DOR = 2, MSR = 4, FIFO = 5, CCR = 7 }
// FLOPPY_REGS;
//
// typedef enum _FLOPPY_CMDS {
//   SPECIFY = 0x03,
//   WRITE = 0x05,
//   READ = 0x06,
//   RECALIBRATE = 0x07,
//   SENSE_INTERRUPT = 0x08,
//   SEEK = 0x0F,
// } FLOPPY_CMDS;
//
// static void detect_floppy() {
//   _outb(0x70, 0x10);
//   uint8_t drives = _inb(0x71);
//
//   plog(INFO_STREAM, "Floppy 0 status: %s\r\n", _drive_types[drives >> 4]);
// }

struct _FILE {
  bool isOpen;
};

static void __lba2chs(DISK *disk, uint32_t lba, uint16_t *cylinder,
                      uint16_t *head, uint16_t *sector) {
  *cylinder = lba / (disk->heads * disk->sectors);
  *head = (lba / disk->sectors) % disk->heads;
  *sector = lba % disk->sectors + 1;
}

static void __diskInit(DISK *disk, uint8_t number) {
  plog(INFO_STREAM, "Initializing disk %hhu\r\n", number);

  uint8_t type;
  uint16_t cylinders;
  uint16_t heads;
  uint16_t sectors;

  if (!_get_drive_info(number, &type, &cylinders, &heads, &sectors)) {
    panic("Failed to get drive info for disk %d\r\n", number);
    return;
  }

  disk->number = number;
  disk->cylinders = cylinders;
  disk->heads = heads;
  disk->sectors = sectors;

  plog(INFO_STREAM, "Disk %hhu: %hu cylinders, %hu heads, %hu sectors\r\n",
       number, cylinders, heads, sectors);
}

static void __diskRead(DISK *disk, uint32_t lba, uint8_t count,
                       uint8_t *buffer) {
  plog(INFO_STREAM, "Reading %hu sectors of disk %hhu\r\n", count,
       disk->number);

  uint16_t cylinder, head, sector;

  __lba2chs(disk, lba, &cylinder, &head, &sector);

  for (int i = 0; i < 3; i++) {
    if (_read_drive_sectors(disk->number, cylinder, sector, head, count,
                            buffer)) {
      plog(INFO_STREAM, "Read %hu sector/s of disk %hhu\r\n", count,
           disk->number);
      return;
    }

    plog(INFO_STREAM, "Failed to read disk %hhu, retrying...\r\n",
         disk->number);
    if (!_reset_drive(disk->number)) {
      panic("Failed to reset disk %d\r\n", disk->number);
      return;
    }
  }

  panic("Failed to read disk %d\r\n", disk->number);
}

void __priv__diskInit(uint8_t drive, DISK *disk) { __diskInit(disk, drive); }

void __priv__diskRead(DISK *disk, uint32_t lba, uint8_t count,
                      uint8_t *buffer) {
  __diskRead(disk, lba, count, buffer);
}
