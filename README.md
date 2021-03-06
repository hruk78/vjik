
# vjik
Этот проект продолжение на языке си проекта 
https://github.com/ArtemKasianov/ICML

Построение Модели
0) Изначально запускается скрипт на Perl, который строит исходные директории и модель с заданными параметрами либо с помощью скрипта на питоне, либо его аналога, написанного на С. Запуски изначального скрипта
perl oldICML.max.min.only_predict.fast.5K.pl ATH_MAP_INFECTION_LEAF_ROOT_STRESS_VITAMIN.txt ZEA_MAP_INFECTION_LEAF_ROOT_STRESS_VITAMIN.txt Ath_vs_Mazie.gt_0.sim orthopairs.ath.zm.txt 0 100
1 и 2 - карты экспрессии генов
3 - рудиментарный параметр, оставленный для обратной совместимости
4 - списко отропар
5 и 6 количесвто итераций для набора отрицательной выборки.

a) XGBoostTree.saveModel.iterPy3.py переписана на третьем Python.
Запуск скриптом oldICML.max.min.only_predict.fast.5K.pl
параметры внутри скрипта
1 - max_depth;
2 - eta;
3 - subsample;
4 - colsample_bytree;
5 - colsample_bylevel;
6 - min_child_weight;
7 -gamma;
8 -alpha;
9 - lambdaParam;
10 - train iteration number;
11 - eval_metric;
12 - scale_pos_weight;
13 - numberOfProcessor Cores
далее разные директории с данными,
последние два параметра - это параметра формата генов в текстовых фалов, есть ли табуляции или пробелы в конце первой группы генов и второй, если есть - оба нули. 

б) Или скрипт на С ./makemodel из исходников makemodel.c, который запускается скриптом 
2ICML.max.min.only_predict.Jan2021.pl. В этом скрипте на 13-ый параметр количество потоков хgboost. Добавлен параметр количества потоков для хgboost.

параметры
1 - max_depth;
2 - eta;
3 - subsample;
4 - colsample_bytree;
5 - colsample_bylevel;
6 - min_child_weight;
7 -gamma;
8 -alpha;
9 - lambdaParam;
10 - train iteration number;
11 - eval_metric;
12 - scale_pos_weight;
13 - numberOfProcessor Cores
далее разные директории с данными
последние два параметра - это параметра формата генов в текстовых фалов - оставлены для обратной соместимости и сейчас в Си-алгоритме не используются, так как вопрос решается автоматически - лишние символы табуляции и пробелы удалаятся. 

Пример модели в фармате xgboost в файле model_1_1_1_1000.test 

Генерация геномных пар для предсказаний

Для этого скрипт
MakeAllPairs.с
Параметры запуска, последние два параметра - номер начальной и конечной директорий с построенными модялями, в данном случае от iter_0 до iter_100. Они не обязательны в этом скрипте, но я оставляю их, чтобы помнить какая серия запусков происходит:
./MakeAllPairs ATH_MAP_INFECTION_LEAF_ROOT_STRESS_VITAMIN.txt ZEA_MAP_INFECTION_LEAF_ROOT_STRESS_VITAMIN.txt pairs10_84.txt 0 100

Все парметры генерации моделистандартные, кроме параметра 12 - scale_pos_weight, который определяется соотношениям количества пар генов в негативной выборке к позитивной, которое в данном случае равняется 84 и параметра 1 - max_depth, глубина 10. С этими параметрами размера модели 500 кб. Предыдущий запуск со значениями 1 и 1 соответсвенно давал модель размером 180 кб.
Предыдущий вариант запуска приводил к тому, что граф был перегружен связями и либо находились два кластера генов: кластер альбидопсиса и кластер кукурузы, либо количество синглетонов составляло от 40% до 75% в зависимости от того, как изменялись параметры отбрасывания лишних связей и порог отсечики (0.5). Поэтому было решено попробовать новые параметры.

./makemodel 10 0.3 1 1 1 1 0 4 1 100 auc 84 20 
iter_$iter/results/training/expression 
$firstSpExpressionFile 
$secondSpExpressionFile 
iter_$iter/results/data_for_learning/folds 
iter_$iter/results/data_for_learning/negative_folds 
0 0

