# PowerProfiler pcm1802
Тестирование AUDIO чипа PCM1802 в качестве ADC на два канала и 24 бита.

Максимально возможная передача по USB2.0FS у BL702 составляет 100 ksps по 24 бита на 2 канала. 

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

* Для работы с постоянным напряжением входные конденсаторы на LIN и RIN необходимо замкнуть.

---

Измерение SNR PCM1802:

![SCH](https://github.com/pvvx/PowerProfiler_pcm1802/blob/master/img/snr.png)

Шум в единицах ADC у PCM1802:

![SCH](https://github.com/pvvx/PowerProfiler_pcm1802/blob/master/img/snr-p-p.png)

По итогу измерений амплитуда шума у PCM1802 не зависит от опорной частоты в диапазоне оцифровки от 1 до 100 ksps и ограничена внутрисхемными особенностями.
Т.е. если отфильтровать питание и использовать хороший внешний Ref, возможно получить ENOB (до шума в p-p) в 20 бит. 

Тестовая программа PowerProfiler и прошивка находятся в каталоге bin в файле TestPowerProfiler.zip
