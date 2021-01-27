import numpy as np
import xgboost as xgb
import sys
import datetime



max_depth = sys.argv[1]
eta = sys.argv[2]
subsample = sys.argv[3]
colsample_bytree = sys.argv[4]
colsample_bylevel = sys.argv[5]
min_child_weight = sys.argv[6]
gamma = sys.argv[7]
alpha = sys.argv[8]
lambdaParam = sys.argv[9]
numIterations = sys.argv[10]
evalMetric = sys.argv[11]
scale_pos_weight = sys.argv[12]
modelNam = sys.argv[13]
species1File = sys.argv[14]
species2File = sys.argv[15]
foldsDir = sys.argv[16]
negFoldsDir = sys.argv[17]
isHeader1 = sys.argv[18]
isHeader2 = sys.argv[19]

sp1Expression = dict()
sp2Expression = dict()

posTrainSetByFold = dict()
negTrainSetByFold = dict()
posTestSetByFold = dict()
negTestSetByFold = dict()

gNamsByFold = dict()
gNamsByNegativeFold = dict()

sp1File = open(species1File,"r")

for line in sp1File:
	if isHeader1 == 1:
		isHeader1 = 0
		continue
	arrLine = line.split('\t')
	gNam = arrLine[0]
	sp1Expression[gNam] = list()
	for i in range(1,len(arrLine)):
		expr_val = arrLine[i]
		sp1Expression[gNam].append(expr_val)
		

sp2File = open(species2File,"r")


for line in sp2File:
	if isHeader2 == 1:
		isHeader2 = 0
		continue
	arrLine = line.split('\t')
	gNam = arrLine[0]
	sp2Expression[gNam] = list()
	for i in range(1,len(arrLine)):
		expr_val = arrLine[i]
		sp2Expression[gNam].append(expr_val)
	
for curr_fold in range(0,10):
	gNamsByFold[curr_fold] = list()
	fileCurrFold = open(foldsDir+"/fold_"+str(curr_fold)+".orthopairs","r")
	for line in fileCurrFold:
		line = line[:-1]
		arrPairs = line.split('\t')
		gNamSp1 = arrPairs[0]
		gNamSp2 = arrPairs[1]
		if not gNamSp1 in sp1Expression:
			continue
		if not gNamSp2 in sp2Expression:
			continue
		gNamsByFold[curr_fold].append((gNamSp1,gNamSp2))
	gNamsByNegativeFold[curr_fold] = list()
	fileCurrNegFold = open(negFoldsDir+"/fold_"+str(curr_fold)+".orthopairs","r")
	for line in fileCurrNegFold:
		line = line[:-1]
		arrPairs = line.split('\t')
		gNamSp1 = arrPairs[0]
		gNamSp2 = arrPairs[1]
		if not gNamSp1 in sp1Expression:
			continue
		if not gNamSp2 in sp2Expression:
			continue
		gNamsByNegativeFold[curr_fold].append((gNamSp1,gNamSp2))	
	
for curr_fold_i in range(0,10):
	posTrainSetByFold[curr_fold_i] = list()
	negTrainSetByFold[curr_fold_i] = list()
	posTestSetByFold[curr_fold_i] = list()
	negTestSetByFold[curr_fold_i] = list()
	listPos = gNamsByFold[curr_fold_i]
	for gNam in listPos:
		posTestSetByFold[curr_fold_i].append(gNam)
	listNeg = gNamsByNegativeFold[curr_fold_i]
	for gNam in listNeg:
		negTestSetByFold[curr_fold_i].append(gNam)
	for curr_fold_j in range(0,10):
		if curr_fold_i == curr_fold_j:
			continue
		listPos = gNamsByFold[curr_fold_j]
		for gNam in listPos:
			posTrainSetByFold[curr_fold_i].append(gNam)
		listNeg = gNamsByNegativeFold[curr_fold_j]
		for gNam in listNeg:
			negTrainSetByFold[curr_fold_i].append(gNam)

posAllTrainList = list()
negAllTrainList = list()
posAllTestList = list()
negAllTestList = list()
for curr_fold in range(0,10):
	listPos = gNamsByFold[curr_fold]
	listNeg = gNamsByNegativeFold[curr_fold]
	posAllTrainList.extend(listPos)
	negAllTrainList.extend(listNeg)
	