На случай нехватки памяти или диска - для данных геномных карт получившийся файл будет около 40Gb - были написаны два похожих скрипта, для генерации первой порции MakePairs.c и порции оставшихся MakePairsLastOnes.c, где последним параметром добавляется количество пар. Для первого скрипта это количество, которе будет сгенерированно, а для второго это количество с которого начнется генерация, чтобы закончиться до конца. В примере ниже генерировалось 1.2 гигопар.
./MakePairs ATH_MAP_INFECTION_LEAF_ROOT_STRESS_VITAMIN.txt ZEA_MAP_INFECTION_LEAF_ROOT_STRESS_VITAMIN.txt pairs.txt 0 24 1200000000
./MakePairsLastOnes ATH_MAP_INFECTION_LEAF_ROOT_STRESS_VITAMIN.txt ZEA_MAP_INFECTION_LEAF_ROOT_STRESS_VITAMIN.txt pairs.txt 0 24 1200000000
Подход генерить пары порциями порождает необходимость строить предсказния порциями, а потом суммировать эти предсказания в общем файле. Скрипты для этого были написаны, но понадобяться они в случае увеличения объёма данных. В данном случае можно все делать одной порцией. 

Все три скрипта построения списка геномных пар требуют большого количества оперативной памяти, желательно не меньше 100 Гб. На случай тестов с меньшим количеством памяти, а так же на случай больших объемов данных написан скрипт MakeAllPairsLowMem.c, который экономит память за счет скорости.  

Вслучае карт альбидопсиса и кукурузы получается 39005x33322 геномных пар для комбинация каждого гена с каждым, и файл пар pairs.txt составляет 30988 MB 

Следующий шаг построение предсказаний с помощью скрипта
./predictAllByPortion ATH_MAP_INFECTION_LEAF_ROOT_STRESS_VITAMIN.txt ZEA_MAP_INFECTION_LEAF_ROOT_STRESS_VITAMIN.txt pairs.txt 0 100 20000000 70
Параметры:
1,2 - геномные карты
3 - файл пар,
4 - начальная итерация iter_0
5 - конечная итерация iter_100
6 - какими порциями читать геномные пары, этот параметр подбирается исходя из количества доступной оперативной памяти,
в данном случае предсказания делались порциями по 20 млн пар, при этом расход оперативной памяти был около 1 тб. Чем меньше порции, тем дольше работает скрипт. На узле node36 макарьича на 70 ядрах генерация предсказаний заняла около 75 часов, памяти потребовалось около 900 ГБ.

Так же есть более продвинутая версия этого скрипта, в которой чтения файла пар для предсказаний организвоно немного рациональней, что важно в случае разбиения порций предсказаний на большее число порций. Данная версия тестировалась на ноутбуке с памятью 32 Гб, 
prdctByOneProc.c
 ./prdctByOneProc.exe ATH_MAP_INFECTION_LEAF_ROOT_STRESS_VITAMIN.txt ZEA_MAP_INFECTION_LEAF_ROOT_STRESS_VITAMIN.txt pairs10_84.txt 0 1 600000 3
Второй скрипт будет необходим в случае увеличения объёма данных на 1-2 порядка.

Update:
В следующей ревизии скрипта prdctByOneProc.c удалось оптимизировать использование памяти у увеличить КПД в три раза, в итоге при большей в четыре раза пориции данных, а именно при 80 000 000 пар за раз объём используемой оперативной памяти составил 1.4 ТБ.

В резульате должно получится количество файлов предсказаний, равное числу итераций, в каждой из которых бралась своя отрицательная выборка. Все файлы должны быть идентичны по размеру и по количеству строк. При последнем запуске на сервере сбоила распределенная файловая ситема хранения, в результате часть файлов оказалась битая. Это выражалось в том, что были файлы меньшего размера. Это недопустимо для дальнейшей работы, потому что следующим шагом строится медина предсказаний для каждой пары. Чтобы исключить поиск каждой пары в каждом из 40 гигобайтных файлов, которых в нашем случае 101, все файлы отсортированны и находятся в одних и тех же строчках во всех файлах. Поэтому прежде чем строить медиана надо проверить все файлы на одинаковое количество строк.

Это можно сделать скриптом 
find -name "expression.other.predictions" -print0 -type f | sort -z | xargs -0 wc -l

   1299724610 ./iter_0/results/predictions/expression.other.predictions
   1299724610 ./iter_100/results/predictions/expression.other.predictions
   1299724610 ./iter_10/results/predictions/expression.other.predictions
   1240431582 ./iter_11/results/predictions/expression.other.predictions
   1299724610 ./iter_12/results/predictions/expression.other.predictions
.....
   1299724610 ./iter_99/results/predictions/expression.other.predictions
   1299724610 ./iter_9/results/predictions/expression.other.predictions
 131212892582 total

Так же можно проверить данные быстрее с помощью оценки размеров, это менее информативно, чем подсчет строк, количество которых должно быть равно количеству строк в файле исходных пар, зато быстрее. Однако это менее надежный способ, потому что если по какой-то причине ошибка записи затронула запис для каждой иттерации одинаково по количеству потерянных строк, этот быстрый метод не покажет ошибки. Впрочем, это маловероятно.

find . -type f -name "expression.other.predictions" -exec du -sh {} ';' | sort -rh

