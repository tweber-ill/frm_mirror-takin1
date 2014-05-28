#!/bin/bash

UIC=uic-qt4
MOC=moc-qt4


echo -e "building uis..."

cd ui
for file in *.ui
do
        outfile=ui_${file%\.ui}.h

        echo -e "${file} -> ${outfile}"
        ${UIC} ${file} -o ${outfile}
done
cd ..

echo -e "\n"




echo -e "building mocs..."


${MOC} tools/taz/taz.h -o tools/taz/taz.moc
${MOC} tools/taz/scattering_triangle.h -o tools/taz/scattering_triangle.moc
${MOC} tools/taz/tas_layout.h -o tools/taz/tas_layout.moc
${MOC} tools/taz/recip3d.h -o tools/taz/recip3d.moc

${MOC} tools/res/ResoDlg.h -o tools/res/ResoDlg.moc
${MOC} dialogs/EllipseDlg.h -o dialogs/EllipseDlg.moc
${MOC} dialogs/EllipseDlg3D.h -o dialogs/EllipseDlg3D.moc

${MOC} dialogs/RecipParamDlg.h -o dialogs/RecipParamDlg.moc
${MOC} dialogs/RealParamDlg.h -o dialogs/RealParamDlg.moc
${MOC} dialogs/SpurionDlg.h -o dialogs/SpurionDlg.moc
${MOC} dialogs/NeutronDlg.h -o dialogs/NeutronDlg.moc

echo -e "\n"
