plantuml -o ../uml-state-diagrams plantuml/*-state-diagram.pu
cd uml-state-diagrams
echo '# Hunspell - UML State Diagrams' > README.md
for i in *png
do
    echo '![]('$i'?raw=true)' >> README.md
done
cd ..

exit

plantuml -o ../uml-object-diagrams plantuml/*-object-diagram.pu
cd uml-object-diagrams
echo '# Hunspell - UML Object Diagrams' > README.md
for i in *png
do
    echo '![]('$i'?raw=true)' >> README.md
done
cd ..

plantuml -o ../uml-class-diagrams plantuml/*-class-diagram.pu
cd uml-class-diagrams
echo '# Hunspell - UML Class Diagrams' > README.md
for i in *png
do
    echo '![]('$i'?raw=true)' >> README.md
done
cd ..

plantuml -o .. plantuml/color-legend.pu