В данном случае мы видим, что файл предсказаний в в iter_11 отличается от остальных, это означает, что для него надо запусить предсказания снова командой
./predictAllByPortion ATH_MAP_INFECTION_LEAF_ROOT_STRESS_VITAMIN.txt ZEA_MAP_INFECTION_LEAF_ROOT_STRESS_VITAMIN.txt pairs.txt 11 11 80000000 70 , то есть с 11 итерации по 11. Скрипт отработает недостающую директорию. Перед запуском надо удалить старый файл предсказаний, так как скрипт из осторожности не удаляет стаырые файлы предсказаний, а дозаписывает их в имеющийся.

Данный подход так же при необходимости позволит уменьшить размер файлов предсказаний в шесть раз, так как нужды в первых двух столбцах генов нет, достаточно одно файла сгенеренных пар генов pairs.txt. Кроме этого при необходимости можно дописать чтение и запись файлов сразу в bzip формате, возможно даже в распаралеленном виде, это снизит потреблене диска примерно в десять раз, соотвественно повысит скорость работы с файлами. Это важно, потому сейчас работа с диском - это бутылочное горлышко процесса предсказаний. Сами предсказания распаралеленны xgboost на Си-уровне и работают на 70 ядрах быстро. При желании для предсказаний можно использовать видеокарты - для этого надо раскоментить две строчки кода перед компиляцией, но опыт показывает, что если выбирать, то важнее многоядерность процессора, чем наличии видео карты. По моему мнению временные затраты на загрузки в память видеокарты 1тб матричных данных и выгрузки предсказаний многими порциями из и обратно в оперативную памятт компьютера сводит на нет выгоду от использования видеокарты. В результате я наблюдал выгоду всего раза в два, но этот результат не точен, так как тестирование происходило на разных узлах и коэффицент ускорения высчитывался эмпирически с учётом разности узлов.  

Далее надо построить медиану предсказаний. Это делает скрипт mediana.c. 
./mediana ATH_MAP_INFECTION_LEAF_ROOT_STRESS_VITAMIN.txt ZEA_MAP_INFECTION_LEAF_ROOT_STRESS_VITAMIN.txt pairs10_84.txt 0 100 0 1
параметр 1,2 - изначальные файлы геномных карт, в скрипте не используются, оставленны для иллюстрационных целей, и чтобы не путаться в параметрах, задание которых однотипно для всех скриптов.
параметр 3 - это файл геномных пар, для которых надо найти медиана. Для скорости работы скрипта проверки соответствия геномных пар исходного файла и файлов предсказаний не проверяется, поэтому такое соответствие, а так же равенство размеров оставляется на совесть оператора. В качестве этого файла может использоваться и один из файлов пресказаний expression.other.predictions, естественно с указанием его полного пути, при чтении пар генов скрипт читает только первые два столбца с генами, все осальные столбцы отбрасывает, в случае чтения вместо файла генных пар, файла предсказаний, он отбрасывает третий столбец весов.
параметры 4,5 - это начальная и конечная итерации предсказаний: от iter_n до iter_N. При равенстве n и N обрабатывает одну итерацию. 
параметр 6 - порции чтения (не тестировался), если ноль то читает все без перерыва.
параметр 7 - количеств потоков, всегда 1, оставлено для совместимости и иллюстраций, и на будущее, если будет желание организовать паралельное чтени и постройку медиан. При данной архитектуре расчетного узла это смысла не имело из-за крайне медленной дисковой системы.

Скрипт открывает сразу все файлы предсказаний, читает из каждого веса для соответствующей паре генов, строит по ним медиану и записывает её в файл mediana.txt. Поскольку в данном примере скрипт открывает 101 файл, каждый из которых 40Gb, задача требует много памяти, выделение которой зависит от работы оперативной системы, а именно от организации ею процесса открытия файлов. При двух одинаковых запусках на разных узлах я наблюдал расход 800 Гб (примерно половину - это занимал системный кэш) в первом и около 350 GB при втором запуске. Я не проводил допольнительных исследований по затратам ресурсов, однако если памяти не хватает, то в программе реализовано чтение кусками - 6 параметр. Однако я этот режим работы скрипта не тестировал и, возможно, он не даст выигрыша в памяти, так как её использование происходит помимо скрипта. На случай нахватки памяти я бы рекомендовал разбивать итерации предсказаний на равные порции, строить медианы для каждой, а потом строить медианы из медиан. Предполагаемый алгоритм этого без изменения скрипта mediana.c для рассморенного примера со 100 итерацией варьирования отрицательной выборки:
1) Запустить построение медианы для первых 10 директорий: iter_0..iter_9
2) После отработки переименовать в expression.other.predictions и переписать получившийся mediana.txt в заведомо большую итерацию, например, iter_1000/results/predictions - директори надо будет создать.
3) Сделать это для оставшихся 9 порций по десять директорий.
4) В итоге получится десять директорий с медианами из 10 итераций, iter_1000..iter_1010, для них запустить опять построение медианаы из 10 итераций.

