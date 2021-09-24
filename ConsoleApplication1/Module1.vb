

Module Module1

    Sub Main()
        Dim rcb3Interface As UInteger
        Dim full_error_String As String
        Dim error_String As String
        Dim last_error As UInteger
        Dim version(65) As Byte
        Dim out_homePositions(24) As UInteger


        If Rcb3.CreateRCB3Interface(1, 115200, 0, rcb3Interface) Then

            Console.WriteLine("ok")            

            If (Rcb3.GetRCBVersion(rcb3Interface, version)) Then
                Console.WriteLine("RCB3 Version:")
                Console.WriteLine(version)
                Console.WriteLine("\n")
            End If

            If Rcb3.GetPortsHomePosition(rcb3Interface, out_homePositions, 2) Then
                For i As Integer = 1 To 24
                    Console.WriteLine("Servo {0:D} = {1:D}\n", i, out_homePositions(i))
                Next i
            Else
                Rcb3.DestroyRCB3Interface(rcb3Interface)
            End If
        Else
            last_error = Rcb3.GetLastRCB3Error()
            error_String = Rcb3.GetLastRCB3ErrorName(last_error)
            Console.WriteLine(error_String)
            full_error_String = Rcb3.GetLastRCB3FullErrorMessage()
            Console.WriteLine(full_error_String)
        End If
    End Sub

End Module