for curr_fold in range(0,10):
	posTrainList = posTrainSetByFold[curr_fold]
	negTrainList = negTrainSetByFold[curr_fold]
	posTestList = posTestSetByFold[curr_fold]
	negTestList = negTestSetByFold[curr_fold]
	
	labelsTrain = list()
	trainSet = list()
	
	labelsTest = list()
	testSet = list()
	print(str(datetime.datetime.now()) + " posTrainList start loading ....")
	for gNamPairs in posTrainList:
		gNamSp1 = gNamPairs[0]
		gNamSp2 = gNamPairs[1]
		labelsTrain.append(1)
		exprListSp1 = sp1Expression[gNamSp1]
		exprListSp2 = sp2Expression[gNamSp2]
		listVals = list(exprListSp1)
		listVals.extend(exprListSp2)
		trainSet.append(listVals)
	
	print(str(datetime.datetime.now()) + " posTrainList loaded ....")
	
	
	print(str(datetime.datetime.now()) + " negTrainList start loading ....")
	for gNamPairs in negTrainList:
		gNamSp1 = gNamPairs[0]
		gNamSp2 = gNamPairs[1]
		labelsTrain.append(0)
		exprListSp1 = sp1Expression[gNamSp1]
		exprListSp2 = sp2Expression[gNamSp2]
		listVals = list(exprListSp1)
		listVals.extend(exprListSp2)
		trainSet.append(listVals)
		
	print(str(datetime.datetime.now()) + " negTrainList loaded ....")
	
	
	
	print(str(datetime.datetime.now()) + " posTestList start loading ....")
	for gNamPairs in posTestList:
		gNamSp1 = gNamPairs[0]
		gNamSp2 = gNamPairs[1]
		labelsTest.append(1)
		exprListSp1 = sp1Expression[gNamSp1]
		exprListSp2 = sp2Expression[gNamSp2]
		listVals = list(exprListSp1)
		listVals.extend(exprListSp2)
		testSet.append(listVals)
	print(str(datetime.datetime.now()) + " posTestList loaded ....")
	
	
	
	print(str(datetime.datetime.now()) + " negTestList start loading ....")
	for gNamPairs in negTestList:
		gNamSp1 = gNamPairs[0]
		gNamSp2 = gNamPairs[1]
		labelsTest.append(0)
		exprListSp1 = sp1Expression[gNamSp1]
		exprListSp2 = sp2Expression[gNamSp2]
		listVals = list(exprListSp1)
		listVals.extend(exprListSp2)
		testSet.append(listVals)
	print(str(datetime.datetime.now()) + " negTestList loaded ....")
	
	
	print(str(datetime.datetime.now()) + " npTrainSet start loading ....")
	npTrainSet = np.array(trainSet)
	print(str(datetime.datetime.now()) + " npTrainSet loaded ....")
	print(str(datetime.datetime.now()) + " npTestSet start loading ....")
	npTestSet = np.array(testSet)
	print(str(datetime.datetime.now()) + " npTestSet loaded ....")
	print(str(datetime.datetime.now()) + " npLabelsTrain start loading ....")
	npLabelsTrain = np.array(labelsTrain)
	print(str(datetime.datetime.now()) + " npLabelsTrain loaded ....")
	print(str(datetime.datetime.now()) + " npLabelsTest start loading ....")
	npLabelsTest = np.array(labelsTest)
	print(str(datetime.datetime.now()) + " npLabelsTest loaded ....")
	
	
	dtrain = xgb.DMatrix(npTrainSet,label=npLabelsTrain)
	dtest = xgb.DMatrix(npTestSet,label=npLabelsTest)
	print(curr_fold)
	print(max_depth)
	print(eta)
	print(subsample)
	print(colsample_bytree)
	print(colsample_bylevel)
	print(min_child_weight)
	print(gamma)
	print(alpha)
	print(lambdaParam)
	print(numIterations)
	print(evalMetric)
	print(scale_pos_weight)
	print(modelNam)
	
	print(dtrain)
	print(dtest)
	param = {'scale_pos_weight':scale_pos_weight,'alpha':alpha,'lambda':lambdaParam,'gamma':gamma,'min_child_weight':min_child_weight,'colsample_bylevel':colsample_bylevel,'colsample_bytree':colsample_bytree,'subsample':subsample,'bst:max_depth':max_depth, 'bst:eta':eta, 'silent':1, 'objective':'binary:logistic' }
	param['nthread'] = 4
	param['eval_metric'] = evalMetric
	evallist  = [(dtest,'eval'), (dtrain,'train')]

	plst = param.items()
	num_round = 30
	print("training");
	bst = xgb.train( plst, dtrain, 50, evallist,early_stopping_rounds=20 )
	bst.save_model(str(modelNam)+"/folds_"+str(curr_fold)+"/model/model_1_1_1_1000.test")
	print(bst)






posTrainList = posAllTrainList
negTrainList = negAllTrainList
posTestList = posTestSetByFold[0]
negTestList = negTestSetByFold[0]

