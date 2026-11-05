namespace SprintTimer.Api.Models;

/// <summary>One completed sprint, as measured by the device.</summary>
public sealed class Run
{
    // Public getters, private setters: the state changes only through the class itself.
    public int Id { get; private set; }
    public int EventId { get; }
    public int? AthleteId { get; }
    public Microseconds Time { get; }
    public DateTimeOffset RecordedAt { get; }

    public Run(int eventId, int? athleteId, Microseconds time, DateTimeOffset recordedAt)
    {
        if (eventId <= 0)
            throw new ArgumentOutOfRangeException(nameof(eventId), "Event id must be positive.");
        if (time.Value <= 0)
            throw new ArgumentOutOfRangeException(nameof(time), "Run time must be positive.");

        EventId = eventId;
        AthleteId = athleteId;
        Time = time;
        RecordedAt = recordedAt;
    }

    // internal: only the repository assigns the database id.
    internal void AssignId(int id) => Id = id;
}

/// <summary>One row of the leaderboard.</summary>
public sealed record LeaderboardEntry(int Rank, string Athlete, long TimeUs);
