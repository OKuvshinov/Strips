# Strips

Программа для вычисления оптимального покрытия множества различными сетками

В качестве покрываемого множества может выступать овал Кассини с различными параметрами, квадрат, треугольник

Сеткой может являться замощение: квадратами; шестиугольниками; кругами, описанными вокруг квадратов; кругами, описанными вокруг шестиугольников

Реализован поиск оптимального покрытия по двум критериям:
1. минимум хаусдорфова расстояния между сеткой и покрываемым множеством
2. минимум числа элементов сетки, необходимых для полного покрытия

Алгоритм поиска оптимального покрытия заключается в переборе всех возможных позиций покрываемого множества относительно сетки с фиксированным шагом смещения и углом поворота

В программе предусмотрено масштабирование изображения. Для реализации алгоритма используется открытая библиотека clipperlib: http://www.angusj.com/clipper2/Docs/Overview.htm
