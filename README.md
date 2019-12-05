# Náplň cvičenia
- zoznámenie sa s časovačom (TIM)
- zoznámenie sa s analógovo digitálnym prevodníkom (ADC)

# Konfigurácia časovača (TIM3)

<p align="center">
    <img src="https://github.com/VRS-Predmet/vrs_cvicenie_10/blob/master/images/tim_config.PNG" width="800">
</p>

- timer 3 využiva interný zdroj hodín čo je v tomto prípade 8MHz

- v projekte je TIM3 využitý na generovanie prerušenia v pravidelnom časovom intervale a preto je dôležité správne nastavenie počítadla a predeličky (prescaler)

- predelička nastavená na 7999 znamená, že počítadlo časovača bude inkrementované každú milisekundu (znížil som frekvenciu inkrementovania počítadla z 8MHz na 1kHz)

- časovač nastavený na 999 znamená, že ak počítadlo napočíta do 999, nastane prerušenie (update event) a počítadlo začne počítať od 0 (ak počítame nahor - up-counting)

# Konfigurácia prevodníka (ADC1)

<p align="center">
    <img src="https://github.com/VRS-Predmet/vrs_cvicenie_10/blob/master/images/adc_config.PNG" width="800">
</p>

- využivaný ADC prevodník ma nastavené 12bit rozlíšenie
- prevod je spúšťaný programom a pri každom spustení sa prevod vykoná len raz (single channel, single conversion mode)
- prevedená hodnota je uložená do registra "DR" - data register
- okrem tohto režimu je možné ADC prevodník využívať v kopec ďalších v závislosti od aplikácie

- signál určený na prevod je privedený na GPIO pin PA0 (A0)
