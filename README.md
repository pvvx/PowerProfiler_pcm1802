# PowerProfiler pcm1802
Тестирование AUDIO чипа PCM1802 в качестве ADC на два канала и 24 бита.

Максимально возможная передача по USB2.0 HS у BL702 составляет 100 ksps по 24 бита на 2 канала. 

Используется модуль RV-Debugger-BL702 и CJMCU-PCM1802

![SCH](https://github.com/pvvx/PowerProfiler_pcm1802/blob/master/img/CJMCU-PCM1802.png)

![SCH](https://github.com/pvvx/PowerProfiler_pcm1802/blob/master/img/RV-Debugger-BL702.png)

Соединения модулей:

|RV-Debugger-BL702 | CJMCU-PCM1802 |
| ------: | ------- |
| 5V | 5V |
| -- | 3.3V |
| GND | GND |
| TMS | DOUT |
| RTS | BCK |
| -- | FSY |
| TDO | LRCK |
| RX | POW |
| TX | SCK |

Установки на CJMCU-PCM1802 

| Маркировка | соединение |
| ------: | ------- |
| OSR | перемычка |
| FMT1 | пусто |
| FMT0 | перемычка |
| MODE1 | пусто |
| MODE0 | перемычка |
| BYPAS | перемычка |
| + | соединить с 3.3V|

Измерение SNR PCM1802:

![SCH](https://github.com/pvvx/PowerProfiler_pcm1802/blob/master/img/snr.png)

Шум в единицах ADC у PCM1802:

![SCH](https://github.com/pvvx/PowerProfiler_pcm1802/blob/master/img/snr-p-p.png)

Тестовая программа PowerProfiler и прошивка находятся в каталоге bin в файле TestPowerProfiler.zip
