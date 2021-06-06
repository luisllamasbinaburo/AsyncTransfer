using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AsyncTransferCsharp
{
    sealed class ArduinoPort : SerialPort
    {
        public ArduinoPort(string portName, int baudRate)
        {
            this.PortName = portName;
            this.BaudRate = baudRate;
        }

        public ArduinoPort()
        {
            ArduinoPort = new System.IO.Ports.SerialPort();
        }

        public void Connect(string portName, int baudRate)
        {
            ArduinoPort.PortName = portName;
            ArduinoPort.BaudRate = baudRate;
            ArduinoPort.Open();

            while (!ArduinoPort.IsOpen)
            { }
        }

        public bool IsOpen()
        {
            return ArduinoPort.IsOpen;
        }

        public void WriteStart()
        {
            ArduinoPort.Write(STX.ToString());
        }

        public void WriteEnd()
        {
            ArduinoPort.Write(ETX.ToString());
        }

        public void WriteInt8(int value)
        {
            byte[] buffer = BitConverter.GetBytes(value);
            WriteByteArray(buffer, 0, 1);
        }

        public void WriteInt16(int value)
        {
            byte[] buffer = BitConverter.GetBytes(value);
            WriteByteArray(buffer, 0, 2);
        }

        public void WriteInt32(int value)
        {
            byte[] buffer = BitConverter.GetBytes(value);
            WriteByteArray(buffer, 0, 4);
        }

        public void WriteByteArray(byte[] data)
        {
            ArduinoPort.Write(data, 0, data.Length);
        }

        public void WriteByteArray(byte[] data, int offset, int length)
        {
            ArduinoPort.Write(data, offset, length);
        }

        public string ReadLine()
        {
            string rst = ArduinoPort.ReadLine();
            rst = rst.Replace("\r", "");
            rst = rst.Replace("\n", "");
            return rst;
        }

        public int ReadInt16()
        {
            return Int16.Parse(ArduinoPort.ReadLine());
        }

        public short ReadInt16()
        {
            return Int16.Parse(ArduinoPort.ReadLine());
        }

        public int ReadInt32()
        {
            return Int32.Parse(ArduinoPort.ReadLine());
        }
    }
}
