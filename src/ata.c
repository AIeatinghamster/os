#define ATA_DATA        0x1F0
#define ATA_ERROR       0x1F1
#define ATA_SECCOUNT0   0x1F2
#define ATA_LBA0        0x1F3
#define ATA_LBA1        0x1F4
#define ATA_LBA2        0x1F5
#define ATA_DRIVE       0x1F6
#define ATA_COMMAND     0x1F7
#define ATA_STATUS      0x1F7

#define ATA_CMD_READ    0x20
#define ATA_CMD_WRITE   0x30

#define ATA_SR_BSY      0x80
#define ATA_SR_DRQ      0x08

unsigned char inb(unsigned short port);
void outb(unsigned short port, unsigned char value);

static void ata_wait_bsy(void)
{
    while (inb(ATA_STATUS) & ATA_SR_BSY);
}

static void ata_wait_drq(void)
{
    while (!(inb(ATA_STATUS) & ATA_SR_DRQ));
}

void disk_write(unsigned int sector, unsigned char *buffer)
{
    ata_wait_bsy();

    outb(ATA_SECCOUNT0, 1);

    outb(ATA_LBA0, sector & 0xFF);
    outb(ATA_LBA1, (sector >> 8) & 0xFF);
    outb(ATA_LBA2, (sector >> 16) & 0xFF);

    outb(ATA_DRIVE, 0xE0 | ((sector >> 24) & 0x0F));

    outb(ATA_COMMAND, ATA_CMD_WRITE);

    ata_wait_bsy();
    ata_wait_drq();

    for (int i = 0; i < 256; i++) {
        unsigned short value =
            buffer[i * 2] |
            ((unsigned short)buffer[i * 2 + 1] << 8);

        __asm__ volatile (
            "outw %0, %1"
            :
            : "a"(value), "Nd"((unsigned short)ATA_DATA)
        );
    }

    ata_wait_bsy();
}
void disk_read(unsigned int sector, unsigned char *buffer)
{
    ata_wait_bsy();

    outb(ATA_SECCOUNT0, 1);

    outb(ATA_LBA0, sector & 0xFF);
    outb(ATA_LBA1, (sector >> 8) & 0xFF);
    outb(ATA_LBA2, (sector >> 16) & 0xFF);

    outb(ATA_DRIVE, 0xE0 | ((sector >> 24) & 0x0F));

    outb(ATA_COMMAND, ATA_CMD_READ);

    ata_wait_bsy();
    ata_wait_drq();

    for (int i = 0; i < 256; i++) {
        unsigned short value;

        __asm__ volatile (
            "inw %1, %0"
            : "=a"(value)
            : "Nd"((unsigned short)ATA_DATA)
        );

        buffer[i * 2]     = value & 0xFF;
        buffer[i * 2 + 1] = (value >> 8) & 0xFF;
    }
}
