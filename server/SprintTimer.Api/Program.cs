using SprintTimer.Api.Data;
using SprintTimer.Api.Models;
using SprintTimer.Api.Services;

var builder = WebApplication.CreateBuilder(args);

var dbPath = builder.Configuration["Database:Path"] ?? "sprint.db";
var connectionString = $"Data Source={dbPath}";

builder.Services.AddSingleton<IRunRepository>(_ => new SqliteRunRepository(connectionString));
builder.Services.AddHostedService<SerialIngestService>();

var app = builder.Build();

await SchemaInitializer.ApplyAsync(connectionString, Path.Combine(AppContext.BaseDirectory, "schema.sql"));

app.UseDefaultFiles();   // serves wwwroot/index.html at "/"
app.UseStaticFiles();

app.MapGet("/api/leaderboard", async (int? eventId, int? take, IRunRepository repo, CancellationToken ct) =>
{
    var rows = await repo.GetLeaderboardAsync(eventId ?? 1, Math.Clamp(take ?? 10, 1, 100), ct);
    return Results.Ok(rows);
});

// Manual entry (for example, a correction or a test without the device).
app.MapPost("/api/runs", async (RunDto dto, IRunRepository repo, CancellationToken ct) =>
{
    if (dto.TimeUs <= 0)
        return Results.BadRequest(new { error = "TimeUs must be positive." });

    var run = new Run(dto.EventId, dto.AthleteId, new Microseconds(dto.TimeUs), DateTimeOffset.UtcNow);
    var id = await repo.AddAsync(run, ct);
    return Results.Created($"/api/runs/{id}", new { id });
});

app.Run();

public sealed record RunDto(int EventId, int? AthleteId, long TimeUs);
