# AntestL Backend
AntestL Backend - это часть программного обеспечения AntestL, которая
позволяет организовать измерение параметров антенн, с помощью ВАЦ, 
внешнего генератора и ОПУ. Взаимодействие осуществляется с помощью
заданий или списков заданий.

## Сборка
Сборка осуществлялась с помощью [MinGW-w64 11.0.0](https://github.com/skeeto/w64devkit/releases/download/v1.19.0/w64devkit-1.19.0.zip)
~~~bash
cmake.exe --build AntestL-Backend-Cpp\cmake-build-debug --target antestl_backend -- -j 6
~~~

## Запуск
В AntestL Backend предусмотрены следующие параметры запуска:

| **Параметр** | **Сокращённый <br/>вариант** | **Описание**                                                                                                                                                                                                                                                                                                  |  
|:------------:|:----------------------------:|:--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
|     -log     |              -l              | Изменяет уровень логирования. <br/> Все сообщения, уровень которых<br/>выше, чем установленный уровень, игнорируются.<br/><dl><dt>Возможные варианты:</dt><dd><ul><li>error (0)</li><li>warn (1)</li><li>info (2)</li><li>debug (3)</li><li>trace (4)</li><ul></dd></dl><br/>По-умолчанию выбран уровень info |
|    -task     |              -t              | Изменяет порт для сокета, который отвечает за приём заданий. <br/>По-умолчанию выбран порт 5006                                                                                                                                                                                                               |                                                                                                                                                                                                                                                                                     
|    -data     |              -d              | Изменяет порт для сокета, который отвечает за передачу результатов. <br/>По-умолчанию выбран порт 5007                                                                                                                                                                                                        |