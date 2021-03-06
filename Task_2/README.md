# Практическое задание №2

**Задача:** Реализовать метод построения конкордансов и вычисление частот
совместной встречаемости.  
**Исходный код:** https://github.com/kkmoskalenko/cl_course/tree/main/Task_2

## Ход решения

В решении используется модуль `Parser` из предыдущего задания: он отвечает за загрузку словаря в память, а также токенизацию текстов корпуса и искомой фразы. Знаки препинания считаются самостоятельными токенами.

Далее для каждого текста корпуса происходит итерация по его токенам, которые вначале нормализуются, а затем добавляются в связный список размера *(размер окна + длина искомой фразы)*. Неизвестные токены считаются нормализованными и добавляются в словарь с пометкой `UNKNOWN`. Если фраза в связном списке подходит под определение левого или правого контекста (т.е. она имеет хотя бы по одной общей лемме для каждого токена с искомой фразой), будут сгенерированы и подсчитаны все возможные комбинации лемм для данного контекста.

## Результаты работы

1. Фраза: `фильм`, размер окна = `2`:

   | Левые контексты             | Правые контексты            |
   | --------------------------- | --------------------------- |
   | L, » . ФИЛЬМ, 61            | R, ФИЛЬМ И СЕРИАЛ, 83       |
   | L, . В ФИЛЬМ, 60            | R, ФИЛЬМ РАССКАЗАЛ О, 53    |
   | L, . ПРЕМЬЕРА ФИЛЬМ, 58     | R, ФИЛЬМ , КОТОРЫЙ, 51      |
   | L, . ПРЕМЬЕР ФИЛЬМ, 58      | R, ФИЛЬМ ГОД ., 37          |
   | L, РОЛЯ В ФИЛЬМ, 41         | R, ФИЛЬМ — «, 30            |
   | L, РОЛЬ В ФИЛЬМ, 41         | R, ФИЛЬМ . В, 29            |
   | L, , ЧТО ФИЛЬМ, 40          | R, ФИЛЬМ ОСНОВАН НА, 27     |
   | L, УЧАСТИЕ В ФИЛЬМ, 30      | R, ФИЛЬМ « МСТИТЕЛЬ, 27     |
   | L, . РЕЖИССЁР ФИЛЬМ, 27     | R, ФИЛЬМ , НО, 26           |
   | L, ИЗВЕСТНЫЙ ПО ФИЛЬМ, 26   | R, ФИЛЬМ : «, 24            |
   | L, ПЕРВЫЙ ТРЕЙЛЕР ФИЛЬМ, 25 | R, ФИЛЬМ « ЧЕЛОВЕК-ПАУК, 23 |
   | L, В НОВЫЙ ФИЛЬМ, 23        | R, ФИЛЬМ , А, 22            |
   | L, В НОВОЕ ФИЛЬМ, 23        | R, ФИЛЬМ « МАТРИЦА, 20      |
   | L, САМЫЙ КАССОВЫЙ ФИЛЬМ, 21 | R, ФИЛЬМ , В, 20            |
   | L, . ПОСТАВИЛ ФИЛЬМ, 20     | R, ФИЛЬМ « ЧЁРНЫЙ, 19       |
   | L, КАДР ИЗА ФИЛЬМ, 19       | R, ФИЛЬМ ПО МОТИВ, 18       |
   | L, КАДР ИЗ ФИЛЬМ, 19        | R, ФИЛЬМ НА ИНОСТРАННЫЙ, 17 |
   | L, « ЛУЧШИЙ ФИЛЬМ, 19       | R, ФИЛЬМ ВЫШЕЛ В, 17        |
   | L, ТАКЖЕ В ФИЛЬМ, 17        | R, ФИЛЬМ НАМЕЧЕН НА, 16     |
   | L, В ЭТОТ ФИЛЬМ, 17         | R, ФИЛЬМ « ЗВЁЗДНЫЙ, 16     |
   | L, В ЭТО ФИЛЬМ, 17          | R, ФИЛЬМ « ДЮНА, 16         |
   | L, . ПЕРВЫЙ ФИЛЬМ, 17       | R, ФИЛЬМ ПРИНЯЛ УЧАСТИЕ, 15 |
   | L, СЕРИАЛ И ФИЛЬМ, 16       | R, ФИЛЬМ «, 15              |
   | L, РАБОТА НАД ФИЛЬМ, 16     | R, ФИЛЬМ С УЧАСТИЕ, 14      |
   | L, ОТЗЫВ НА ФИЛЬМ, 16       | R, ФИЛЬМ ГАЙ РИЧИ, 14       |



