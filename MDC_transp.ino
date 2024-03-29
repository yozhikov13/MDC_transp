//подключение библиотек для работы модулей и датчиков
#include <OneWire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <TFT_ILI9163C.h>

//определение цветов экрана в переменных
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0  
#define WHITE   0xFFFF

//создание идентификатора под номер пина, к которому подключен датчик давления
#define PIN_WATERPRESSURE A0

//создание идентификатора под номер пина, к которому подключена помпа для подачи жидкости
#define PIN_WATERPUMP 13

//создание идентификатора под номер пина, к которому подключен термометр 1
OneWire ds1(14); 
//создание идентификатора под номер пина, к которому подключен термометр 2
OneWire ds2(15); 
////создание идентификатора под номер пина, к которому подключен куллер охлаждения контейнера
#define PIN_FAN 16
////создание идентификатора под номер пина, к которому подключен нагревательный элемент контейнера
#define PIN_HEATER 17
//создание идентификатора под номера пинов, к которым подключен жк экран
#define __CS 18
#define __DC 19
#define __RES 20
//создание идентификатора под номер пина, к которому подключен пьезо-элемент
#define PIN_ZOMMER 21
//создание идентификатора под номер пина, к которому подключен газовый шаровой кран
#define PIN_GASTAP 22
//создание идентификатора под номер пина, к которому подключена кнопка выбора действия
#define PIN_BTCONTROL 23
//создание идентификатора под номер пина, к которому подключена кнопка прибавления значения
#define PIN_BTPLUS 24
//создание идентификатора под номер пина, к которому подключена кнопка убавления значения
#define PIN_BTMINUS 25

//реализация работы жк экрана
TFT_ILI9163C display = TFT_ILI9163C(__CS, __DC, __RES);

//создание переменной для числа Пи
float p = 3.1415926;
//создание переменной для хранения считанной температуры внутри обогащенной жидкости
float temperature1;
//создание переменной для хранения считанной температуры внутри контейнера
float temperature2;
//создание переменных для хранения напряжения от датчика давления и давления обогащенной жидкости 
float V, P;
//создание переменной для хранения коэффициента датчика давления
float OffSet = 0.483;
//создание переменной для контроля работы 
bool work_check = true;
//создание переменной для контроля выбранной опции с помощью кнопки выбора действия
int bt_check = 0;
//создание переменной для хранения значения, подаваемого на помпу подачи жидкости
int wt_pump = 0;
//создание переменной для хранения значения, подаваемого на газовый шаровой кран 
int prs_cntrl = 0;
//создание переменной для хранения названия выбранного действия
string line;
//создание переменной для хранения средней заданной поддерживаемой температуры в контейнере
float sr_t = 0;
//создание переменной для хранения заданного поддерживаемого давления жидкости
float fluid_pressure = 0;

//функция старта программы 
void setup()
{
  //настройка монитора порта
  Serial.begin(9600); 
  //настройка пина помпы подачи жидкости на "выход"
	pinMode(PIN_WATERPUMP, OUTPUT);
	//настройка пина куллера охлаждения на "выход"
	pinMode(PIN_FAN, OUTPUT);
	//настройка пина газового шарового крана на "выход"
	pinMode(PIN_GASTAP, OUTPUT);
	//настройка пина кнопки определения действия на "вход"
	pinMode(PIN_BTCONTROL, INPUT);
	//настройка пина кнопки прибавления значения на "вход"
	pinMode(PIN_BTPLUS, INPUT);
	//настройка пина кнопки убавления значения на "вход"
	pinMode(PIN_BTMINUS, INPUT);
	
	//запуск и инициализация жк экрана
	display.begin();
  attachInterrupt(0, swp, RISING);
}

