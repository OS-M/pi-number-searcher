# Pi Number Searcher
### Обзор
*Вы знали что число 360 находится на 360-ой позиции числа Pi?*<br>
Проект представляет собой консольное приложение для поиска подстроки в строке за время *O(|t| + |s|)* с использованием префиксной функции и многопоточного программирования.<br>
Изначально программа писалась чтобы находить интересные подпстроки числа Pi (так как текстовые редакторы плохо работают с txt-файлом размером 1гб), например год рождения или номер мобильного. Потом проект перерос в небольшую исследовательскую работу по многопоточному программированию.<br>

----------------------------------
### Выводы
Поищем разные строки в 1гб первых цифр [числа Pi](https://drive.google.com/file/d/1e6SEAWjzPRLk2fCKKECrbZJzApp0qjen/view?usp=sharing).<br>
Тесты проводились на:
- Ubuntu 20.04
- Intel Core i5-10400F (12 threads, 4GHz)
- HyperX Predator 16Gb DDR4 (2666MHz 13-15-15)

Длина строки \ Количество потоков |   1   |   2   |   4   |   8   |  12  
|             :---:               | :---: | :---: | :---: | :---: | :---:
**1** | 7.4 | 4.2 | 2.4 | 1.8 | 1.8
**2** |  6  | 3.3 | 1.7 | 1.2 | 1.1
**4** |  6  | 3.1 | 1.7 | 1.1 | 1
**8** |  6  | 3.1 | 1.7 | 1.1 | 1
**16**| 5.8 | 3.1 | 1.7 | 1.1 | 1

-----------------------------
Пока мы искали строки малой длины, мы находили много вхождений и, соответственно, между потоками была так называемая "гонка" за доступ к массиву с ответами, а когда строки стали длинее, алгоритм стал находить несколько вхождений и многопоточность стала давать б*о*льший прирост. А малую корреляцию с длиной искомой строки объяснить еще проще: так как алгоритм работает за линейное от длины строк время, а длина искомой строки составляет очень малую часть от длины текста (в моих тестах это было 1000000003 символов), мы видим малую зависимость времени выполнения от длины искомой строки.<br>
Также нужно отметить, что большее (в разумных количествах, я тестировал до 50) количество потоков не дает прироста, но и почти не замедляет выполнение благодаря планировщику ОС.

---------------------------------
### Те самые интересные вхождения
- "**360**": позиции 360, 1414 (всего нашлось 1000211 вхождений)
- "**03062020**": позиция 160640504 (всего нашлось 9 вхождений)
