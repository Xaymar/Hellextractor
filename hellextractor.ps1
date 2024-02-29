function Read-WithDefault {
    param (
        [string]$prompt,
        [string]$default
    )

    # Show the prompt with the default value
    $inputVar = Read-Host "$prompt `nPress Enter for the default [$default]"

    # Use default value if input is empty
    if ([string]::IsNullOrWhiteSpace($inputVar)) {
        return $default
    }
    else {
        return $inputVar
    }
}


function Get-SteamPath {
    # Try to get Steam path from the registry
    try {
        $registryPath = "HKCU:\Software\Valve\Steam"
        $steamPath = (Get-ItemProperty -Path $registryPath -Name "SteamPath").SteamPath
        if ($steamPath) {
            return $steamPath
        }
        else {
            throw
        }
    }
    catch {
        Write-Host "Unable to find Steam installation directory in the registry."
        return $null
    }
}

function Find-Helldivers2Directory {
    param (
        [string]$steamPath
    )

    $helldivers2Path = Join-Path $steamPath "steamapps\common\Helldivers 2\data"
    if (Test-Path $helldivers2Path) {
        return $helldivers2Path
    }
    else {
        Write-Host "Helldivers 2 directory not found."
        return $null
    }
}

function Invoke-HellExtractor {
    # Fetch Steam installation path
    $steamPath = Get-SteamPath
    if (-not $steamPath) {
        $steamPath = Read-Host "Please enter your Steam installation path"
    }
    
    # Find Helldivers 2 directory
    $gamePath = Find-Helldivers2Directory -steamPath $steamPath
    if (-not $gamePath) {
        $gamePath = Read-Host "Please enter the full path to the Helldivers 2 data directory"
    } else {
        $gamePath = Read-WithDefault -prompt "Please enter the full path to the Helldivers 2 data directory" -default $gamePath
    }

    if (-not (Test-Path $gamePath)) {
        Write-Host "Invalid path"
        return
    }

    # Asking user for the operation type
    $operation = Read-Host "Select operation: `n1 - Extract all files to specific directory `n2 - Extract specific file type `n3 - Extract all files and translate names and types `n4 - Extract all files, translate names and types, and rename files with older names"
    
    if ([string]::IsNullOrWhiteSpace($operation)) {
        Write-Host "Invalid operation"
        return
    }

    # Base command
    $command = "extract"

    $outputDir = Read-WithDefault "Enter output directory" -default "./output"
    
    # Building the command based on user input
    switch -regex ($operation) {
        "1" {
            $command += " -o $outputDir `"$gamePath`""
        }
        "2" {
            $fileType = Read-WithDefault "Enter file type regex (e.g., .*\.texture$)" -default ".*\.texture$"
            $command += " -f `"$fileType`" -o $outputDir `"$gamePath`""
        }
        # extract and translate names and types or rename files with older names
        "3|4" {
            if ($operation -eq "4") {
                $command += " -r"
            }
            $typesFile = Read-WithDefault "Enter types file path" -default "./types.txt"
            $namesFile = Read-WithDefault "Enter names file path" -default "./files.txt"
            $stringsFile = Read-WithDefault "Enter strings file path" -default "./strings.txt"
            $command += " -o $outputDir -t $typesFile -n $namesFile -s $stringsFile `"$gamePath`""
        }
        default {
            Write-Host "Invalid operation"
            return
        }
    }
    
    # Executing the command
    Write-Host "Executing: $command"
    Start-Process -FilePath "./hellextractor.exe" -ArgumentList $command -NoNewWindow -Wait
}

# Running the function
Invoke-HellExtractor
