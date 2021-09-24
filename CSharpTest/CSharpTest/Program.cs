using System;
using System.Collections.Generic;
using System.Text;


namespace CSharpTest
{
    class Program
    {
        static void Main(string[] args)
        {
            uint myrcb = 1;
            if (RCB3Interface.CreateRCB3Interface(6, 115200, 0,ref myrcb))
            {
                StringBuilder version = new StringBuilder(65);
                //char[] version = new char[65];
                if (RCB3Interface.GetRCBVersion(myrcb, version))
                {
                    Console.WriteLine("RCB3 Version:");
                    Console.WriteLine(version);
                    Console.WriteLine("\n");
                }

                uint[] out_homePositions = new uint[24];
                if (RCB3Interface.GetPortsHomePosition(myrcb, out_homePositions/*24*/, 2))
                {
                    for (int i = 0; i < 24; i++)
                    {
                        Console.WriteLine("Servo {0:D} = {1:D}\n", i, out_homePositions[i]);
                    }
                }
                RCB3Interface.DestroyRCB3Interface(myrcb);                
            }
        }
    }
}
