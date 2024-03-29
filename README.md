# Transport Catalogue

Проект "Transport Catalogue" поддерживает графический вывод, поиск маршрутов и расчет времени в между остановками.

## Описание работы:

"Транспортный каталог" представляет собой приложение, которое принимает входные данные остановок и маршрутов в формате JSON. На основе этих данных оно создает маршруты и предоставляет возможность пользователю указать начальную и конечную точку для запроса оптимального маршрута по времени. После обработки запросов программа выводит время наилучего маршрута или же выводит изображение маршрута с svg расширением.

#### Параметры запуска программы:

* `make_base` - создание базы данных транспортного справочника на основе запросов `base_requests` и ее сериализация в файл.

* `process_requests` - десериализации базы данных из файла и использования ее для ответа на запросы `stat_requests`.

## Сборка и установка
Сборка с помощью любой IDE либо сборка из командной строки. Требуется соблюдать системные требования.

## Системные требования
Компилятор С++ с поддержкой стандарта C++17 и выше, а также CMAKE и Protobuf.