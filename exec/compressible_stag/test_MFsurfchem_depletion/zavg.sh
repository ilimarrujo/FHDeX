#!/bin/bash

exec=../../../../FBoxLib/Tools/Postprocessing/F_Src/faverage.Linux.gfortran.exe

if [ $# == 0 ]
then
    pltfiles=(`ls -d plt0*`)
    pltfile=${pltfiles[-1]}
else
    pltfile=$1
fi

outfile=res.zavg_$pltfile

$exec -p $pltfile -o $outfile -v 2 rhoYkInstant_0 rhoYkInstant_1 

gnuplot -persist <<-EOFMarker
    plot "$outfile" u 1:2 w l
EOFMarker
