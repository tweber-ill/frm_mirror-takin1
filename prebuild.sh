#!/bin/bash



echo -e "building uis..."

cd ui
for file in *.ui
do
        outfile=ui_${file%\.ui}.h

        echo -e "${file} -> ${outfile}"
        uic ${file} -o ${outfile}
done
cd ..

echo -e "\n"




echo -e "building mocs..."


moc tools/taz/taz.h -o tools/taz/taz.moc
moc tools/taz/scattering_triangle.h -o tools/taz/scattering_triangle.moc
moc tools/taz/tas_layout.h -o tools/taz/tas_layout.moc
moc tools/taz/recip3d.h -o tools/taz/recip3d.moc

echo -e "\n"
