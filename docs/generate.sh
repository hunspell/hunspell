BASE='https://raw.githubusercontent.com/hunspell/hunspell/master/docs'

plantuml -o ../uml-state-diagrams plantuml/*-state-diagram.pu
cd uml-state-diagrams
echo '# Hunspell - UML State Diagrams' > README.md
for i in *png
do
    echo '[![]('$i'?raw=true)]('$BASE'/uml-state-diagrams/'$i')' >> README.md
done
cd ..

plantuml -o ../uml-usecase-diagrams plantuml/*-usecase-diagram.pu
cd uml-usecase-diagrams
echo '# Hunspell - UML Usecase Diagrams' > README.md
for i in *png
do
    echo '[![]('$i'?raw=true)]('$BASE'/uml-usecase-diagrams/'$i')' >> README.md
done
cd ..

exit
#TODO Add diagrams for rendering below

plantuml -o ../uml-object-diagrams plantuml/*-object-diagram.pu
cd uml-object-diagrams
echo '# Hunspell - UML Object Diagrams' > README.md
for i in *png
do
    echo '[![]('$i'?raw=true)]('$BASE'/uml-object-diagrams/'$i')' >> README.md
done
cd ..

plantuml -o ../uml-class-diagrams plantuml/*-class-diagram.pu
cd uml-class-diagrams
echo '# Hunspell - UML Class Diagrams' > README.md
for i in *png
do
    echo '[![]('$i'?raw=true)]('$BASE'/uml-class-diagrams/'$i')' >> README.md
done
cd ..
