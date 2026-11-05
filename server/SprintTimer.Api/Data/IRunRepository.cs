using SprintTimer.Api.Models;

namespace SprintTimer.Api.Data;

/// <summary>Storage abstraction. SQLite today; PostgreSQL could be added without touching the API.</summary>
public interface IRunRepository
{
    Task<int> AddAsync(Run run, CancellationToken ct = default);

    Task<IReadOnlyList<LeaderboardEntry>> GetLeaderboardAsync(
        int eventId, int take, CancellationToken ct = default);
}
