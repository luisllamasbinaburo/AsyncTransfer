using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AsyncTransferCsharp
{
    public class AsyncTransferPacket
    {

        public AsyncTransferPacket()
        {
        }

        public AsyncTransferPacket(char data)
        {
            Data.Add((byte)data);
        }

        public AsyncTransferPacket(byte data)
        {
            Data.Add(data);
        }

        public AsyncTransferPacket(IEnumerable<byte> data)
        {
            Data.AddRange(data);
        }

        private readonly List<byte> Data = new List<byte>();

        public byte Id { get; set; }


        public byte[] Payload => Data.ToArray();

        public ushort Checksum => ByteHelpers.ChecksumFletcher16(Payload);

        public byte[] ChecksumBytes => ByteHelpers.FromUInt16(Checksum);


        public byte[] Content
        {
            get
            {
                var content = new List<byte>();
                content.Add((byte)Controlcode.StartOfHeading);
                content.Add(Id);

                var lengthBytes = ByteHelpers.FromUInt16((ushort)Data.Count());
                content.AddRange(lengthBytes);

                content.Add((byte)Controlcode.StartOfText);
                content.AddRange(Payload);
                content.Add((byte)Controlcode.EndOfText);

                content.AddRange(ChecksumBytes);
                content.Add((byte)Controlcode.EndOfTransmission);

                return content.ToArray();
            }
        }

        public void Clear() => Data.Clear();
        public void AddByte(byte data) => Data.Add(data);
        public void AddByteArray(byte[] data) => Data.AddRange(data);
        public void AddChar(char data) => Data.Add((byte)data);
        public void AddInt8(int value) => AddByteArray(ByteHelpers.FromInt8(value));
        public void AddInt16(short value) => AddByteArray(ByteHelpers.FromInt16(value));
        public void AddUInt16(ushort value)=> AddByteArray(ByteHelpers.FromUInt16(value));
        public void AddInt32(int value) => AddByteArray(ByteHelpers.FromInt32(value));
        public void AddUInt32(uint value) => AddByteArray(ByteHelpers.FromUInt32(value));
        public void AddFloat(float value) => AddByteArray(ByteHelpers.FromFloat(value));
        public void AddDouble(float value) => AddByteArray(ByteHelpers.FromDouble(value));
      
    }
}
