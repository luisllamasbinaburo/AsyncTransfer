import datetime
import time

from AsyncTransfer import AsyncTransfer, AsyncTransferPacket

def handle_data(data):
    timeStamp = datetime.datetime.fromtimestamp(time.time()).strftime('%H:%M:%S  ')
    print(str(timeStamp) + " " + data.decode('utf-8'))

com = AsyncTransfer()
com.Connect("COM5", 9600)
com.StartRead(handle_data)

packet = AsyncTransferPacket()
packet.AddInt16(50)
packet.AddInt16(100)
packet.AddInt16(150)

com.SendPacket(packet)
time.sleep(1000)