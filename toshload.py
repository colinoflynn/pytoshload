
import serial
import struct

def lazyord(s):
    if isinstance(s, int):
        resb = s
    else:
        resb = ord(s)
    return resb    

class LowLevelBootloader(object):
    """TLCS900 low-level bootloader, based on at least the TLCS900L1 datasheets. May or
    may not work for others.
    """

    def __init__(self, comobject, reset_function, password=[0xFF]*12, reset_and_connect=True):
        """Initialize bootloader connection. Requires you pass at minimum a serial object
        (comobject), along with a function that when called will reset the target & enter
        the bootloader."""
        self.ser = comobject
        self.reset = reset_function
        self.password = password

        if reset_and_connect:
            self.reset()
            self.connect()

    def connect(self):
        """Send connection byte (0x86) to check if bootloader seems to be present"""
        self.flush()
        self.write(b"\x86")
        res = self.read(1)

        if lazyord(res[0]) != 0x86:
            raise IOError("Connect Fail - read: %s"%res)

    def calc_checksum(self, r):
        """Calculate checksum used on bootloader coms"""
        s = 0
        for c in r:
            s += c
        cs = 0x100 - (s & 0xff)
        return cs

    def write(self, data):
        self.ser.write(data)

    def read(self, len):
        return self.ser.read(len)

    def flush(self):
        self.ser.flush()

    def tx_rx(self, cmd, expectedlen=0, rxlen=-1):
        """Transmit data to the bootloader, and receive some back. Returns both a string & list version for convience"""

        if rxlen == -1:
            if expectedlen == 0:
                print("WARNING: Should call this with expectedlen or rxlen non-default")
                rxlen = 100
            rxlen = expectedlen
        self.write(cmd)
        response =self.read(rxlen)
        if expectedlen:
            if len(response) != expectedlen:
                raise IOError("Unexpected response length %d (data: %s)"%(len(response), str(response)))

        responsehex = [lazyord(c) for c in response]
        return response, responsehex

    def cmd_checkack(self, cmd, expected_extra_payload=0, expected_response=None):
        """Send a command to the bootloader, and check that the command was echo'd back as the normal ACK response.
        You can override the expected ACK to account for some commands with different ACKs."""

        bcmd = bytes(bytearray([cmd]))
        response, responsehex = self.tx_rx(bcmd, expected_extra_payload+1)

        if expected_response is None:
            expected_response = cmd

        if responsehex[0] != expected_response:
            raise IOError("Unexpected ACK = 0x%x"%responsehex[0])
        
        return response[1:], responsehex[1:]

    def cmd_productinfo(self):
        """Get product info (device name, protection status, and anything else encoded in there)"""

        response, responsehex = self.cmd_checkack(0x30, 62)
            
        pn = response[4:16]
        print(pn)

        if isinstance(response, str):
            response = bytes([ord(c) for c in response])

        pwaddr = struct.unpack("<I", response[16:20])
        print("PW Comparison Address: 0x%x"%pwaddr)

        ramaddr = struct.unpack("<I", bytes(response[20:24]))
        print("RAM Start Address: 0x%x"%ramaddr)

        ramendaddr = struct.unpack("<I", bytes(response[24:28]))
        print("RAM End Address: 0x%x"%ramendaddr)

        protection = responsehex[40]
        if protection == 0:
            print("Read: protected")
            print("Write: protected")
        elif protection == 1:
            print("Read: not protected")
            print("Write: protected")
        elif protection == 2:
            print("Read: protected")
            print("Write: not protected")
        elif protection == 3:
            print("Read: not protected")
            print("Write: not protected")
        else:
            raise IOError("Invalid protection byte")

        if self.calc_checksum(responsehex[0:61]) != responsehex[61]:
            raise IOError("Checksum error")
        
        return response

    def cmd_erase(self):
        """Perform chip erase, also clears FLASH protection"""

        self.cmd_checkack(0x40) #Erase command
        response, responsehex = self.cmd_checkack(0x54, 2) #Erase enable command, 2 seperate ACKs back

        if responsehex[0] != 0x4F:
            raise IOError("Error in erase")
        
        if responsehex[1] != 0x5D:
            raise IOError("Error in erase")

    def cmd_protect_set(self):
        """Set FLASH protection. Requires correct password."""

        self.cmd_checkack(0x60)
        cs = self.calc_checksum(self.password)
        self.write(bytes(bytearray(self.password)))
        response, responsehex = self.cmd_checkack(cs, 2, 0x60)

        if responsehex[0] != 0x6F:
            raise IOError("ACK Response to Protect Set Command: Error (%x)"%responsehex[0])

        if responsehex[1] != 0x31:
            raise IOError("ACK Response: Error (%x)"%responsehex[1])

    def cmd_ram_transfer(self, data, starting_address, skipcmd=False):        
        """Transfer RAM program. Requires correct password"""

        #If user already sent command byte, skip this
        if skipcmd == False:
            self.cmd_checkack(0x10)

        cs = self.calc_checksum(self.password)
        self.write(bytes(bytearray(self.password)))
        self.cmd_checkack(cs, 0, 0x10)

        cmddata = [(starting_address >> 24) & 0xff, (starting_address >> 16) & 0xff, (starting_address >> 8) & 0xff, starting_address & 0xff,
        (len(data) >> 8) & 0xff, len(data) & 0xff]

        cs = self.calc_checksum(cmddata)
        self.write(bytes(bytearray(cmddata)))        
        self.cmd_checkack(cs, 0, 0x10)

        cs = self.calc_checksum(data)
        self.write(bytes(bytearray(data)))        
        self.cmd_checkack(cs, 0, 0x10)

    def cmd_get_crc(self):

        response, responsehex = self.cmd_checkack(0x20, expected_extra_payload=3)
        cs = self.calc_checksum(responsehex[0:2])

        if cs != responsehex[2]:
            raise IOError("Expected checksum mismatch: %x != %x"%(cs, responsehex[2]))
        
        crc = (responsehex[0] << 8) | responsehex[1]
        return crc





