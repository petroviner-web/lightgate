using System.Globalization;
using System.IO.Ports;
using SprintTimer.Api.Data;
using SprintTimer.Api.Models;

namespace SprintTimer.Api.Services;

/// <summary>
/// Reads "RUN;start;finish;elapsed" lines from the device's COM port and stores them.
/// Runs in the background for the life of the web app.
/// </summary>
public sealed class SerialIngestService : BackgroundService
{
    private readonly IRunRepository _repo;
    private readonly IConfiguration _config;
    private readonly ILogger<SerialIngestService> _log;

    public SerialIngestService(IRunRepository repo, IConfiguration config, ILogger<SerialIngestService> log)
    {
        _repo = repo;
        _config = config;
        _log = log;
    }

    protected override Task ExecuteAsync(CancellationToken stoppingToken)
    {
        var portName = _config["Serial:Port"];
        if (string.IsNullOrWhiteSpace(portName))
        {
            _log.LogInformation("Serial:Port is not set, device ingest is disabled.");
            return Task.CompletedTask;
        }

        var eventId = _config.GetValue("Serial:EventId", 1);
        var baud = _config.GetValue("Serial:Baud", 115200);

        // SerialPort.ReadLine blocks, so the loop runs on its own thread.
        return Task.Run(() => ReadLoop(portName, baud, eventId, stoppingToken), stoppingToken);
    }

    private void ReadLoop(string portName, int baud, int eventId, CancellationToken ct)
    {
        using var port = new SerialPort(portName, baud) { NewLine = "\n", ReadTimeout = 1000 };
        try
        {
            port.Open();
            _log.LogInformation("Listening on {Port} at {Baud} baud.", portName, baud);
        }
        catch (Exception ex)
        {
            _log.LogError(ex, "Cannot open {Port}. Device ingest stopped.", portName);
            return;
        }

        while (!ct.IsCancellationRequested)
        {
            string line;
            try
            {
                line = port.ReadLine().Trim();
            }
            catch (TimeoutException)
            {
                continue; // no data within 1 s, check the cancellation token again
            }

            if (!TryParseRun(line, eventId, out var run))
            {
                _log.LogDebug("Ignored device line: {Line}", line);
                continue;
            }

            var id = _repo.AddAsync(run!, ct).GetAwaiter().GetResult();
            _log.LogInformation("Stored run {Id}: {Time}", id, run!.Time);
        }
    }

    /// <summary>Parses "RUN;start_us;finish_us;elapsed_us". Returns false for other lines.</summary>
    internal static bool TryParseRun(string line, int eventId, out Run? run)
    {
        run = null;
        var parts = line.Split(';');
        if (parts.Length != 4 || parts[0] != "RUN")
            return false;

        if (!long.TryParse(parts[3], NumberStyles.Integer, CultureInfo.InvariantCulture, out var elapsed)
            || elapsed <= 0)
            return false;

        run = new Run(eventId, null, new Microseconds(elapsed), DateTimeOffset.UtcNow);
        return true;
    }
}