//функция считывания данных с 2 датчиков температуры DS18b20
void temperature_read()
{
  //определение температуры 2 датчиков DS18b20
  //создание под хранилище начальных данных этих датчиков
  byte data[4]; 
  
  //сброс предыдущих команд и параметров 1 датчика DS18b20
  ds1.reset();
  //подача 1 датчику DS18b20 команды пропустить поиск по адресу 
  ds1.write(0xCC); 
  //подача 1 датчику DS18b20 команды измерить температуру. само значение температуры 
  //еще не получаем - датчик его определит во внутреннюю память
  ds1.write(0x44); 
  
  //сброс предыдущих команд и параметров 2 датчика DS18b20
  ds2.reset(); 
  //подача 2 датчику DS18b20 команды пропустить поиск по адресу 
  ds2.write(0xCC); 
  //подача 2 датчику DS18b20 команды измерить температуру. само значение температуры 
  //еще не получаем - датчик его определит во внутреннюю память
  ds2.write(0x44);
  
  //ожидание измерения в течении 1 секунды 
  delay(1000);  
  
  //подготовка к получению значения измеренной температуры датчика 1
  ds1.reset(); 
  ds1.write(0xCC); 
  //передача значения регистров со значением температуры датчика 1
  ds1.write(0xBE); 
  //подготовка к получению значения измеренной температуры датчика 2
  ds2.reset(); 
  ds2.write(0xCC); 
  //передача значения регистров со значением температуры датчика 2
  ds2.write(0xBE); 

  //получение ответа
  //считывание младшего байта значения температуры датчика 1
  data[0] = ds1.read(); 
  //считывание старшего байта значения температуры датчика 1
  data[1] = ds1.read(); 
  //считывание младшего байта значения температуры датчика 2
  data[2] = ds2.read(); 
  //считывание старшего байта значения температуры датчика 2
  data[3] = ds2.read(); 

  //формирование итоговых значений: 
  //    - сперва "склеиваем" значения для каждого из датчиков, 
  //    - затем умножаем их на коэффициент, соответствующий  
  //          разрешающей способности (для 12 бит по умолчанию - это 0,0625)
  temperature1 =  ((data[1] << 8) | data[0]) * 0.0625;
  temperature2 =  ((data[3] << 8) | data[2]) * 0.0625;

}

//функция контроля температуры жидкости
void water_temp_control(float t)
{
  //если температура выше или равна 36 градусам С, то включаем куллер охлаждения
  if(t >= 36)
  {
    digitalWrite(PIN_FAN, HIGH);
  }
  //иначе, если ниже или равна 35 градусам С, то отключаем куллер охлаждения
  else if(t <= 35)
  {
    digitalWrite(PIN_FAN, LOW);
  }
}

//функция контроля нагревательного элемента внутри контейнера
void heater_control(float t)
{
  //если температура в контейнере выше или равна 36 градусам С, то отключаем нагревательный эл.
  if(t >= 36)
  {
    digitalWrite(PIN_HEATER, LOW);
  }
  //иначе, если температура в контейнере ниже или равна 35 градусам С, то включаем нагрв. эл.
  else if(t <= 35)
  {
    digitalWrite(PIN_HEATER, HIGH);
  }
}

//функция определения давления обогащенной жидкости
void pressure_read()
{
  //датчик подключен к аналоговому пину A0
  //считывание вольтажа с датчика
  V = analogRead(PIN_WATERPRESSURE) * 5.00 / 1024;
  //вычисление давления обогащенной жидкости по считанному вольтажу и перевод в Па
  P = (V - OffSet) * 250 / 1000;                         
}

//функция контроля подачи газа
void gas_control()
{
  //подача переданного при вызове данной функции количества напряжения на пин,
  //       к которому подключен газовый шаровой кран
  digitalWrite(PIN_GASTAP, gas_pressure);
}

//функция контроля подачи жидкости в систему
void waterpump_control()
{
  //подача переданного при вызове данной функции напряжения на пин,
  //       к которому подключена помпа подачи жидкости
  digitalWrite(PIN_WATERPUMP, wt_pump);
}

//функция подачи звукового сигнала тревоги при нарушении работы
void zommer_alarm()
{
  //подача напряжения на пин, к которому подключен пьезо-излучатель
  digitalWrite(PIN_ZOMMER, 1);
  //включение задержки в 200мсек для продолжительности звукового сигнала
  delay(200);
  //отключение напряжения на пине, к которому подключен пьезо-излучатель
  digitalWrite(PIN_ZOMMER, 0);
  //включение задержки в 200мсек для паузы между звуковыми сигналами
  delay(200);
  //подача напряжения на пин, к которому подключен пьезо-излучатель
  digitalWrite(PIN_ZOMMER, 1);
  //включение задержки в 200мсек для продолжительности звукового сигнала
  delay(200);
  //отключение напряжения на пине, к которому подключен пьезо-излучатель
  digitalWrite(PIN_ZOMMER, 0);
  //включение задержки в 200мсек для паузы между звуковыми сигналами
  delay(200);
  //подача напряжения на пин, к которому подключен пьезо-излучатель
  digitalWrite(PIN_ZOMMER, 1);
  //включение задержки в 200мсек для продолжительности звукового сигнала
  delay(200);
  //отключение напряжения на пине, к которому подключен пьезо-излучатель
  digitalWrite(PIN_ZOMMER, 0);
  //включение задержки в 200мсек для паузы между звуковыми сигналами
  delay(200);
}

