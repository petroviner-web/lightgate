namespace SprintTimer.Api.Models;

/// <summary>
/// Time value in microseconds (the unit the firmware sends).
/// A value type with operator overloads, so arithmetic reads naturally.
/// </summary>
public readonly record struct Microseconds(long Value)
{
    public static Microseconds operator -(Microseconds later, Microseconds earlier)
        => new(later.Value - earlier.Value);

    public static Microseconds operator +(Microseconds a, Microseconds b)
        => new(a.Value + b.Value);

    public static bool operator <(Microseconds a, Microseconds b) => a.Value < b.Value;
    public static bool operator >(Microseconds a, Microseconds b) => a.Value > b.Value;

    public TimeSpan ToTimeSpan() => TimeSpan.FromTicks(Value * 10); // 1 tick = 100 ns

    public override string ToString() => $"{Value / 1_000_000.0:0.000} s";
}
