// https://github.com/ultraslayyy/I-am-bored/tree/info/code-runner/src/sdks/runner-sdk.cs
// Please don't delete the above line, to credit me. But I can't stop you.
using System.Diagnostics;
using System.Net.Sockets;
using System.Text;
using System.Text.Json;

namespace RunnerClient;

public enum Lang
{
    Python,
    Python3,
    Javascript,
    Typescript,
    Cpp,
    C,
    Java,
    CSharp,
    Go,
    Kotlin,
    Swift,
    Rust,
    Ruby,
    Php,
    Dart,
    Scala,
    Elixir,
    Erlang,
    Racket,
    Bash,
    MySql,
    MsSql,
    Postgres,
    Oracle,
    Pandas
}

public class RunResult
{
    public string Stdout { get; set; }
    public string Stderr { get; set; }
    public int? ExitCode { get; set; }
    public bool Success { get; set; }
    public double TimeMs { get; set; }
    public long MemoryKb { get; set; }
}

public class TestCase
{
    public string Input { get; set; }
    public string Expected { get; set; }
}

public class TestCaseOptions
{
    public bool Parallel { get; set; } = false;
    public int ConcurrencyLimit { get; set; } = 10;
    public TestCase[] Cases { get; set; }
}

public class TestCaseResult : RunResult
{
    public string Input { get; set; }
    public string Expected { get; set; }
    public bool Passed { get; set; }
}

public class RunnerClient(string host = "127.0.0.1", int port = 4000)
{
    private readonly string _host = host;
    private readonly int _port = port;
    private Process serverProcess;

    public void StartServer(string filePath = "runner-server.ts", bool createWindow = false)
    {
        ProcessStartInfo startInfo = new()
        {
            FileName = "node",
            Arguments = filePath,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            UseShellExecute = false,
            CreateNoWindow = !createWindow
        };

        serverProcess = new()
        { StartInfo = startInfo };
        process.OutputDataReceived += (sender, e) => { if (e.Data != null) Console.WriteLine(e.Data); };
        process.ErrorDataReceived += (sender, e) => { if (e.Data != null) Console.Error.WriteLine(e.Data); };

        process.Start();
        process.BeginOutputReadLine();
        process.BeginErrorReadLine();

        Console.WriteLine($"Started server with PID {serverProcess.Id}");
    }

    public void StopServer()
    {
        if (serverProcess != null && !serverProcess.HasExited)
        {
            serverProcess.Kill();
            serverProcess.WaitForExit();
            Console.WriteLine("Server stopped.");
        }
        else
        {
            Console.WriteLine("Server is not running.");
        }
    }

    public async Task<RunResult> RunCodeLocalAsync(string code, Lang language, string input = "")
    {
        return await SendRequestAsync<RunResult>(code, language, input, null, false);
    }

    public async Task<TestCaseResult[]> RunCodeLocalAsync(string code, Lang language, TestCaseOptions testCases)
    {
        return await SendRequestAsync<TestCaseResult[]>(code, language, "", testCases, false);
    }

    public async Task<RunResult> RunCodeDockerAsync(string code, Lang language, string input = "")
    {
        return await SendRequestAsync<RunResult>(code, language, input, null, true);
    }

    public async Task<TestCaseResult[]> RunCodeDockerAsync(string code, Lang language, TestCaseOptions testCases)
    {
        return await SendRequestAsync<TestCaseResult[]>(code, language, "", testCases, true);
    }

    private async Task<T> SendRequestAsync<T>(string code, Lang language, string input, object testCases, bool useDocker)
    {
        var request = new
        {
            code,
            language = language.ToString().ToLower(),
            input,
            testCases,
            runner = useDocker ? "docker" : "local"
        };

        string jsonRequest = JsonSerializer.Serialize(request) + "\n";

        using TcpClient client = new();
        await client.ConnectAsync(_host, _port);
        using NetworkStream stream = client.GetStream();

        byte[] requestBytes = Encoding.UTF8.GetBytes(jsonRequest);
        await stream.WriteAsync(requestBytes);

        StringBuilder responseBuilder = new();
        byte[] buffer = new byte[1024];
        int bytesRead;
        do
        {
            bytesRead = await stream.ReadAsync(buffer, 0, buffer.Length);
            responseBuilder.Append(Encoding.UTF8.GetString(buffer, 0, bytesRead));
        } while (!responseBuilder.ToString().Contains('\n'));

        string jsonResponse = responseBuilder.ToString().Trim();
        return JsonSerializer.Deserialize<T>(jsonResponse);
    }
}
