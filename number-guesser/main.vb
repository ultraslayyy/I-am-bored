Imports System

Module Program
    Sub Main()
        Dim random As New Random()
        Dim num As Integer = random.Next(0, 101)

        while True
            Console.Write("Guess: ")
            Dim input As String = Console.ReadLine()

            Dim guess As Integer
            If Not Integer.TryParse(input, guess) Then
                Console.WriteLine("Please enter a valid number")
                Continue While
            End If

            If guess > num Then
                Console.WriteLine("Lower")
            ElseIf guess < num Then
                Console.WriteLine("Higher")
            Else
                Console.WriteLine($"You got it! The number was {num}")
                Exit While
            End If
        End While
    End Sub
End Module