Время работы скрипта можно оценить из возможностей дисковой системы, сколько времени понадобиться, чтобы прочесть 40ГБх101+записать 40ГБ. В случае второго тестирования на узле Макарьича node35 и страдртной его же дисковой системы 5.3 гб предсказаний (1/7 всего размера) заняла 4.5 часа. Ориентировочное время построения медиана - 30-40 часов в зависимости от сторонней нагрузки на файловую систему.

В итоге получился файл медианы предсказаний весов для всех генных пар. Далее смотрим кластеризация с помнощью пакета скриптов, в которых реализован метод маркова для поиска кластеров в графе. Описание и сами скрипты на сайте 
https://micans.org/mcl/

Далее можно провести догенерацию пар для положительной и отрицательной выборок, чтобы построить ROC кривую и дописать отдельно сгенерённые значения для ортопар и отрицательной выборки в файл весов геннных пар, однако эту рутинную процедуру стоит делать только в том случае, если предварительно будут найдены в необходимом количестве кластера. 

Пока же мы проверим наличие кластеров. Для этого генерим двай фала, один файл с матрицей весов для всех пар генов, а другой с обозначениями. Это сократит размер данных от 40 ГБ до 20 ГБ и преобразует их к универсальному матричному виду.

mcxload -abc medians.txt --stream-mirror --write-binary -write-tab medians.tab -o medians.mci
Далее ищем кластеры при разных инфляциях, на 48 ядрах, памяти требуется 20 ГБ, по размеру файла с матрицей medNewParFeb.mci 

mcl medNewParFeb.mci -t 48 -I 10.0 -use-tab medNewParFeb.tab -o medBnI100.txt
mcl medNewParFeb.mci -t 48 -I 8.0 -use-tab medNewParFeb.tab -o medBnI90.txt
mcl medNewParFeb.mci -t 48 -I 8.0 -use-tab medNewParFeb.tab -o medBnI80.txt
mcl medNewParFeb.mci -t 48 -I 7.0 -use-tab medNewParFeb.tab -o medBnI70.txt
mcl medNewParFeb.mci -t 48 -I 6.0 -use-tab medNewParFeb.tab -o medBnI60.txt
mcl medNewParFeb.mci -t 48 -I 5.0 -use-tab medNewParFeb.tab -o medBnI50.txt
mcl medNewParFeb.mci -t 48 -I 4.0 -use-tab medNewParFeb.tab -o medBnI40.txt
mcl medNewParFeb.mci -t 48 -I 3.0 -use-tab medNewParFeb.tab -o medBnI30.txt
mcl medNewParFeb.mci -t 48 -I 2.0 -use-tab medNewParFeb.tab -o medBnI20.txt
mcl medNewParFeb.mci -t 48 -I 1.8 -use-tab medNewParFeb.tab -o medBnI18.txt
mcl medNewParFeb.mci -t 48 -I 1.0 -use-tab medNewParFeb.tab -o medBnI10.txt

К сожалению результаты после потроения модели по последним параметрам
./makemodel 10 0.3 1 1 1 1 0 4 1 100 auc 84 20 
показывают, что модель всё ещё не натринерованна в должной мере. Это выдно пот тому, сколько генов остаётся в гигантских кластерах и в синглетонах. Наша цель, чтобы и первый и вторых типов генов было, как можно меньше. При любых параметрах кластеризации находится только два примерно равных кластера, один - кукурузы, другой - альбидопсиса и от 324 до 5 000 синглетонов. По-видимому scale_pos_weight=84 был недостаточно высок. Он вычислялся из соображений, что суммарная отрицательная выборка по всем 101 иттерациям в 84 раза больше количества отропар. В статье на основе которой подибраются параметры этот параметр брался в 10-1000 раз выше. Возможно, так же, что проблемы из-за недостаточной глубины дерева, котрая была 10. В результате следующую модель строим с другими параметрами, scale_pos_weight=2, max_depth=7, кол-во итераций пусть будет 75,
./makemodel 7 0.3 1 1 1 1 0 4 1 75 auc 2 20 

