cd ../src/hunspell
for i in *.[ch]xx
do
    trueprint -2 $i > ../../checks/$i.ps
done
cd ../../checks
ls -ltrh *.ps
# man -t 5 hunspell |ps2pdf - hunspell.pdf
