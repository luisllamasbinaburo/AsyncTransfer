import threading

import serial

C_STARTOFHEADING = 0x01
C_STARTOFTEXT = 0x02
C_ENDOFTEXT = 0x03
C_ENDOFTRANSMISSION = 0x04
C_ENQUIRY = 0x05
C_ACKNOWLEDGE = 0x06
C_BELL = 0x07
C_BACKSPACE = 0x08
C_HORIZONTALTABULATION = 0x09
C_LINEFEED = 0x10
C_VERTICALTABULATION = 0x11
C_FORMFEED = 0x12
C_CARRIAGERETURN = 0x13
C_SHIFTOUT = 0x14
C_SHIFTIN = 0x15
C_DATALINKESCAPE = 0x16
C_DEVICECONTROLONE = 0x17
C_DEVICECONTROLTWO = 0x18
C_DEVICECONTROLTHREE = 0x19
C_DEVICECONTROLFOUR = 0x20
C_NEGATIVEACKNOWLEDGE = 0x21
C_SYNCHRONOUSIDLE = 0x22
C_ENDOFTRANSMISSIONBLOCK = 0x23
C_CANCEL = 0x24
C_ENDOFMEDIUM = 0x25
C_SUBSTITUTE = 0x26
C_ESCAPE = 0x27
C_FILESEPARATOR = 0x28
C_GROUPSEPARATOR = 0x29
C_RECORDSEPARATOR = 0x30
C_UNITSEPARATOR = 0x31

class AsyncTransfer:
    def Connect(self, port, baud):
        self.serial_port = serial.Serial()
        self.serial_port.port = port
        self.serial_port.baudrate = baud
        self.serial_port.timeout = 1
        self.serial_port.setDTR(False)
        self.serial_port.open()

    def __send(self, sendData):
        sendPacket = bytearray()
        sendPacket.append(C_STARTOFHEADING)
        sendPacket.append(0)
        sendPacket.append(len(sendData) >> 8)
        sendPacket.append(len(sendData) & 0xFF )

        sendPacket.append(C_STARTOFTEXT)
        for dataByte in sendData:
            sendPacket.append(dataByte)
        sendPacket.append(C_ENDOFTEXT)

        checksum1, checksum2 = self.__Fletcher16(sendData)
        sendPacket.append(checksum1)
        sendPacket.append(checksum2)
        sendPacket.append(C_ENDOFTRANSMISSION)
        
        self.serial_port.write(sendPacket)

    def __readThread(self, ser, callback):
        while True:
            reading = ser.readline()
            callback(reading)

    def __Fletcher16(self, dataArray):
        sum1 = sum2 = 0
        for d in dataArray:
            sum1 = (sum1 + d) % 255
            sum2 = (sum2 + sum1) % 255
        return sum1, sum2

    def StartRead(self, callback):
        self.thread = threading.Thread(target=self.__readThread, args=(self.serial_port, callback,))
        self.thread.start()

    def SendBytes(self, data):
        self.__send(data)

    def SendPacket(self, data):
        self.__send(data.Payload)


class AsyncTransferPacket:
    def __init__(self):
        self.Payload = bytearray()

    def AddByte(self, item):
        self.Payload.append(item)

    def AddByteArray(self, item):
        for b in item:
            self.AddByte(b)

    def AddChar(self, item):
        self.AddByte(item)

    def AddInt8(self, item):
        self.AddByte(item)

    def AddInt16(self, item):
        tmp = bytearray()
        tmp.append(item & 0xFF)
        tmp.append((item >> 8) & 0xFF)
        self.AddByteArray(tmp)

    def AddInt32(self, item):
        tmp = bytearray()
        tmp.append(item & 0xff)
        tmp.append((item >> 8) & 0xff)
        tmp.append((item >> 16) & 0xff)
        tmp.append((item >> 24) & 0xff)
        self.AddByteArray(tmp)

    def AddUInt8(self, item):
        self.AddByte(item)

    def AddUInt16(self, item):
        self.append(item)

    def AddUInt32(self, item):
        self.append(item)

    def AddFloat(self, item):
        self.append(item)

    def AddString(self, item):
        self.append((str).encode("Utf-8"))