Кроме этого сложности с кластеризацией могут быть из-за некоректных предскзаний для ортопар и для и для отрицаетльной выборки, которые надо строить инчаче, чем предсказания для остальных генных пар. Поэтому написаны два скрипта, которые строят предсказания отдельно для этих обеих групп. Так же эти данные можно использовать для постройки ROC-кривой. Параметры - два первый файл просто для информации дебаг-выдачи, далее от начальной до конечной итерации и количество потоков.
./PositivPairsMedianaForROC ATH_MAP_INFECTION_LEAF_ROOT_STRESS_VITAMIN.txt. ZEA_MAP_INFECTION_LEAF_ROOT_STRESS_VITAMIN.txt 0 100 0 4
В результата получится необходимый файл ортопар orthoMediana.txt, отсортированный по первому и второму столбцу, а так его тёзки с расширениями sor1 и sort2, где во втором сортировака по второму и первому столбцу, а в третьем сортировка по столбцу весов. Эти файлы можно использовать для валидации результатов.
./NegativPairsForRoc ATH_MAP_INFECTION_LEAF_ROOT_STRESS_VITAMIN.txt. ZEA_MAP_INFECTION_LEAF_ROOT_STRESS_VITAMIN.txt 0 100 0 4
В результата получиться необходимый нам файл ортопар negMediana.txt всё остальное по аналогии с предыдущим файлом. Негативные и орто выборки предсказаны по методу кроссвалидации, из статьи.

Следующим шагом эти данные надо вставить с заменой в уже построенную медиану предсказаний по всем иттерациям. Это делается скриптом. CopyPredToMedWithChange.c 
Скрипт перезаписывает имеющиеся данные, а скорость этого копирования зависит от большего файла, поэтому копиравать надо меньшие в большие файле, а копировать в файль мишень в последнюю очередь. В нашем случае это:

CopyPredToMedWithChange orthoMediana.txt negMediana.txt
Полученный файл надо будет переименовать в файл, например, orthoAndNegMed.txt, после чего запусить уже копирование данных в исходную медиану, которая как мы помним размером в 40 ГБ, поэтому должно быть достаточно памяти. Для надёжности я беру в 2.5 с половиной раза больше, чем файл, то есть 100 ГБ. Этот скрпит серийный, поэтому работает на одном ядре.  

Если скрипт вставляет новую пару, то выдача в процессе работы такая
Insert StrCol1[3689]=AT4G10180  StrCol2[3689]=Zm00001d020989    wght=0.99       to Dest 354156
То есть вставлена новая генетическая пара, из позиции 3689 в первом файле с весом 0.99 в позицию 354156 во втором. Выдачу имеет смысл смотреть на предмет ошибок, например в файлах отропар и негативной выборки не должно быть пересечений, поэтому если в выдаче вставки отропар в негативные пары окажется, что не все значения вставляются, а какие-то перезаписываются новыми весами, значит где-то раньше произошла недопустимая ошибка. Однако при вставлении суммарной орто и негативной выборки в конечную медиану не должно быть именно вставок, а должна быть только перезапись, так как конечная медиана по условию генерации содержит все возможные комбинации генных пар. Поэтому если выдача покажет, что была вставка, значит где-то в процессе изнчальной генерации медианы произошла критическая ошибка. Начиная с генерации пар для предсказаний, далее во время генерации предсказаний, как мы это уже видели из-за ошибок на диске, заканчивая генерацией медианы.  

Кроме этого обе позиции, как и оба имени генов должны монотонно возрастать. Если этого не происходит, то означает, что на вход скрипта поданы неверно отсортированные данные. Сортировка и источника, и мишени должны быть остортированя по первому столбцу, и во вторую очередь - если значения по первому совпадают - по второму. В противном случае скрипт работать не будет! Таким образом достигается наискорейшая работа скрипта, при которой происходит по одному прочтению обоих файлов. 

В случае, если скрипт находит в исходном файле уже имеющуюся в файле-мишени генетическую пару, то он сообщит об этом в выдаче и перезапишет значение веса для этой пары в файле-мишени, а именно напишет какой старый вес он заменяет новым. Эта информация может быть полезна, чтобы видеть как отличаются предсказания по основной модели и предсказания по кросс-модели. 

В итоге получиться файл mediana2.txt, который надо переименовать в orthoAndNegMed.txt, чтобы снова запусить вставку уже в основную медиану.
./CopyPredToMedWithChange orthoAndNegMed.txt medians.txt

