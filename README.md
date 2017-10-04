# myMouse: Mouse filter driver

Драйвер-фильтр для USB мыши. Инвертирует ось Y, т.е. при движении мыши вверх курсор движется вниз и наоборот. 

## Файлы для установки

**myMouse.cer** - самоподписанный сертификат<br />
**myMouse.inf** - установочный файл<br />
**myMouse.cat** - установочная папка<br />
**myMouse.sys** - бинарный код драйвера<br />

## Установка и откат

Чтобы установить драйвер:
1. Включите тестовые подписи: запустите командную строку (cmd.exe) от имени администратора, введите
```bash
> bcdedit /set TESTSIGNING ON
> bcdedit /set nointegritychecks ON
```
и перезагрузите компьютер. Альтернативный способ:
  * Нажмите сочетание клавиш Windows + I.
  * Выберите Выключение, зажмите (и держите) клавишу Shift и выберите Перезагрузка.
  * На экране Выбор действий нажмите Диагностика.
  * Далее нажмите Дополнительные параметры.
  * Выберите Параметры загрузки и нажмите Перезагрузить.
  * Нажмите F7.
2. Нажмите правой кнопкой мыши на сертификат и выберите Установить сертификат.
3. Нажмите правой кнопкой мыши на установочный файл и выберите Установить, затем Все равно установить этот драйвер.
4. Откройте Диспетчер устройств (Device manager), откройте Свойства для мыши и на вкладке Драйвер нажмите кнопку Обновить. Потом здесь же можно будет Откатить установку. Выберите Поиск и установка драйверов вручную, затем Выбрать драйвер из списка уже установленных драйверов. Выберите в списке Stasia Filtered USB Device или нажмите Установить с диска, введите адрес папки FilterDriver с .inf файлом и нажмите ОК. Нажмите Далее. Готово!
5. Теперь вы можете вернуть проверку подписей:
```bash
> bcdedit /set TESTSIGNING OFF
> bcdedit /set nointegritychecks OFF
```
Изменения вступят в силу после перезагрузки.

## Работа драйвера

Чтобы включить инверсию оси Y, поместите курсор в крайнее левое положение (в любую точку левого края экрана) и, не изменяя положение курсора, щелкните последовательно правой кнопкой мыши, затем левой, затем снова правой. Готово! Проделайте те же действия для отмены инверсии.