class RamCodeProtocol(object):
    """Segger RAM Code bootloader communication class for TLCS900.

    This implements the protocol Segger uses for the TLCS900 devices. You
    must have used the low-level bootloader to previously perform the serial
    setup along with loading the RAMCode.
    """

    protSD1 = 0xEB
    protSD2 = 0xED
    protED = 0xEC

    CMD_ID = 0
    CMD_ERASE_SECTOR = 1
    CMD_PROGRAM = 2
    CMD_READ_BACK = 3
    CMD_GET_CRC = 4
    CMD_BLANK_CHECK = 5
    CMD_PROTECT_BLOCK = 6
    CMD_GET_SUM = 7

    CMD_NACK = 0xF1
    CMD_ACK = 0xF2
    CMD_ERROR = 0xF3

    ERROR_ERASE = 1
    ERROR_WRITE = 2
    ERROR_COMPARE = 3
    ERROR_BLANKCHECK = 4
    ERROR_NACK = 5
    ERROR_PROTECT = 6

    def __init__(self, ser):
        """Setup the connection, requires you pass a serial object"""
        self.ser = ser

    def write(self, data):
        """Serial write (pass-thru), attempts to detect if you pass just a single byte"""
        if hasattr(data, "len") is False:
            data = [data]
        self.ser.write(data)

    def read(self, len):
        """Serial read (pass-thru)"""
        return self.ser.read(len)

    def flush(self):
        """Serial flush (pass-thru)"""
        self.ser.flush()

    def u32_to_buffer(self, u32):
        """Packs u32 to buffer"""
        buf = [(u32 & 0xff), (u32 >> 8) & 0xff, (u32 >> 16) & 0xff, (u32 >> 24) & 0xff]
        return buf

    def u16_to_buffer(self, u16):
        """Packs u16 to buffer"""
        buf = [(u16 & 0xff), (u16 >> 8) & 0xff]
        return buf
       
    def sendPacket(self, payload):
        """Send a packet to the bootloader, adding required header"""
        self.write(self.protSD1)
        self.write(self.protSD2)

        lenLow = len(payload) & 0xFF
        lenHigh = len(payload) >> 8

        self.write(lenLow)
        self.write(lenHigh)

        checksum = lenLow + lenHigh

        for c in payload:
            checksum += c
            self.write(c)
        
        self.write(checksum & 0xff)

        self.write(self.protED)

    def rxExpect(self, expected):
        """Reads a single byte and compares it to an expected value"""
        c = self.read(1)
        if lazyord(c[0]) != expected:
            raise IOError("Sync Error - received %x (expected %x)"%(lazyord(c[0]), expected))

    def rxPacket(self):
        """Internal function, reads a packet and raises IOError if checksum is incorrect"""
        self.rxExpect(self.protSD1)
        self.rxExpect(self.protSD2)

        l = self.read(2)
        llsb = lazyord(l[0])
        lmsb = lazyord(l[1])

        checksum = llsb + lmsb

        plen = llsb + (lmsb << 8)

        if plen:
            payload = self.read(plen)
            for i in payload:
                checksum += lazyord(i)
        else:
            payload = []
        
        checksum = checksum & 0xff
        rxchecksum = lazyord(self.read(1)[0])

        self.rxExpect(self.protED)

        if rxchecksum != checksum:
            raise IOError("Checksum error: RX'd %x, expected %x"%(rxchecksum, checksum))
        
        return payload

    def cmd_id(self):
        """Read ID from the bootloader, just has some flags along with buffer size info"""
        self.sendPacket([self.CMD_ID])
        payload = self.rxPacket()

        cmdack = payload[0]

        flags = payload[1:5]
        flags = lazyord(flags[0]) + (lazyord(flags[1]) << 8) + (lazyord(flags[2]) << 16) + (lazyord(flags[3]) << 24)

        bufsize = payload[5:7]
        bufsize = lazyord(bufsize[0]) + (lazyord(bufsize[1]) << 8)

        return flags, bufsize

    def cmd_read(self, address, len):
        """Read flash memory, watch for the address remap that happens
        when booted in 'single boot mode', typically flash gets mapped
        around 0x10000 instead of 0xff0000."""        
        dleft = len
        readbackdata = ""

        while dleft > 0:
            if dleft > 0x100:
                dread = 0x100
            else:
                dread = dleft
            
            packet = [self.CMD_READ_BACK]
            packet.extend(self.u32_to_buffer(address))
            packet.extend(self.u16_to_buffer(dread))
            self.sendPacket(packet)

            response = self.rxPacket()

            readbackdata += response[7:]
            

            dleft -= dread
            address += dread
        
        return readbackdata


    def cmd_program(self, address, data):
        """Program flash memory, watch for the address remap that happens
        when booted in 'single boot mode', typically flash gets mapped
        around 0x10000 instead of 0xff0000."""

        dleft = len(data)
        dindex = 0

        while dleft > 0:
            if dleft > 0x100:
                dwrite = 0x100
            else:
                dwrite = dleft
            
            packet = [self.CMD_PROGRAM]
            packet.extend(self.u32_to_buffer(address))
            packet.extend(self.u16_to_buffer(dwrite))
            packet.extend(data[dindex:(dindex+dwrite)])
            self.sendPacket(packet)

            response = self.rxPacket()
            rc = lazyord(response[0])
            if rc != self.CMD_ACK:
                raise IOError("Response to program was %x (not ACK)"%rc)
            
            dleft -= dwrite
            address += dwrite
            dindex += dwrite


