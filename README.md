# Sprint Timer

Sistem za merenje vremena trke na 60 m (ili bilo koju dužinu) pomoću dve svetlosne barijere.
Mikrokontroler meri vreme sa tačnošću od mikrosekunde, backend čuva rezultate u bazi,
a web stranica prikazuje živu rang listu.

```
 [svetlosna barijera START]    [svetlosna barijera CILJ]
            |                              |
            v                              v
 +-----------------------------------------------------+
 |  Raspberry Pi Pico (RP2040, ARM Cortex-M0+)         |
 |  C: prekidi na GPIO, konačni automat, UART/USB      |
 +-----------------------------------------------------+
            |  USB-serijski:  RUN;start_us;finish_us;elapsed_us
            v
 +-----------------------------------------------------+
 |  ASP.NET Core (C#)                                  |
 |  SerialIngestService -> IRunRepository -> SQLite    |
 |  REST: GET /api/leaderboard, POST /api/runs         |
 +-----------------------------------------------------+
            |  HTTP + JSON
            v
 +-----------------------------------------------------+
 |  Web stranica (HTML + JavaScript)                   |
 |  osvežava rang listu svake 2 s                      |
 +-----------------------------------------------------+
```

## Tehnologije i gde se koriste

| Oblast iz spiska          | Gde u projektu                                                  |
|---------------------------|------------------------------------------------------------------|
| C                         | `firmware/src/` - prekidi, konačni automat, protokol            |
| C++ / ARM mikrokontroler  | RP2040 (ARM Cortex-M0+), Pico SDK, `firmware/CMakeLists.txt`    |
| .NET (C#)                 | `server/SprintTimer.Api/` - servis, REST API, serijska komunikacija |
| OOP                       | Interfejs `IRunRepository`, privatni setter-i, `internal` metode, operatori u `Microseconds` |
| Kolekcije                 | `List<LeaderboardEntry>`, `IReadOnlyList<T>`                    |
| JavaScript                | `server/SprintTimer.Api/wwwroot/app.js` - fetch, DOM, osvežavanje |
| Osnove weba              | HTTP metode, JSON, statički fajlovi, rute u `Program.cs`        |
| Relaciona baza i SQL      | `db/schema.sql` - tabele, indeks, `LEFT JOIN`, `ORDER BY ... LIMIT` |

## Struktura

```
sprint-timer/
├── firmware/            C kod za Raspberry Pi Pico
│   ├── include/         sprint_fsm.h
│   ├── src/             sprint_fsm.c (čista logika), hal_pico.c (hardver), main.c
│   ├── tests/           unit testovi koji rade na PC-u, bez hardvera
│   └── CMakeLists.txt
├── server/
│   └── SprintTimer.Api/ ASP.NET Core API + wwwroot (web stranica)
└── db/
    └── schema.sql       SQL šema baze
```

Logika merenja (`sprint_fsm.c`) je odvojena od hardvera (`hal_pico.c`), pa se može testirati na računaru.

## Protokol između uređaja i servera

Uređaj šalje jednu liniju po trci:

```
RUN;1234567890;1239017890;4250000
```

`start_us`, `finish_us` i `elapsed_us` su mikrosekunde od pokretanja uređaja. Server u
`SerialIngestService.TryParseRun` proverava format i odbacuje sve ostale linije (npr. `READY`, `ARMED`).

## Pokretanje

### 1. Testovi konačnog automata (bez hardvera)

Iz Developer Command Prompt-a za Visual Studio:

```bat
cd firmware\tests
cl /nologo /W4 /I ..\include test_sprint_fsm.c ..\src\sprint_fsm.c
test_sprint_fsm.exe
```

Očekivani ispis: `OK: all sprint_fsm tests passed`.

### 2. Firmver za Raspberry Pi Pico

Potreban je [Pico SDK](https://github.com/raspberrypi/pico-sdk) i CMake, a `PICO_SDK_PATH` mora da pokazuje na SDK.

```bash
cd firmware
mkdir build && cd build
cmake ..
cmake --build .
```

Dobija se `sprint_timer.uf2`. Pico se uključi držeći BOOTSEL, pa se fajl kopira na disk koji se pojavi.

**Ožičenje:** svaka barijera je IR LED dioda i fototranzistor. Izlaz fototranzistora ide na
GPIO 2 (START) i GPIO 3 (CILJ), uz pull-up. Kad zrak pukne, linija pada na 0 V, što generiše
falling-edge prekid. Logika je 3,3 V.

### 3. Server

Potreban je .NET 8 SDK.

```bash
cd server/SprintTimer.Api
dotnet run
```

U `appsettings.json` postavi `Serial:Port` (npr. `COM5`) da bi server čitao podatke sa uređaja.
Bez toga server radi kao obična web aplikacija. Otvori http://localhost:5000 (ili port koji ispiše `dotnet run`).

Ručno dodavanje trke (bez uređaja):

```bash
curl -X POST http://localhost:5000/api/runs -H "Content-Type: application/json" -d "{\"eventId\":1,\"athleteId\":null,\"timeUs\":4250000}"
```

## Status

- [x] Konačni automat i testovi (C)
- [x] Hardverska veza za RP2040 (prekidi, GPIO)
- [x] Protokol i parsiranje na serveru
- [x] REST API, SQLite repozitorijum, SQL šema
- [x] Web rang lista (JavaScript)
- [ ] Dodavanje takmičara i povezivanje trke sa takmičarom
- [ ] Više dionica (međuvremena) sa trećom barijerom
- [ ] Autentifikacija za POST /api/runs

## Ideje za dalji razvoj

- Treća barijera za međuvremena (split times).
- Tabela takmičara i stranica za dodeljivanje trke takmičaru.
- Prebacivanje na PostgreSQL (šema je standardni SQL, potrebno je zameniti SQLite konektor).
- Offline režim: uređaj pamti rezultate u flash memoriji, ako server nije dostupan.
