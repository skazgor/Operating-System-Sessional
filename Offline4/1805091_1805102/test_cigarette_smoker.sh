echo "Testing Cigarette Smokers with Zemaphore"
g++    cs_zemaphore.c zemaphore.c -o cs_zemaphore -pthread
./cs_zemaphore 10  > cs_zemaphore.txt
awk -v MAX=10 -f test.awk cs_zemaphore.txt
echo "Testing Cigarette Smokers with Zemaphore Done"
echo "Testing Cigarette Smokers with CV"
g++    cs_cv.c -o cs_cv -pthread
./cs_cv 10  > cs_cv.txt
awk -v MAX=10 -f test.awk cs_cv.txt
echo "Testing Cigarette Smokers with CV Done"