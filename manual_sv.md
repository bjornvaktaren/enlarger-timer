# Översikt

Idén med denna timer är att exponering ska ske enligt en fix tidsserie av exponeringssteg, alltså 4 s, 8 s, 16 s och så vidare, samt fraktioner därav. Att arbeta på detta sättet har otroligt många fördelar (sök "f-stop printing" eller läs Way Beyond Monochrome). Den har automatisk exponeringsserie för testremsa och möjlighet att sätta upp två separata serier med mörkning (pjatt), exponering och efterbelysning när man gör förstoringar. De två serierna kan användas för "split-grade" metoden.

Det finns ett antal vippor med vilka inställningar och lägen kan ändras:

* Auto / On: I läget "On" hålls förstoringsapparaten igång. I läget "Auto" tar den automatiserade logiken över. Läget "On" används till exempel vid fokusering.
* Test strip / Print: Med denna väljer man läget för testremsa (test strip) eller förstoring (print).
* 1/3, 1/6, 1/12: Med denna väljer man upplösning i delar av exponeringssteg.
* Timer: Med denna kan man byta mellan två separata timers (exponeringsserier) om apparaten är i förstoringsläge (print).

Men knapparna ändrar man inställningar och startar eller avbryter en exponeringsserie:

* Start / stop: Starta eller avbryt en exponeringsserie.
* Plus / minus: Öka eller minska exponeringstiden.
* Upp / ner: Bläddra genom exponeringstiderna i testremsaläge (test strip) eller välj inställning (basexponering, mörkning, efterbelysning) i förstoringsläge (print).

För att sätta igång timern, sätt i sladden i ett vägguttag och förstoringsapparaten i det uttaget på timern.

## Testremsa (test strip)

I detta läge görs en serie av 7 exponeringar ämnat för att skapa en testremsa med den valda upplösningen (1/3, 1/6 eller 1/12 exponeringssteg). Man väljer den lägsta exponeringstiden genom att använda knapparna plus/minus. Man kan bläddra mellan de resulterande exponeringstiderna med knapparna upp/ner. Det är dessa exponeringstider som kommer att bli på testremsan.

En testremsa-serie startas genom att trycka knappen start/stop. Man kan när som helst avbryta genom att trycka knappen igen. "1o7" kommer att synas på displayen i 2 sekunder innan exponeringen börjar. Det betyder att det är första av 7 exponeringar. Efter varje exponering täcker man över ytterligare en bit av testremsan. Mellan varje exponering är det en två sekunder lång fördröjning

Exempel 1: Upplösning 1/3, basexponering 8 s:

| Exponering nummer | Exponeringstid (s) | Ackumulerad exponering på testremsan (s) |
|-------------------|--------------------|------------------------------------------|
| 1                 | 8,0                | 8,0                                      |
| 2                 | 2,1                | 10,1                                     |
| 3                 | 2,6                | 12,7                                     |
| 4                 | 3,3                | 16,0                                     |
| 5                 | 4,2                | 20,2                                     |
| 6                 | 5,2                | 25,4                                     |
| 7                 | 6,6                | 32,0                                     |

Exempel 2: Upplösning 1/12, basexponering 16 s:

| Exponering nummer | Exponeringstid (s) | Ackumulerad exponering på testremsan (s) |
|-------------------|--------------------|------------------------------------------|
| 1                 | 16,0               | 16,0                                     |
| 2                 | 1,0                | 17,0                                     |
| 3                 | 1,0                | 18,0                                     |
| 4                 | 1,0                | 19,0                                     |
| 5                 | 1,2                | 20,2                                     |
| 6                 | 1,2                | 21,4                                     |
| 7                 | 1,2                | 22,6                                     |

## Förstoring (print)

I detta läge kan man sätta upp exponeringsserier för förstoringar. Det finns alltid en basexponeringstid. Det är den som visas när apparaten sätts i förstoringsläge. Den ändras genom att använda plus och minusknapparna. Man kan också definiera en serie av mörkningar och efterbelysningar. Tryck upp eller ner en gång för att se vilken den aktuella inställningen som visas är: "bE" för basexponering, d0N (N = 0, 1, ..., 9) för mörkning (dodge), och b0N (N = 0, 1, ..., 9) för efterbelysning (burn). Tryck upp eller ner igen för att ändra vilken inställning som visas. Efter två sekunder återgår skärmen till att visa exponeringstiden eller värdet för mörkning eller efterbelysning. Värdet för efterbelysning (b0N) och mörkning (d0N) är i 1/12 steg. 12 innebär alltså 12/12 = 1 steg. Här följer några exempel på hur man kan definiera en exponeringsserie.

Exempel 1: endast basexponering

| Exponering nummer | Typ | Exponeringstid (s) | Ackumulerad exponering (s) |
|-------------------|-----|--------------------|----------------------------|
| 1                 | bE  | 16,0               | 16,0                       |

Exempel 2: Basexponering (bE) på 16s, två mörkningar (d01, d02) och en efterbelysning (b01)

| Exponering nummer | Typ | Exponeringstid (s) | Ackumulerad exponering (s) |
|-------------------|-----|--------------------|----------------------------|
| 1                 | d02 | -12/12 = -1 steg   | 8,0                        |
| 2                 | d01 | -4/12 = -1/3 steg  | 11,3                       |
| 3                 | bE  | 16,0 - 11,3 = 4,7  | 16,0                       |
| 4                 | b01 | +24/12 = 2 steg    | 64,0                       |

Man kan sätta upp två sådana serier. Byt serie med "Timer" vippan. Den har två inställningar som kallas 00 och 5. Namnen antyder att de kan användas till "split-grade" metoden med filter 00 och 5.