labelsTrain = list()
trainSet = list()

labelsTest = list()
testSet = list()
print(str(datetime.datetime.now()) + " posTrainList start loading ....")
for gNamPairs in posTrainList:
	gNamSp1 = gNamPairs[0]
	gNamSp2 = gNamPairs[1]
	labelsTrain.append(1)
	exprListSp1 = sp1Expression[gNamSp1]
	exprListSp2 = sp2Expression[gNamSp2]
	listVals = list(exprListSp1)
	listVals.extend(exprListSp2)
	trainSet.append(listVals)

print(str(datetime.datetime.now()) + " posTrainList loaded ....")


print(str(datetime.datetime.now()) + " negTrainList start loading ....")
for gNamPairs in negTrainList:
	gNamSp1 = gNamPairs[0]
	gNamSp2 = gNamPairs[1]
	labelsTrain.append(0)
	exprListSp1 = sp1Expression[gNamSp1]
	exprListSp2 = sp2Expression[gNamSp2]
	listVals = list(exprListSp1)
	listVals.extend(exprListSp2)
	trainSet.append(listVals)
	
print(str(datetime.datetime.now()) + " negTrainList loaded ....")



print(str(datetime.datetime.now()) + " posTestList start loading ....")
for gNamPairs in posTestList:
	gNamSp1 = gNamPairs[0]
	gNamSp2 = gNamPairs[1]
	labelsTest.append(1)
	exprListSp1 = sp1Expression[gNamSp1]
	exprListSp2 = sp2Expression[gNamSp2]
	listVals = list(exprListSp1)
	listVals.extend(exprListSp2)
	testSet.append(listVals)
print(str(datetime.datetime.now()) + " posTestList loaded ....")



print(str(datetime.datetime.now()) + " negTestList start loading ....")
for gNamPairs in negTestList:
	gNamSp1 = gNamPairs[0]
	gNamSp2 = gNamPairs[1]
	labelsTest.append(0)
	exprListSp1 = sp1Expression[gNamSp1]
	exprListSp2 = sp2Expression[gNamSp2]
	listVals = list(exprListSp1)
	listVals.extend(exprListSp2)
	testSet.append(listVals)
print(str(datetime.datetime.now()) + " negTestList loaded ....")


print(str(datetime.datetime.now()) + " npTrainSet start loading ....")
npTrainSet = np.array(trainSet)
print(str(datetime.datetime.now()) + " npTrainSet loaded ....")
print(str(datetime.datetime.now()) + " npTestSet start loading ....")
npTestSet = np.array(testSet)
print(str(datetime.datetime.now()) + " npTestSet loaded ....")
print(str(datetime.datetime.now()) + " npLabelsTrain start loading ....")
npLabelsTrain = np.array(labelsTrain)
print(str(datetime.datetime.now()) + " npLabelsTrain loaded ....")
print(str(datetime.datetime.now()) + " npLabelsTest start loading ....")
npLabelsTest = np.array(labelsTest)
print(str(datetime.datetime.now()) + " npLabelsTest loaded ....")


dtrain = xgb.DMatrix(npTrainSet,label=npLabelsTrain)
dtest = xgb.DMatrix(npTestSet,label=npLabelsTest)
print("all")
print(max_depth)
print(eta)
print(subsample)
print(colsample_bytree)
print(colsample_bylevel)
print(min_child_weight)
print(gamma)
print(alpha)
print(lambdaParam)
print(numIterations)
print(evalMetric)
print(scale_pos_weight)
print(modelNam)

print(dtrain)
print(dtest)
param = {'scale_pos_weight':scale_pos_weight,'alpha':alpha,'lambda':lambdaParam,'gamma':gamma,'min_child_weight':min_child_weight,'colsample_bylevel':colsample_bylevel,'colsample_bytree':colsample_bytree,'subsample':subsample,'bst:max_depth':max_depth, 'bst:eta':eta, 'silent':1, 'objective':'binary:logistic' }
param['nthread'] = 4
param['eval_metric'] = evalMetric
evallist  = [(dtest,'eval'), (dtrain,'train')]

plst = param.items()
num_round = 30
print("training");
bst = xgb.train( plst, dtrain, 50, evallist,early_stopping_rounds=20 )
bst.save_model(str(modelNam)+"/folds_all/model/model_1_1_1_1000.test")
print(bst)






























#dtrain = xgb.DMatrix(trainFile)
#dtest = xgb.DMatrix(testFile)




#evalHistory = xgb.cv( plst, dtrain,int(numIterations),100,['auc','error'])
#print(evalHistory);
