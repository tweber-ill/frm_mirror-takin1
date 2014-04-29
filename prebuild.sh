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

moc tools/res/ResoDlg.h -o tools/res/ResoDlg.moc
moc dialogs/EllipseDlg.h -o dialogs/EllipseDlg.moc
moc dialogs/EllipseDlg3D.h -o dialogs/EllipseDlg3D.moc

moc dialogs/RecipParamDlg.h -o dialogs/RecipParamDlg.moc
moc dialogs/RealParamDlg.h -o dialogs/RealParamDlg.moc

echo -e "\n"