//функция сброса настраиваемых параметров
void work_reset()
{
  //обнуление переменной контроля выбранной опции с помощью кнопки выбора действия
  bt_check = 0;
  //обнуление переменной хранения значения, подаваемого на помпу подачи жидкости
  wt_pump = 0;
  //обнуление переменной хранения значения, подаваемого на газовый шаровой кран 
  prs_cntrl = 0;
  //обнуление переменной хранения названия выбранного действия
  line;
  //обнуление переменной хранения средней заданной поддерживаемой температуры в контейнере
  sr_t = 0;
  //обнуление переменной хранения заданного поддерживаемого давления жидкости
  fluid_pressure = 0;
}

//функция изменения выбранного параметра
void btcntrl_action()
{
  //переключение по выбранной опции изменения параметров
  switch(bt_check)
  {
    //изменение средней поддерживаемой температуры в контейнере
    case 0:
      //запись выбранной опции для вывода на жк экран
      line = "Средняя t";
      //изменение данных средней температуры через получение значения из функции button_action_read
      sr_t = button_action_read(sr_t);
      //завершение данного кейса
      break;
      
    //изменение заданного давление обогащенной жидкости
    case 1:
    //запись выбранной опции для вывода на жк экран
      line = "Заданное давление P";
      //изменение данных заданного давления жидкости через получение значения из функции button_action_read
      fluid_pressure = button_action_read(fluid_pressure);
      //завершение данного кейса
      break;
      
    //изменение скорости перфузии
    case 2:
    //запись выбранной опции для вывода на жк экран
      line = "Скорость перфузии";
      //изменение данных скорости перфузии через получение значения из функции button_action_read
      wt_pump = button_action_read(wt_pump);
      //завершение данного кейса
      break;
      
    //изменение скорости подачи газа
    case 3:
    //запись выбранной опции для вывода на жк экран
      line = "Скорость подачи газа";
      //изменение данных скорости подачи газа через получение значения из функции button_action_read
      gas_pressure = button_action_read(gas_pressure);
      //завершение данного кейса
      break;
      
    //запуск сброса заданных параметров  
    case 4:
    //запись выбранной опции для вывода на жк экран
      line = "Сброс параметров";
      //вызов функции сброса параметров
      work_reset();
      //завершение данного кейса
      break;
  }
}

//функция вывода данных на жк экран
void tft_lcd_writer()
{
  //очистка экрана от предыдущих данных
  display.clearScreen();
  //определение горизонтали расположения отображаемых элементов 
  display.setRotation(1);
  //определение размера отображаемого текста
  display.setTextSize(1);
  //определение цвета отображаемого текста
  display.setTextColor(YELLOW);
  //определение начала координат для отображаемого текста
  display.setCursor(0,0);
  //вывод текущей температуры жидкости на экран
  display.println(utf8rus("Температура жидкости: " + temperature1));
  //определение цвета отображаемого текста
  display.setTextColor(GREEN);
  //вывод текущей температуры воздуха в контейнере
  display.println(utf8rus("Температура воздуха в контейнере: " + temperature2));
  //определение цвета отображаемого текста
  display.setTextColor(RED);
  //вывод текущего давления обогащенной жидкости в системе
  display.println(utf8rus("Давление жидкости: " + P + " Па"));
  //определение цвета отображаемого текста
  display.setTextColor(WHITE);
  //вывод названия выбранного действия
  display.println(utf8rus("Выбранное действие: " + line));
  //вывод заданной средней поддерживаемой температуры контейнера
  display.println(utf8rus("Заданная ср. температура: " + sr_t));
  //вывод заданного поддерживаемого давления жидкости в системе
  display.println(utf8rus("Заданное давление: " + fluid_pressure));
  //вывод заданной поддерживаемой скорости перфузии
  display.println(utf8rus("Заданная скорость перфузии: " + wt_pump));
  //вывод заданной поддерживаемой скорости подачи газа
  display.println(utf8rus("Заданная скорость пподачи газа: " + gas_pressure));
  //пропуск строчки на экране
  display.println();
  
  //проверка работы системы
  //если значение "true", значит система работает исправно
  if(work_check == true)
  {
    //определение цвета отображаемого текста
    display.setTextColor(GREEN);
    //вывод сообщение на экран о норме в работе системы
    display.println(utf8rus("Работа в норме!"));
  }
  //если значение "false", значит система работает некорректно
  else
  {
    //определение цвета отображаемого текста
    display.setTextColor(RED);
    //вывод сообщения о нарушения в работе системы на экран
    display.println(utf8rus("РАБОТА НАРУШЕНА!!!"));
    //вызов метода звукового сигнала тревоги
    zommer_alarm();
  }
}

