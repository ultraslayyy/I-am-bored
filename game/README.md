# Game

Windows only game engine w sample game.

<!--
## Ways to allow writing game code in C#

### C++/CLI Bridge

Windows (MSVC) only

Folder structure:
Engine/
-- Engine.h
-- Engine.cpp

EngineCLIWrapper/
-- EngineWrapper.cpp

CSharpApp/
-- Program.cs

---

#### C++ Engine

`Engine.h`
```h
#pragma once
#include <string>

class Engine {
public:
    void PrintMessage(const std::string& msg);
    int Add(int a, int b);
};
```

`Engine.cpp`
```cpp
#include "Engine.h"
#include <iostream>

void Engine::PrintMessage(const std::string& msg) {
    std::cout << "[Engine] " << msg << std::endl;
}

int Engine::Add(int a, int b) {
    return a + b;
}
```

#### C++/CLI Wrapper
This is compiled with `/clr`

`EngineWrapper.cpp`
```cpp
#include "../Engine/Engine.h"
#include <msclr/marshal_cppstd.h>

using namespace System;

public ref class EngineWrapper {
public:
    EngineWrapper() {
        engine = new Engine();
    }

    ~EngineWrapper() {
        this->!EngineWrapper();
    }

    !EngineWrapper() {
        delete engine;
        engine = nullptr;
    }

    void PrintMessage() {
        std::string native = msclr::interop::marshal_as<std::string>(msg);
        engine->PrintMessage(native);
    }

    int Add(int a, int b) {
        return engine->Add(a, b);
    }

private:
    Engine* engine;
};
```

#### C# usage

`Program.cs`
```cs
using System;

class Program
{
    static void Main()
    {
        var engine = new EngineWrapper();

        engine.PrintMessage("Hello from C#");

        int result = engine.Add(10, 20);
        Console.WriteLine("Result: " + result);
    }
}
```

#### How to build
In Visual Studio:

1. Create 3 projects:
   - Native C++ (static lib or included)
   - C++/CLI Class Library
   - C# Console App
2. Enable CLR for wrapper. In EngineCLRWrapper project: `Project Properties → General → Common Language Runtime Support → /clr`
3. Reference the wrapper. In C# project: `Add Reference → EngineCLIWrapper`

-->