2. Фраза: `дата выхода`, размер окна = `2`:

   | Левые контексты                   | Правые контексты              |
   | --------------------------------- | ----------------------------- |
   | L, ВТОРОЙ СЕЗОН ДАТА ВЫХОД, 69    | R, ДАТА ВЫХОД : НОЯБРЬ, 67    |
   | L, ВТОРА СЕЗОН ДАТА ВЫХОД, 68     | R, ДАТА ВЫХОД : ЯНВАРЬ, 66    |
   | L, ТРЕТИЙ СЕЗОН ДАТА ВЫХОД, 33    | R, ДАТА ВЫХОД : АВГУСТА, 64   |
   | L, ЧЕТВЁРТЫЙ СЕЗОН ДАТА ВЫХОД, 21 | R, ДАТА ВЫХОД : АВГУСТ, 64    |
   | L, ШЕСТОЙ СЕЗОН ДАТА ВЫХОД, 9     | R, ДАТА ВЫХОД : ГОД, 61       |
   | L, , СЕЗОН ДАТА ВЫХОД, 9          | R, ДАТА ВЫХОД : СЕНТЯБРЬ, 60  |
   | L, ПЯТЫЙ СЕЗОН ДАТА ВЫХОД, 7      | R, ДАТА ВЫХОД : ОКТЯБРЬ, 59   |
   | L, . ТОЧНЫЙ ДАТА ВЫХОД, 5         | R, ДАТА ВЫХОД : МАЙ, 59       |
   | L, ! » ДАТА ВЫХОД, 5              | R, ДАТА ВЫХОД : ИЮЛЬ, 59      |
   | L, ЧЕЛОВЕК » ДАТА ВЫХОД, 4        | R, ДАТА ВЫХОД : МАЯ, 58       |
   | L, ЗЕМЛЯ » ДАТА ВЫХОД, 4          | R, ДАТА ВЫХОД : ДЕКАБРЬ, 55   |
   | L, ДРАКОН » ДАТА ВЫХОД, 4         | R, ДАТА ВЫХОД : ИЮНЬ, 48      |
   | L, ГЛАЗ » ДАТА ВЫХОД, 4           | R, ДАТА ВЫХОД : АПРЕЛЬ, 46    |
   | L, , СПЕЦЭПИЗОД ДАТА ВЫХОД, 4     | R, ДАТА ВЫХОД : ФЕВРАЛЬ, 44   |
   | L, ) . ДАТА ВЫХОД, 4              | R, ДАТА ВЫХОД : МАРТА, 44     |
   | L, УМИРАТЬ » ДАТА ВЫХОД, 3        | R, ДАТА ВЫХОД : МАРТ, 44      |
   | L, ТИЗЕР И ДАТА ВЫХОД, 3          | R, ДАТА ВЫХОД ПОКА НЕ, 16     |
   | L, СТАЛ ИЗВЕСТЕН ДАТА ВЫХОД, 3    | R, ДАТА ВЫХОД ФИЛЬМ «, 6      |
   | L, РОГ » ДАТА ВЫХОД, 3            | R, ДАТА ВЫХОД ПОКА НЕТ, 6     |
   | L, ПРИЗРАК » ДАТА ВЫХОД, 3        | R, ДАТА ВЫХОД ВТОРОЙ СЕЗОН, 5 |

   