StrCol1[531307]=AT5G67640<----->StrCol2[531307]=Zm00001d002071<>Replace old wght=0.23<-> with new 0.75
StrCol1[531308]=AT5G67640<----->StrCol2[531308]=Zm00001d004862<>Replace old wght=0.03<-> with new 0.61
StrCol1[531309]=AT5G67640<----->StrCol2[531309]=Zm00001d005893<>Replace old wght=0.00<-> with new 0.21
StrCol1[531310]=AT5G67640<----->StrCol2[531310]=Zm00001d015086<>Replace old wght=0.30<-> with new 1.00
StrCol1[531311]=AT5G67640<----->StrCol2[531311]=Zm00001d016705<>Replace old wght=0.25<-> with new 0.94
StrCol1[531312]=AT5G67640<----->StrCol2[531312]=Zm00001d017205<>Replace old wght=0.02<-> with new 0.95
StrCol1[531313]=AT5G67640<----->StrCol2[531313]=Zm00001d021725<>Replace old wght=0.00<-> with new 0.03
StrCol1[531314]=AT5G67640<----->StrCol2[531314]=Zm00001d024874<>Replace old wght=0.00<-> with new 0.01
StrCol1[531315]=AT5G67640<----->StrCol2[531315]=Zm00001d025776<>Replace old wght=0.07<-> with new 0.90
StrCol1[531316]=AT5G67640<----->StrCol2[531316]=Zm00001d028197<>Replace old wght=0.25<-> with new 1.00
StrCol1[531317]=AT5G67640<----->StrCol2[531317]=Zm00001d037518<>Replace old wght=0.26<-> with new 0.99
StrCol1[531318]=AT5G67640<----->StrCol2[531318]=Zm00001d039338<>Replace old wght=0.16<-> with new 0.98
StrCol1[531319]=AT5G67640<----->StrCol2[531319]=Zm00001d041991<>Replace old wght=0.11<-> with new 0.99
StrCol1[531320]=AT5G67640<----->StrCol2[531320]=Zm00001d045237<>Replace old wght=0.01<-> with new 0.01
StrCol1[531321]=AT5G67640<----->StrCol2[531321]=Zm00001d047355<>Replace old wght=0.34<-> with new 0.98
StrCol1[531322]=AT5G67640<----->StrCol2[531322]=Zm00001d053018<>Replace old wght=0.35<-> with new 1.00

Можно запускать сразу две задачи последовательно одним заданием:
./CopyPredToMedWithChange orthoMediana.txt negMediana.txt
mv mediana2.txt orthoAndNegMed.txt
./CopyPredToMedWithChange orthoAndNegMed.txt medians.txt >CopNegAndPosToMediana.me

Далее можно возвращаться к пункту генерации из файла медианы матрицы весов с помощью mcl, после чего можно строить кластера. Данные по медиана отрицательной и положительной выборок можно удалить, но не раньше, чем они будут использованны для постройки ROC кривой. 

Теперь, после генерации медиан положительной и отрицательной выборок можно построить ROC кривую. Скрипт RocDataAuc можно запускать из любого места, где есть два необходимых для него два файла с данными, лучше всего из основной директории, где лежат все медианы со всеми вариантами сортировки. Эти два файла содержат генные пары из положительной и отрицательной выборок, отсортированные по третьему столбцу, по весам. 

ReadTwoKeyColAndFloat("negMediana.txt_sort3", &gNegPairsPred);
ReadTwoKeyColAndFloat("orthoMediana.txt_sort3", &gPosPairsPred);

В результате такая выдача
[hruk@node36 make_model]$ ./RocDataAuc - запуск программы
argv[ 0 ] = ./RocDataAuc 
Current working dir: /mnt/lustre/hruk/make_model
Reading two column and one float are finished... - удачно прочитан список отрицательной выборки
Reading two column and one float are finished... - удачно прочитан список положительной выборки
threshold=0.100 - построение ROC-кривой
threshold=0.200
threshold=0.300
threshold=0.400
threshold=0.500
threshold=0.600
threshold=0.700
threshold=0.800
threshold=0.900
Free memory start... - скрипт отработал и освобождает память.

В итоге получается файл с зависимостями TPR и FPR от threshold, которые встваляем в эксель файл и строим ROC-кривую и интегральную кривую для вычисления AUC. Среди файлов проэкта есть два вариана кривых для дву разных параметров тренировки. Для перовой ROC.txt ROC_np84_d10.txt и два экселевских файла с кривыми. Для первой тренировки AUC=0.92. Для второй AUC=0.86.

В эксель файле
TPR - столбец B
FPR - столбeц А
Threshold column C
AUC column D - интегральная кривая, результирующее значения AUC в 12 строке, соответсвующее TPR=1,FPR=1. 

Примерная скорость работы скриптов для полученя ROC кривой 6 часов для отропар и 12 часов для негативных пар.

Теперь о кластеризации подробней. Можно снова попытаться найти кластеры в данных, скоректированных предсказаниями для положительных и отрицательных выброк для второго набора параметров тренировки.

Наконец можно проводить кластеризацию с помощью https://micans.org/mcl/.
Сперва генерим матрицу весов пар геннов в формате mcl. 

