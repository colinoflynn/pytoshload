
import serial
import struct

class LowLevelBootloader(object):

    def __init__(self, comobject, reset_function, password=[0xFF]*12):
        self.ser = comobject
        self.reset = reset_function
        self.password = password
        self.reset()
        self.connect()

    def connect(self):
        self.flush()
        self.write(b"\x86")
        res = self.read(1)

        if ord(res[0]) != 0x86:
            raise IOError("Connect Fail - read: %s"%res)

    def calc_checksum(self, r):
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

        responsehex = [ord(c) for c in response]
        return response, responsehex

    def cmd_checkack(self, cmd, expected_extra_payload=0, expected_response=None):
        bcmd = bytes(bytearray([cmd]))
        response, responsehex = self.tx_rx(bcmd, expected_extra_payload+1)

        if expected_response is None:
            expected_response = cmd

        if responsehex[0] != expected_response:
            raise IOError("Unexpected ACK = 0x%x"%responsehex[0])
        
        return response[1:], responsehex[1:]

    def cmd_productinfo(self):

        response, responsehex = self.cmd_checkack(0x30, 62)
            
        pn = response[4:16]
        print(pn)

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
        self.cmd_checkack(0x40) #Erase command
        response, responsehex = self.cmd_checkack(0x54, 2) #Erase enable command, 2 seperate ACKs back

        if responsehex[0] != 0x4F:
            raise IOError("Error in erase")
        
        if responsehex[1] != 0x5D:
            raise IOError("Error in erase")

    def cmd_protect_set(self):
        self.cmd_checkack(0x60)
        cs = self.calc_checksum(self.password)
        self.write(bytes(bytearray(self.password)))
        response, responsehex = self.cmd_checkack(cs, 2, 0x60)

        if responsehex[0] != 0x6F:
            raise IOError("ACK Response to Protect Set Command: Error (%x)"%responsehex[0])

        if responsehex[1] != 0x31:
            raise IOError("ACK Response: Error (%x)"%responsehex[1])

    def cmd_ram_transfer(self, data, starting_address):        
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

class RamCodeProtocol(object):
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
    CMD_ACT = 0xF2
    CMD_ERROR = 0xF3

    ERROR_ERASE = 1
    ERROR_WRITE = 2
    ERROR_COMPARE = 3
    ERROR_BLANKCHECK = 4
    ERROR_NACK = 5
    ERROR_PROTECT = 6

    def __init__(self, ser):
        self.ser = ser

    def write(self, data):
        if hasattr(data, "len") is False:
            data = [data]
        self.ser.write(data)

    def read(self, len):
        return self.ser.read(len)

    def flush(self):
        self.ser.flush()

    def u32_to_buffer(self, u32):
        buf = [(u32 & 0xff), (u32 >> 8) & 0xff, (u32 >> 16) & 0xff, (u32 >> 24) & 0xff]
        return buf

    def u16_to_buffer(self, u16):
        buf = [(u16 & 0xff), (u16 >> 8) & 0xff]
        return buf
       
    def sendPacket(self, payload):
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
        c = self.read(1)
        if ord(c[0]) != expected:
            raise IOError("Sync Error - received %x (expected %x)"%(ord(c[0]), expected))

    def rxPacket(self):
        self.rxExpect(self.protSD1)
        self.rxExpect(self.protSD2)

        l = self.read(2)
        llsb = ord(l[0])
        lmsb = ord(l[1])

        checksum = llsb + lmsb

        plen = llsb + (lmsb << 8)

        if plen:
            payload = self.read(plen)
            for i in payload:
                checksum += ord(i)
        else:
            payload = []
        
        checksum = checksum & 0xff
        rxchecksum = ord(self.read(1)[0])

        self.rxExpect(self.protED)

        if rxchecksum != checksum:
            raise IOError("Checksum error: RX'd %x, expected %x"%(rxchecksum, checksum))
        
        return payload

    def cmd_id(self):
        self.sendPacket([self.CMD_ID])
        payload = self.rxPacket()

        cmdack = payload[0]

        flags = payload[1:5]
        flags = ord(flags[0]) + (ord(flags[1]) << 8) + (ord(flags[2]) << 16) + (ord(flags[3]) << 24)

        bufsize = payload[5:7]
        bufsize = ord(bufsize[0]) + (ord(bufsize[1]) << 8)

        return flags, bufsize

    def cmd_read(self, address, len):
        
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


        


