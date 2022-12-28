
import serial

class LowLevelBootloader(object):

    def __init__(self, comobject, reset_function):
        self.ser = comobject
        self.reset = reset_function
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

    def cmd_protect_set(self, password=[0xFF]*12):
        self.cmd_checkack(0x60)
        cs = self.calc_checksum(password)
        self.write(bytes(bytearray(password)))
        response, responsehex = self.cmd_checkack(cs, 2, 0x60)

        if responsehex[0] != 0x6F:
            raise IOError("ACK Response to Protect Set Command: Error (%x)"%responsehex[0])

        if responsehex[1] != 0x31:
            raise IOError("ACK Response: Error (%x)"%responsehex[1])


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

    def write(self, byte):
        todo

    def writeU16(self, data):
        self.write(data & 0xff)
        self.write((data >> 8) & 0xff)

    def writeU32(self, data):
        self.write(data & 0xffff)
        self.write((data >> 16) & 0xffff)


    def sendPacket(self, payload):

        self.write(self.protSD1)
        self.write(self.protSD2)

        lenLow = len(payload) & 0xFF
        lowHigh = len(payload) >> 8

        self.write(lenLow)
        self.write(lenHigh)

        checksum = lenLow + lenHigh

        for c in payload:
            checksum += c
            self.write(c)
        
        self.write(checksum & 0xff)

        self.write(self.protED)