# KrokowceRewolwerowce
Kod do obsługi silników krokowych w module laboratorium łazika HAL062 na zawody URC 2026 za pomocą magistrali CAN. 
### Hardware
Nucleo C092RC  
Zmodyfikowany CNC Shield - rezystor 10k przylutowany między 3V3 a EN zamiast między 5V a EN, dodatkowy kabel przylutowany do wejścia V+, do wpięcia w VIN nucleo (wymaga to przepięcia złączki JP1 z STLK na VIN podczas zasilania przez pin VIN. ***Zła pozycja złączki może zepsuć Nucleo lub USB w laptopie, V+ MAX 12V***). 
Dwa sterowniki tmc2209 Bigtreetech na portach X i Y. Wymagają złączek na pozycjach M0 i M1 na shieldzie, M2 nie zwarty (microstepping 1/16). Sterowniki można zastąpić sterownikami A4988, wtedy należy umieścić złączki na M0, M1 i M2 (microstepping 1/16), jednak silniki będą chodziły gorzej i będą grzać się znacznie bardziej. ***Nie wpinać/wypinać sterowników ani silników gdy podłączone jest zasilanie.***
### Komunikacja CAN
ID modułu - 0x099
Wszystkie zapytania zwracają  **_ramkę pozycyjną_**  opisaną w następnej sekcji. 
Sterowanie silnikami - 5 bajtów, kolejno uint8_t, int16_t, uint16_t.
uint8_t = 0xPQ
- P = 1 dla silnika X, P = 2 dla silnika Y
- Q = 1 dla ruchu do określonej pozycji, Q = 2 dla ruchu względem obecnej pozycji
int16_t - ilość kroków do wykonania. Pełen obrót to 200*16 = 3200 kroków.
uint16_t - odstęp pomiędzy krokami w 1/50 ms. Nie schodzić poniżej 30, dla operacji laboratoryjnych rekomendowane jest około 200 * (1/50ms).
**Inne zapytania**
- 0x13 - home X
- 0x23 - home Y
- 0x00 - zapytanie o pozycję silników
- 0x90 - wyłącz sterowniki i silniki (sterowanie pinem EN). Wysłanie komendy ruchu gdy silniki są wyłączone odeślę ramkę pozycyjną z pierwszym bajtem 0x90.
- 0x91 - włącz sterowniki i silniki
- **0x99 - awaryjne zatrzymanie silników**
### Ramka pozycyjna
Ramka odsyłana po każdym zapytaniu, ID = 0x099. Składa się z pięciu bajtów:
- 1 - Status - 0x90 po wysłaniu komendy ruchu gdy sterowniki są wyłączone, inne wartości zostaną zaimplenentowane w następnych aktualizacjach.
- 2 i 3 - pozycja silnika X jako int16_t w krokach
- 3 i 4 - pozycja silnika Y jako int16_t w krokach

 
