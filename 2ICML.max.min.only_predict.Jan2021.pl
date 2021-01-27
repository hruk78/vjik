use strict;

my $firstSpExpressionFile = $ARGV[0];
my $secondSpExpressionFile = $ARGV[1];
my $similarityFile = $ARGV[2];

my $orthopairsFile = $ARGV[3];

my $minIterations = $ARGV[4];
my $maxIterations = $ARGV[5];
#my $output_prefix = $ARGV[5];

for(my $iter = $minIterations;$iter <=$maxIterations;$iter++ )
{
	mkdir("iter_$iter");
	mkdir("iter_$iter/results");
	mkdir("iter_$iter/results/data_for_learning");
	mkdir("iter_$iter/negative_sample");
	
	system("cp -r $similarityFile iter_$iter/results/data_for_learning/filtered_table.sim");
	system("perl GetNegativeRandomSet.pl $orthopairsFile 5202 $firstSpExpressionFile $secondSpExpressionFile iter_$iter/negative_sample/Negative_Sample.list");
	system("cp $firstSpExpressionFile iter_$iter/results/data_for_learning/expression.predict");
	system("cp $orthopairsFile iter_$iter/results/data_for_learning/orthopairs.list");
	mkdir("iter_$iter/results/data_for_learning/folds");
	mkdir("iter_$iter/results/data_for_learning/negative_folds");
	mkdir("iter_$iter/results/data_for_learning/svm");
	system("perl GetRandomFoldOfOrthopairs.pl iter_$iter/results/data_for_learning/orthopairs.list 10 iter_$iter/results/data_for_learning/folds");
	system("perl GetRandomFoldOfOrthopairs.pl iter_$iter/negative_sample/Negative_Sample.list 10 iter_$iter/results/data_for_learning/negative_folds");
	
	
	mkdir("iter_$iter/results/training");
	mkdir("iter_$iter/results/training/expression");
	for(my $i = 0;$i <= 9;$i++)
	{
		mkdir("iter_$iter/results/training/expression/folds_$i");
		mkdir("iter_$iter/results/training/expression/folds_$i/model");
	}
	mkdir("iter_$iter/results/training/expression/folds_all");
	mkdir("iter_$iter/results/training/expression/folds_all/model");
system("./makemodel.exe 10 0.3 1 1 1 1 0 4 1 50 auc 84 iter_$iter/results/training/expression $firstSpExpressionFile $secondSpExpressionFile iter_$iter/results/data_for_learning/folds iter_$iter/results/data_for_learning/negative_folds 0 0");
#system("gdb --args ./makemodel.exe 6 0.3 1 1 1 1 0 4 1 50 auc 1 iter_$iter/results/training/expression $firstSpExpressionFile $secondSpExpressionFile iter_$iter/results/data_for_learning/folds iter_$iter/results/data_for_learning/negative_folds 0 0");
#system("gdb --args ./capidemoNew.exe 0 0.3 1 1 1 1 0 4 1 50 auc 1 iter_$iter/results/training/expression $firstSpExpressionFile $secondSpExpressionFile iter_$iter/results/data_for_learning/folds iter_$iter/results/data_for_learning/negative_folds 0 0");
}