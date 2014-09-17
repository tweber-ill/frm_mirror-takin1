#!/bin/bash

UIC=uic-qt4
MOC=moc-qt4


echo -e "--------------------------------------------------------------------------------"
echo -e "building uis..."

cd ui
for file in *.ui
do
        outfile=ui_${file%\.ui}.h

        echo -e "${file} -> ${outfile}"
        ${UIC} ${file} -o ${outfile}
done
cd ..


echo -e "--------------------------------------------------------------------------------"
echo -e "\n"




echo -e "--------------------------------------------------------------------------------"
echo -e "building mocs..."

declare -a hfiles=(
        "tools/taz/taz.h"
        "tools/taz/scattering_triangle.h"
        "tools/taz/tas_layout.h"
        "tools/taz/recip3d.h"
        "tools/taz/nicos.h"
        "tools/res/ResoDlg.h"
        "dialogs/EllipseDlg.h"
        "dialogs/EllipseDlg3D.h"
        "dialogs/RecipParamDlg.h"
        "dialogs/RealParamDlg.h"
        "dialogs/SpurionDlg.h"
        "dialogs/NeutronDlg.h"
        "dialogs/SrvDlg.h"
)

for hfile in ${hfiles[@]}
do
        mocfile=${hfile%\.h}.moc

        echo -e "${hfile} -> ${mocfile}"
        ${MOC} ${hfile} -o ${mocfile}
done

echo -e "--------------------------------------------------------------------------------"
