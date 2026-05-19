# KrokowceRewolwerowce
Kod do obsługi silników krokowych w module laboratorium łazika HAL062 na zawody URC 2026 za pomocą magistrali CAN. 
### Hardware
Nucleo C092RC  
Zmodyfikowany CNC Shield - rezystor 10k przylutowany między 3V3 a EN zamiast między 5V a EN, dodatkowy kabel przylutowany do wejścia V+, do wpięcia w VIN nucleo (wymaga to przepięcia złączki JP1 z STLK na VIN podczas zasilania przez pin VIN. ***Zła pozycja złączki może zepsuć Nucleo lub USB w laptopie, V+ MAX 12V***). 
Dwa sterowniki tmc2209 Bigtreetech na portach X i Y. Wymagają złączek na pozycjach M0 i M1 na shieldzie, M2 nie zwarty (microstepping 1/16). Sterowniki można zastąpić sterownikami A4988, wtedy należy umieścić złączki na M0, M1 i M2 (microstepping 1/16), jednak silniki będą chodziły gorzej i będą grzać się znacznie bardziej. ***Nie wpinać/wypinać sterowników ani silników gdy podłączone jest zasilanie.***. W kodzie domyślnie silnik X to rewolwer z probówkami, Y to strzykawki. Można to prosto zmienić w kodzie zamieniając pozProbowek i pozStrzykawek w komendach CAN 0x31 i 0x32.
### Komunikacja CAN
Adres CAN do wysyłania ramek do krokowców: ***0x099***
Kod odsyła ramki z adresem Identifier ***0x099***
Wszystkie zapytania zwracają  **_ramkę pozycyjną_**  opisaną w dalszej sekcji. 

***Ruszanie silnikami***

Każda komenda sterowania silnikami składa się z 5 bajtów, kolejno sklejanych w uint8_t, int16_t, uint16_t.
uint8_t = 0xPQ
- P = 1 dla silnika X, P = 2 dla silnika Y
- Q - typ komendy:
  -  1 - ruch do określonej pozycji absolutnej.
  -  2 - ruch o określoną liczbę kroków względem obecnej pozycji. Kierunek silnika można swapnąć w if w funkcjach moveX i moveY.
  -  3 - ruch do konkretnej strzykawki/probówki. Pozycje należy uzupełnić w kodzie na podstawie zmierzonych po zmontowaniu labo.
  -  8 - home. Kierunek homeowania ustawia się w kodzie, dodając znak "-" przy ilości kroków do wykonania w komendzie 0x81 lub 0x82. Przerwanie od czujników krańcowych jest aktywne ***TYLKO PODCZAS HOMEOWANIA*** i nie będzie hamować rewolwerów poza nim. Można to zmienić w kodzie, usuwając zerowanie zmiennych xEndstopEn i yEndstopEn w przerwaniu Falling_Callback.
int16_t - ilość kroków do wykonania. Pełen obrót to 200*16 = 3200 kroków.
uint16_t - odstęp pomiędzy krokami w ***1/100ms**. Nie schodzić poniżej 30, dla operacji laboratoryjnych rekomendowane jest około 200 lub więcej * (1/50ms).

**Inne zapytania**
- **0x99 - awaryjne zatrzymanie silników**
- 0x00 - zapytanie o pozycję silników
- 0x90 - wyłącz sterowniki i silniki (sterowanie pinem EN). Wysłanie komendy ruchu gdy silniki są wyłączone odeślę ramkę pozycyjną z pierwszym bajtem 0x90.
- 0x91 - włącz sterowniki i silniki. Początkowo silniki są **włączone**.
- **0x99 - awaryjne zatrzymanie silników**

### Ramka pozycyjna
Ramka odsyłana po każdym zapytaniu, ID = 0x099, 5 bajtów kolejno uint8_t, int16_t, int16_t. Ramka wysyłana jest gdy **obydwa** silniki zakończą ruch lub od razu w przypadku błędu. Składa się z bajtów:
- 1 - Status silników
  - 0x01 - OK, wszystkie inne statusy to "błędy"
  - 0x02 - Niepoprawna instrukcja
  - 0x90 - Silniki wyłączone
  - 0x13/0x23 - wybrano probówkę/strzykawkę z poza zakresu 
- 2 i 3 - pozycja silnika X jako int16_t w krokach
- 3 i 4 - pozycja silnika Y jako int16_t w krokach

 
