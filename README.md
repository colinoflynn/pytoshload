# PyToshLoad for Toshiba TLCS Processors.

This repo contains a simple Python Implementation of Segger ToshLoad program for Toshiba TLCS900. Primarily this uses the existing RAMCode written by Segger, which is available in the `segger` directory.

Note the RAMCode is under a seperate license from Segger and not part of the open-source license.


## Usage Example

With serial port:

```
def reset_target():
    # If you had control of nrst do this here,
    # otherwise do it manually
    set_boot_pin_low()
    gpio_toggle_nrst()

import serial
ser = serial.Serial('COM4', 9600, timeout=0.5)

bl = tl.LowLevelBootloader(ser, reset_target)
bl.cmd_productinfo()

bl.cmd_ram_transfer(rc.B_F16_RAM1000_ROM10000_TLCS900L1["data"], rc.B_F16_RAM1000_ROM10000_TLCS900L1["start_address"])
rl = tl.RamCodeProtocol(target.ser)
time.sleep(0.01)

```

This just gets you the ramloader. You could for example then read the flash out:

```
flash = rl.cmd_read(0x10000, 0x20000)
data = open("flash.bin", "wb")
data.write(flash)
data.close()
```

For this to work, you need the password. If you erase the device first the password becomes the default one (`ff` 12 times). So
for example to erase & flash a device, using a serial port where the `DTR` line is connected to BOOT pin, and `RTS` connected
to the reset line:

```
import pytoshload.ramcode as rc
import pytoshload.toshload as tl
import time
import serial

ser = serial.Serial('COM4', 9600, timeout=0.5)

def reset_function(boot_mode=True):
    #RTS/DTR logic levels "backwards" - watch out!!
    ser.rts = True
    if boot_mode:
        ser.dtr = True
    else:
        ser.dtr = False
    time.sleep(0.1)
    ser.rts = False
    time.sleep(1)


bl = tl.LowLevelBootloader(ser, reset_function)
print(bl.cmd_productinfo())
bl.cmd_erase()


f = open("firmware_file.bin", "rb")
data = f.read()
f.close()

bl = tl.LowLevelBootloader(ser, reset_function)
print(bl.cmd_productinfo())

rd = rc.B_F16_RAM1000_ROM10000_TLCS900L1["data"]

# Hack to deal with TMP91FW60, set to True
device_is_tmp91fw60 = False

if device_is_tmp91fw60:
    rd[0x23] = 0xA2 # INTES1
    rd[0x37] = 0xA2 # INTES1
    rd[0x2E] = 0x2D # MicroDMA for INTTX1

bl.cmd_ram_transfer(rd, rc.B_F16_RAM1000_ROM10000_TLCS900L1["start_address"])
rl = tl.RamCodeProtocol(ser)

time.sleep(0.1)

print(rl.cmd_id())
rl.cmd_program(0x10000, data)

#Reset normal mode
reset_function(False)
```

## Using with TMP91FW60DFG & Other TLCS900L1 Devices

The firmware `B_F16_RAM1000_ROM10000_TLCS900L1` is compiled with some common settings for the serial port. In particular you
may need to modify:

* Set INTES1 address (in B_F16_RAM1000_ROM10000_TLCS900L1 assumed to be `0x9D`)
* INTTX1 Micro DMA start vector (in B_F16_RAM1000_ROM10000_TLCS900L1 assumed to be `0x24`)

These values are *incorrect* for the TMP91FW60DFG, and possibly other devices. This can be fixed by patching the address as shown
here:

```
rd = rc.B_F16_RAM1000_ROM10000_TLCS900L1["data"][:]
rd[0x23] = 0xA2 # INTES1
rd[0x37] = 0xA2 # INTES1
rd[0x2E] = 0x2D # MicroDMA for INTTX1
bl.cmd_ram_transfer(rd, rc.B_F16_RAM1000_ROM10000_TLCS900L1["start_address"])
```

Note the indexes there are valid only for `B_F16_RAM1000_ROM10000_TLCS900L1`.


Note that the TMP91FW60DFG claims to use the `B_F16_RAM1000_ROM10000_TLCS900L1` firmware from the Segger config file, but it won't
work as-is. It doesn't seem to work with ToshLoad itself either.

You can see the actual details in the `system.c` file. Note that there are some other address you could need to modify, but it seems
that only the above ones are the actual important ones.