mcxload -abc mediana2.txt --stream-mirror --write-binary -write-tab medNewPar3PN.tab -o medNewPar3PN.mci
mcl medNewPar3PN.mci -t 32 -I 8.0 -pi 9 -tf 'ceil(0.55)' -use-tab medNewPar3PN.tab -o med3PNI80cl55pi9.txt
mcl medNewPar3PN.mci -t 32 -I 8.0 -pi 9 -use-tab medNewPar3PN.tab -o med3PNI80pi9.txt

Далее можно варьировать параметры кластеризации. Для самой первой серии я перепробовал комбинации параметров в разных диапозонах и запусис около сотни задач кластеризации, прежде чем получил разумное количество малых калстеров - несколько тысяч. Для второй серии примерно столько же. В итоге я пришёл к выводу, что достаточно просканнировать только диапозон инфляций с 2 до 8 с шагом 1, и если при остальных стандартных параметрах не найдется кластеров, то следует запускать тренировку заново. Полезно варьировать такие параметры: gq,ceil,ceilnb,knn и инфляцию I [2,8], и прединфляцию pI [0.1,10].

Вот пример такого запуска, где предпринята попытка облегчить граф соответствий:
mcl mediansBn.mci -t 12 -I 6.0 -tf 'gq(0.5),#ceilnb(950),#knn(1000)' -use-tab mediansBn.tab -o medBnI60Knn1000clnb950.txt

Поэтому для оценки выбрал качества тренировки я выбрал такие параметры:
mcl medNewPar3PN.mci -t 32 -I 8.0 -pi 9 -use-tab medNewPar3PN.tab -o med3PNI80pi9.txt

Надо понимать, что это это всего лишь двумерное сечение многомерной зависимости клатеризации и вряд ли даст вразумительную зависимость количества кластеров, это делается для того, что бы подобрать такие параметры, чтобы было минимум синглетонов и минимум кластеров размером больше 10 000 геннов.

Таблица с результатами кластеризации при выбранных стандартных параметрах для пяти вариантов параметров тренировки прилагается в файле DpndceTrnPrmVsClsrts.xlsx.

Для изучения статистика по калстеризации написан скрипт CountClustersDistr.c 
Скрипт собирает все файлы из директории, в которой находится clusterXXX.txt, считает в них клсатера и по размерам с процентовкой распределения записывает результаты в файл clusterXXX.txt.stat. Чтобы скрипт коректно работал в директории не должно быть никаких других файлов, кроме самой программы и файлов с данными кластеризации типа clusterXXX.txt. Если после предыдущего подсчета статистики остались файлы ...txt.stat их надо удалить, иначе скрпт выдаст ошибку при попытке найти кластеры в файле статистики, где ксалтеров нет. Тип генов (альбидопсис и кукуруза) зашит в скрипт, по сути он различает две маски A... и Z... Если наборы генов будут другие, то маски надо менять и перекомпелировать скрипт.


(продолжение следует, аплоадить все три скрипта)





Иногда нужно перекопировать всю структуру данных за исключениеим файлов предсказаний, то есть модели, отрицательную выборку, модели кроссвалидации и так далее для всех, например, ста одной итерации. В этом случае полезна команда, которая скопирует все файлы размером меньше одного гигабайта.
rsync -rv --max-size=1000m ~/make_model/ ~/make_model_repair/
Так же в случае, если какая-либо из программ прервалась в середине, то некоторые из них надо запускать с начала, например, генерацию предсказаний для отрицательной выборки. Тогда может быть полезна команда, удаляющая все директории /neg с содержимым.
find . -name neg -type d -exec rm -rf {} +
или можно удалить искомые файлы
find -name "expression.other.predictions" -delete
Так же полезна комманда разбиения больших файлов на фрагменты
split -l 200000 pairs10_84.txt.

Кроме это было написано два скрипта, фактически база данных, для быстрого поиска всех пар генов, соответсвующих конкретному гену. Этот скрипт-сервер, будучи единожды загруженным в оперативную память и потребляя около двойного размера данных позволяет с помощью второго скрипта-клиента искать набор пар за несколько минут. Первичная загрузка данных занимает около получаса и остаётся в оперативной памяти сколь угодного долго. Скрипт-клиент посылает скрипту-серверу запрос с именем гена и через несколько мин записывает ответ в файл в месте запуска.






--------------------------далее черновик
1)
./predictAllByPortion ATH_MAP_INFECTION_LEAF_ROOT_STRESS_VITAMIN.txt ZEA_MAP_INFECTION_LEAF_ROOT_STRESS_VITAMIN.txt pairs.txt 0 100 20000000 75
где аргументы
1,2 - файлы с генами 
3 - файл с парами предсказаний - файл pairs.txt почти полный перебор генов 1.2 млрд, размер 28 гб, из чего следует, что для ста директорий iter_N таких файлов будет 100, то есть почти 3 тб
4 - начальная директория iter_
5 - конечная директория iter_
можно запускать не с начала от любого до любой
6 - количество пар которые будут обрабатываться за одно предсказание.
Опыт показал, что 30 млн пар занимают 1.5 Тб оперативки, теоретически в 2 Тб должно влезть 40 млн, но я перезакладываюсь на возможные утечки памяти из хгбуста и беру 20 млн пар для Node36 c 2 tb памяти.
7 - количество потоков, беру 75 в расчете на Node36, где восемдесят ядер.

