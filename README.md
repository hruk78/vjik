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
показывают, что модель перетренированна. При любых параметрах кластеризации находится только
два примерно равных кластера, один - кукурузы, другой - альбидопсиса и от 324 до 5 000 синглетонов. По-видимому scale_pos_weight=84 был ошибкой. Он вычислялся из соображений, что суммарная отрицательная выборка по всем 101 иттерациям в 84 раза больше количества отропар. Однако это, если оценивать это соотношение в пределах одной иттерации, для 101 она чуть меньше еденицы. Возможно, так же, что это из-за глубины дерева, котрая была 10. В результате следующую модель строим с другими параметрами, scale_pos_weight=2, max_depth=7, кол-во итераций пусть будет 75,
./makemodel 7 0.3 1 1 1 1 0 4 1 75 auc 2 20 

Кром этого сложности с кластеризацией могут быть из-за некоректных предскзаний для ортопар и для и для отрицаетльной выборки, которые надо строить инчаче, чем предсказания для остальных генных пар. Поэтому написаны два скрипта, которые строят предсказания отдельно для этих обеих групп. Так же эти данные можно использовать для постройки ROC-кривой. 
./PositivPairsMedianaForROC ATH_MAP_INFECTION_LEAF_ROOT_STRESS_VITAMIN.txt. ZEA_MAP_INFECTION_LEAF_ROOT_STRESS_VITAMIN.txt 0 100 0 4
./NegativPairsForRoc ATH_MAP_INFECTION_LEAF_ROOT_STRESS_VITAMIN.txt. ZEA_MAP_INFECTION_LEAF_ROOT_STRESS_VITAMIN.txt 0 100 0 4

Следующим шагом эти данные надо вставить с заменой в уже построенную медиану предсказаний по всем иттерациям. Это делается скриптом.


(продолжение следует, аплоадить все три скрипта)




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
