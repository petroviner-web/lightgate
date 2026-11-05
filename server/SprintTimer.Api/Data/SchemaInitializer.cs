using Microsoft.Data.Sqlite;

namespace SprintTimer.Api.Data;

public static class SchemaInitializer
{
    /// <summary>Runs schema.sql (all statements are idempotent) against the database.</summary>
    public static async Task ApplyAsync(string connectionString, string schemaPath)
    {
        var script = await File.ReadAllTextAsync(schemaPath);

        await using var conn = new SqliteConnection(connectionString);
        await conn.OpenAsync();

        await using var cmd = conn.CreateCommand();
        cmd.CommandText = script;
        await cmd.ExecuteNonQueryAsync();
    }
}
