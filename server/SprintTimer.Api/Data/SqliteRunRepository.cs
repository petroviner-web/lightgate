using Microsoft.Data.Sqlite;
using SprintTimer.Api.Models;

namespace SprintTimer.Api.Data;

public sealed class SqliteRunRepository : IRunRepository
{
    private readonly string _connectionString;

    public SqliteRunRepository(string connectionString)
    {
        _connectionString = connectionString;
    }

    public async Task<int> AddAsync(Run run, CancellationToken ct = default)
    {
        await using var conn = new SqliteConnection(_connectionString);
        await conn.OpenAsync(ct);

        await using (var insert = conn.CreateCommand())
        {
            insert.CommandText = """
                INSERT INTO runs (event_id, athlete_id, time_us, recorded_at)
                VALUES ($event, $athlete, $time, $at);
                """;
            insert.Parameters.AddWithValue("$event", run.EventId);
            insert.Parameters.AddWithValue("$athlete", (object?)run.AthleteId ?? DBNull.Value);
            insert.Parameters.AddWithValue("$time", run.Time.Value);
            insert.Parameters.AddWithValue("$at", run.RecordedAt.ToString("O"));
            await insert.ExecuteNonQueryAsync(ct);
        }

        await using var idCmd = conn.CreateCommand();
        idCmd.CommandText = "SELECT last_insert_rowid();";
        var id = Convert.ToInt32(await idCmd.ExecuteScalarAsync(ct));

        run.AssignId(id);
        return id;
    }

    public async Task<IReadOnlyList<LeaderboardEntry>> GetLeaderboardAsync(
        int eventId, int take, CancellationToken ct = default)
    {
        await using var conn = new SqliteConnection(_connectionString);
        await conn.OpenAsync(ct);

        await using var cmd = conn.CreateCommand();
        cmd.CommandText = """
            SELECT COALESCE(a.full_name, 'Anonimus') AS athlete, r.time_us
            FROM runs r
            LEFT JOIN athletes a ON a.id = r.athlete_id
            WHERE r.event_id = $event
            ORDER BY r.time_us
            LIMIT $take;
            """;
        cmd.Parameters.AddWithValue("$event", eventId);
        cmd.Parameters.AddWithValue("$take", take);

        var rows = new List<LeaderboardEntry>();
        await using var reader = await cmd.ExecuteReaderAsync(ct);
        var rank = 1;
        while (await reader.ReadAsync(ct))
        {
            rows.Add(new LeaderboardEntry(rank++, reader.GetString(0), reader.GetInt64(1)));
        }
        return rows;
    }
}
