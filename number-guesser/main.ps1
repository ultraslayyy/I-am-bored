$num = Get-Random -Maximum 101
$guess = 0

while ($true) {
    $guessIn = Read-Host -Prompt "Guess"
    $temp = [int]::TryParse($guessIn, [ref]$guess)

    if (-not $temp) {
        Write-Host "Please enter a valid number"
        continue
    }

    if ($guess -gt $num) {
        Write-Host "Lower"
    }
    elseif ($guess -lt $num) {
        Write-Host "Higher"
    }
    else {
        Write-Host "You got it! The number was $num"
        break
    }
}