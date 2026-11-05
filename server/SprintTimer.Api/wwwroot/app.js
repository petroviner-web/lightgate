// Leaderboard page: fetches /api/leaderboard every 2 seconds and renders the table.
(function () {
  "use strict";

  const params = new URLSearchParams(window.location.search);
  const eventId = Number(params.get("eventId") || 1);
  const REFRESH_MS = 2000;

  const rowsEl = document.getElementById("rows");
  const statusEl = document.getElementById("status");
  document.getElementById("event").textContent = String(eventId);

  // Converts microseconds from the device into "12.345 s".
  function formatMicros(us) {
    return (us / 1_000_000).toFixed(3) + " s";
  }

  function renderRows(entries) {
    rowsEl.replaceChildren();

    if (entries.length === 0) {
      const tr = document.createElement("tr");
      const td = document.createElement("td");
      td.colSpan = 3;
      td.textContent = "Još nema rezultata.";
      tr.appendChild(td);
      rowsEl.appendChild(tr);
      return;
    }

    for (const entry of entries) {
      const tr = document.createElement("tr");
      if (entry.rank === 1) tr.className = "first";

      const rank = document.createElement("td");
      rank.textContent = entry.rank;

      // textContent (not innerHTML) so names from the database can't inject markup.
      const name = document.createElement("td");
      name.textContent = entry.athlete;

      const time = document.createElement("td");
      time.className = "time";
      time.textContent = formatMicros(entry.timeUs);

      tr.append(rank, name, time);
      rowsEl.appendChild(tr);
    }
  }

  async function refresh() {
    try {
      const res = await fetch(`/api/leaderboard?eventId=${eventId}&take=10`, { cache: "no-store" });
      if (!res.ok) throw new Error(`HTTP ${res.status}`);
      const entries = await res.json();
      renderRows(entries);
      statusEl.textContent = "Ažurirano " + new Date().toLocaleTimeString("sr-RS");
    } catch (err) {
      statusEl.textContent = "Greška pri učitavanju: " + err.message;
    }
  }

  refresh();
  setInterval(refresh, REFRESH_MS);
})();