Видеокарта в коде отключена, соответсвующие две строки с
XGBoost... gpu.... - можно раскоментить и перекомпелировать, однако для качественного запуска надо понять сколько потоков в видеокарте и есть ли у неё ограничения по памяти или все это происходит внутри драйверов и думать об этом не надо.

Опыт показал, что память не течет в XGBoost при переходе от одной директории iter к другой. Осталось убедиться, что память не течет при разрушении объектов хгбуста dmatrix и booster, которые я освобождаю и уничтожаю при каждой новой порции данных из файла пар для предсказания.

Если такая утечка будет, то сделать с ней ничего нельзя, не залезая внутрь самого хгбуста, чего бы я не делал. Плохое решение, это запускать этот скрипт перлом много раз, для этого надо только сгенерить не один файл пар, а например 100 и скриптом на перле запускать каждый по отдельности. Лучшее решение, это я планировал в таком случае перейти к С++ и засунуть С-код в объектно-ориентированную оболочку. И внутри программы пересоздавать объекты, при этом память будет автоматически вычищаться.

Можно вообще все автоматизировать, если подсчитывать память изначально, но с этим будет много возни и проще прикидывать порции на глазок.

Возможно при порции равной 0, будет читаться весь файл целиком, надо проверять. Точно то, что если задать больше пар, чем в файле, то прочтется весь файл за раз.

в новой версии хгбуста авторы поменяли интерфейс функции predict, добавили аргумент. В старой версии могут быть ошибки при компиляции. 
В связи с прекращением версии питона 2.7 новое сочетание версий: python36, scypi, cython - надо закачивать и ставить самую новую версию,
git clone https://github.com/cython/cython.git
2to3 XGBoostTree.saveModel.iter.py с небольшими недоделками переводит в третью версию питона. 

Программа генерит пары, алгоритм простейши 1 из файл 1 
/MakePairs ATH_MAP_INFECTION_LEAF_ROOT_STRESS_VITAMIN.txt ZEA_MAP_INFECTION_LEAF_ROOT_STRESS_VITAMIN.txt pairs0_03gp.txt 0 24 30000000

аргумент 3 - новый файл пар
аргумент 4 и 5 - начальная и конечная директории iter, в алгоритеме не используется, так как пока решили игнорировать то, что часть пар ортопары или из отрицательной выборки, и разбираться с ними уже при обработке результатов предсказаний
аргумент 6 - количество пар.

Первые две программы работаю,
последяя которая строит сами модели capidemoNew.c нуждается в отладке, а именно в случае если среди генов встречаются те, которых нет в основных файлах с генами и экспрессией
 ATH_MAP_INFECTION_LEAF_ROOT_STRESS_VITAMIN.txt ZEA_MAP_INFECTION_LEAF_ROOT_STRESS_VITAMIN.txt 
происходит сбой при заполнении массива, в котором готовиться данные для дматрицы. Это сложная ошибка и искать её надо с дебаггером, уровня эклипса, чтобы можно было все посмотреть. Можно и в емаксе попробовать.
Так же чтобы это работало, надо обязательна первый параметр, глубину максимальную дерева, задавать больше 0. В питоне и в си этот ноль толкуется по-разному. В си он приводит к тому, что модели не строятся. Поэтому надо задавать 3 в запуске на перле. Все остальные параметры как в аналогичном алгоритме на перле. 
./capidemoNew 3 0.3 1 1 1 1 0 4 1 50 auc 1 iter_$iter/results/training/expression $firstSpExpressionFile $secondSpExpressionFile iter_$iter/results/data_for_learning/folds iter_$iter/results/data_for_learning/negative_folds 0 0
Только последние два не используются. Я либо не понял, либо не сталкивался с фалами, где эти параметры есть. Судя по программе на питоне кажется это показывало наличие спец символов в концах строк. Эта проверка у меня проводится автоматически.
Программа сапидемоНью почти рабочая, однако в процессе попыток отыскать баг могли появиться новые баги. Поскольку алгоритм дублирует то, что и так пока можно делать алгоритмом на питоне, его отладка не стояла на первом месте. Однако это нужно доделать, чтобы уйти от версии питона 2.7, на которой написан питон алгоритм построения модели. Ну, и на макриче глюк, который не позволяет запустить питон алгоритм через очередь. Приходится запускать в лоб на какомн-нибудь узле.