//функция контроля выбора изменяемого параметра
void button_contorl()
{
  //создание переменной для определения нажатия кнопки смены действия
  int bt_switch = digitalRead(PIN_BTCONTROL);
  
  //если кнопка была нажата и при этом значение в 
  //        переменной bt_check не превышает 5 возможных действий
  if((bt_switch == HIGH) && (bt_check < 5))
  {
    //то добавляем единицу к переменной bt_check
    bt_check++;
  }
  //иначе, если кнопка нажата и при этом значение в
  //        переменной bt_check превышает 5 возможных действий
  else if((bt_switch == HIGH) && (bt_check >= 5))
  {
    //то обнуляем переменную выбранного действия bt_check
    //      для того, чтобы "закольцевать" выбор возможных действий
    bt_check = 0;
  }
}

//функция добавления или вычитания значения в выбранном действии
int button_action_read(int bt_action)
{
  //создание переменной для определения нажатия кнопки добавления значения
  int bt_plus = digitalRead(PIN_BTPLUS);
  //создание переменной для определения нажатия кнопки вычитания значения
  int bt_minus = digitalRead(PIN_BTMINUS);
  
  //если кнопка добавления нажата и при этом значение в переменной не превышает лимита
  if((bt_plus == HIGH) && (bt_action <= 255))
  {
    //то добавляем значение 15 к предыдущему значению переменной bt_action
    bt_action += 15;
  }
  //если кнопка убавления нажата и при этом значение в переменной не ниже лимита
  if((bt_minus == HIGH) && (bt_action >= 0))
  {
    //то вычитаем значение 15 из предыдущего значения в переменной bt_action
    bt_action -= 15;
  }
  
  //передаем значение из переменной bt_action туда, откуда был совершен вызов данной функции
  return bt_action;
}

//основная функция программы, работающая в режиме бесконечного цикла
void loop()
{
  //вызов функции определения давления обогащенной жидкости в системе
  pressure_read();
  //вызов функции определения температур обогащенной жидкости и воздуха в контейнере
  temperature_read();
  //вызов функции контроля подачи жидкости в системе
  waterpump_control();
  //вызов функции контроля подачи газа в системе 
  gas_control();
  
  //если температура обогащенной жидкости не находится в пределах нужных значений
  if((temperature1 >= 36) || (temperature1 <= 34.5))
  {
    //то вызываем функцию контроля температуры жидкости с передачей в нее текущей температуры
    //    обогащенной жидкости
    water_temp_control(temperature1);
    //заявляем о нарушении работы системы
    work_check = false;
  }  
  //если температура воздуха в контейнере не находится в пределах нужных значений
  if((temperature2 >= 36) || (temperature2 <= 34.5))
  {
    //то вызываем функцию контроля температуры жидкости с передачей в нее текущей температуры
    //    воздуха в контейнере
    water_temp_control(temperature2);
    //и вызываем функцию контроля нагревательного элемента в контейнере с передачей в нее
    //    текущей температуры воздуха в контейнере
    heater_control(temperature2);
    //заявляем о нарушении работы системы
    work_check = false;
  }
  
  //вызываем функцию выбора изменяемого параметра
  button_contorl();
  //вызываем функцию изменения выбранного параметра
  btcntrl_action();
  //вызываем функцию вывода данных на жк экран
  tft_lcd_writer();
  //ставим задержку в 1 секунду для возможности считывания оператором
  //    данных системы
  delay(1000);
}