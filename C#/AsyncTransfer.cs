using System;
using System.IO.Ports;

namespace AsyncTransferCsharp
{
    public class AsyncTransfer
    {
        private SerialPort _arduinoPort;
        public event EventHandler OnDataArrive;

        public bool IsOpen => _arduinoPort?.IsOpen ?? false;

        public bool IsReady { get; private set; }
        public bool IsAcknowledgeReceived { get; private set; }

        public string TextRecieved { get; private set; } = "";
        public string LastRecieved { get; private set; } = "";

        public bool Open(string port, int baud)
        {
            try
            {
                _arduinoPort = new SerialPort(port);
                _arduinoPort.BaudRate = baud;
                _arduinoPort.DtrEnable = true;
                _arduinoPort.ReadTimeout = 100;
                _arduinoPort.WriteTimeout = 100;
                _arduinoPort.DataReceived += new SerialDataReceivedEventHandler(DataRecieved);
                _arduinoPort.Open();
                return true;
            }
            catch (Exception)
            {
                return false;
            }
        }


        public void Close()
        {
            if (_arduinoPort.IsOpen) _arduinoPort.Close();
        }

        public void SendHandShake()
        {
            SendData(new byte[] { (byte)Controlcode.StartOfHeading });
        }

        public void SendHandShake(Action handshakeHandler)
        {
            SendData(new byte[] { (byte)Controlcode.StartOfHeading });
            ReceiveHandShake(handshakeHandler);
        }

        public void SendPacket(AsyncTransferPacket packet)
        {
            SendData(packet.Content);
        }

        public bool SendPacketWithAcknowledge(AsyncTransferPacket packet)
        {
            SendPacket(packet);
            IsAcknowledgeReceived = ReceiveControlCode(Controlcode.Acknowledge);
            return IsAcknowledgeReceived;
        }

        private void ReceiveHandShake(Action handshakeHandler)
        {
            IsReady = ReceiveControlCode(Controlcode.EndOfTransmission);
            handshakeHandler?.Invoke();
        }

        public void RecieveAcknowledge(object sender, SerialDataReceivedEventArgs e)
        {
            ReceiveControlCode(Controlcode.Acknowledge);
        }

        private void SendData(byte[] dataBytes)
        {
            _arduinoPort.Write(dataBytes, 0, dataBytes.Length);
        }

        private bool ReceiveControlCode(char controlCode)
        {
            var rst = false;
            _arduinoPort.ReadTimeout = 2000;
            try
            {
                var data = (char)_arduinoPort.ReadChar();
                rst = data == controlCode;
            }
            catch (TimeoutException)
            {
                rst = false;
            }
            return rst;
        }

        private void DataRecieved(object sender, SerialDataReceivedEventArgs e)
        {
            try
            {
                LastRecieved = _arduinoPort.ReadExisting();
                TextRecieved += LastRecieved;
                OnDataArrive?.Invoke(this, new EventArgs());
            }
            catch (Exception)
            {
                // Do nothing
            }
        }

        public void ClearData()
        {
            TextRecieved = "";
        }
    }
}