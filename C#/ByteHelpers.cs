using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AsyncTransferCsharp
{
    public static class ByteHelpers
    {
        public static byte[] FromInt8(int value)
        {
            return BitConverter.GetBytes((byte)value);
        }

        public static byte[] FromUInt16(ushort value)
        {
            return BitConverter.GetBytes(value);
        }

        public static byte[] FromUInt16(uint value)
        {
            return BitConverter.GetBytes((ushort)value);
        }

        public static byte[] FromInt16(short value)
        {
            return BitConverter.GetBytes(value);
        }

        public static byte[] FromInt16(int value)
        {
            return BitConverter.GetBytes((short)value);
        }

        public static byte[] FromUInt32(uint value)
        {
            return BitConverter.GetBytes(value);
        }

        public static byte[] FromInt32(int value)
        {
             return BitConverter.GetBytes(value);
        }

        public static byte[] FromFloat(float value)
        {
            return BitConverter.GetBytes(value);
        }

        public static byte[] FromDouble(double value)
        {
            return BitConverter.GetBytes(value);
        }

        public static ushort ChecksumFletcher16(byte[] data)
        {
            byte sum1 = 0;
            byte sum2 = 0;

            foreach (var dataByte in data)
            {
                sum1 = (byte)((sum1 + dataByte) % 255);
                sum2 = (byte)((sum2 + sum1) % 255);
            }
            return (ushort)((sum2 << 8) | sum1);
        }

    }
}
