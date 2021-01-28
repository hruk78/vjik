# vjik
Построение Модели

a) XGBoostTree.saveModel.iterPy3.py переписана на третьем Python.
Запуск скриптом oldICML.max.min.only_predict.fast.5K.pl
1 - max_depth;
2 - eta;
3 - subsample;
4 - colsample_bytree;
5 - colsample_bylevel;
6 - min_child_weight;
7 -gamma;
8 -alpha;
9 - reg_alpha;
10 - lambdaParam;
11 - eval_metric;
12 - scale_pos_weight;
далее разные директории с данными

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
9 - reg_alpha;
10 - lambdaParam;
11 - eval_metric;
12 - scale_pos_weight;
13 - nthread;
далее разные директории с данными

Генерация геномных пар для предсказаний

Для этого скрипт
MakeAllPairs.с
Параметры запуска, последние два параметра - номер начальной и конечной директорий с построенными модялями, в данном случае от iter_0 до iter_100. Они не обязательны в этом скрипте, но я оставляю их, чтобы помнить какая серия запусков происходит:
./MakeAllPairs ATH_MAP_INFECTION_LEAF_ROOT_STRESS_VITAMIN.txt ZEA_MAP_INFECTION_LEAF_ROOT_STRESS_VITAMIN.txt pairs10_84.txt 0 100

На случай нехватки памяти или диска - для данных геномных карт получившийся файл будет около 40Gb - были написаны два похожих скрипта, для генерации первой порции MakePairs.c и порции оставшихся MakePairsLastOnes.c, где последним параметром добавляется количество пар. Для первого скрипта это количество, которе будет сгенерированно, а для второго это количество с которого начнется генерация, чтобы закончиться до конца. В примере ниже генерировалось 1.2 гигопар.
./MakePairs ATH_MAP_INFECTION_LEAF_ROOT_STRESS_VITAMIN.txt ZEA_MAP_INFECTION_LEAF_ROOT_STRESS_VITAMIN.txt pairs.txt 0 24 1200000000
./MakePairsLastOnes ATH_MAP_INFECTION_LEAF_ROOT_STRESS_VITAMIN.txt ZEA_MAP_INFECTION_LEAF_ROOT_STRESS_VITAMIN.txt pairs.txt 0 24 1200000000
Подход генерить пары порциями порождает необходимость строить предсказния порциями, а потом суммировать эти предсказания в общем файле. Скрипты для этого были написаны, но понадобяться они в случае увеличения объёма данных. В данном случае можно все делать одной порцией. 

(продолжение следует)







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
