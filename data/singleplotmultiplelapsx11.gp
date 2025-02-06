reset

set terminal x11 background rgb 'black' size 1200,600
set style line 101 lc rgb 'white' lt 1 lw 1
set style line 1 lc rgb 'red' lt 1 lw 1
set style line 2 lc rgb 'green' lt 1 lw 1
set style line 101 lc rgb 'white' lt 1 lw 1
set border 3 front ls 101
set nokey


set samples 10000
#set terminal dumb size 200, 50;
#set datafile separator ","

inputfile = '~/.cache/simmonitor/data.out'

stats inputfile using 1 name 'POINTS'
POINTS_range = POINTS_max - POINTS_min
POINTS_mid = POINTS_max / 2

stats inputfile using 7 name 'STEERINGLAP1'
STEERINGLAP1_range = STEERINGLAP1_max - STEERINGLAP1_min
stats inputfile using 13 name 'STEERINGLAP2'
STEERINGLAP2_range = STEERINGLAP2_max - STEERINGLAP1_min

stats inputfile using 6 name 'ACCELERATIONLAP1'
ACCELERATIONLAP1_range = ACCELERATIONLAP1_max - ACCELERATIONLAP1_min
stats inputfile using 12 name 'ACCELERATIONLAP2'
ACCELERATIONLAP2_range = ACCELERATIONLAP2_max - ACCELERATIONLAP2_min

stats inputfile using 5 name 'BRAKELAP1'
BRAKELAP1_range = BRAKELAP1_max - BRAKELAP1_min
stats inputfile using 11 name 'BRAKELAP2'
BRAKELAP2_range = BRAKELAP2_max - BRAKELAP2_min

stats inputfile using 4 name 'GEARLAP1'
GEARLAP1_range = GEARLAP1_max - GEARLAP1_min
stats inputfile using 10 name 'GEARLAP2'
GEARLAP2_range = GEARLAP2_max - GEARLAP2_min

stats inputfile using 3 name 'RPMLAP1'
RPMLAP1_range = RPMLAP1_max - RPMLAP1_min
stats inputfile using 9 name 'RPMLAP2'
RPMLAP2_range = RPMLAP2_max - RPMLAP2_min

stats inputfile using 2 name 'SPEEDLAP1'
SPEEDLAP1_range = SPEEDLAP1_max - SPEEDLAP1_min
stats inputfile using 8 name 'SPEEDLAP2'
SPEEDLAP2_range = SPEEDLAP2_max - SPEEDLAP2_min

# define the offset at which the fake y-axes start; decrease or increase offsetIncrease for spacing (effectively: blank labels) between 'graphs'
startYTicsOffset = 0
numberOfFakeYTicsPerData = 6
scalingFactor = 1.0/(numberOfFakeYTicsPerData - 1.0)
offsetIncrease = numberOfFakeYTicsPerData + 0.5

#to get rid of actual yrange numbering, set a dummy label that will be overwritten
set ytics ("dummy" 0)

#increase total actual yrange factor as needed for additional series
set yrange [0: 6 * offsetIncrease]

startYTicsOffset = startYTicsOffset
do for[i=0:numberOfFakeYTicsPerData-1]{
  set ytics add (sprintf("%i kph", SPEEDLAP1_min + i * scalingFactor * SPEEDLAP1_range) startYTicsOffset+i)
}
do for[i=0:numberOfFakeYTicsPerData-1]{
  set ytics add (sprintf("%i kph", SPEEDLAP2_min + i * scalingFactor * SPEEDLAP2_range) startYTicsOffset+i)
}

#add tics for fat - I couldn't figure out how to get gnuplot to print actual '%' character in sprintf directive (should be '%%' but doesn't appear to work)
startYTicsOffset = startYTicsOffset + offsetIncrease
do for[i=0:numberOfFakeYTicsPerData-1]{
  set ytics add (sprintf("%i rpm", RPMLAP1_min + i * scalingFactor * RPMLAP1_range) startYTicsOffset+i)
}
do for[i=0:numberOfFakeYTicsPerData-1]{
  set ytics add (sprintf("%i rpm", RPMLAP2_min + i * scalingFactor * RPMLAP2_range) startYTicsOffset+i)
}

#add tics for fat - I couldn't figure out how to get gnuplot to print actual '%' character in sprintf directive (should be '%%' but doesn't appear to work)
startYTicsOffset = startYTicsOffset + offsetIncrease
do for[i=0:numberOfFakeYTicsPerData-1]{
  set ytics add (sprintf("%i gear", GEARLAP1_min + i * scalingFactor * GEARLAP1_range) startYTicsOffset+i)
}
do for[i=0:numberOfFakeYTicsPerData-1]{
  set ytics add (sprintf("%i gear", GEARLAP2_min + i * scalingFactor * GEARLAP2_range) startYTicsOffset+i)
}



#add tics for fat - I couldn't figure out how to get gnuplot to print actual '%' character in sprintf directive (should be '%%' but doesn't appear to work)
startYTicsOffset = startYTicsOffset + offsetIncrease
do for[i=0:numberOfFakeYTicsPerData-1]{
  set ytics add (sprintf("%.1f percent", BRAKELAP1_min + i * scalingFactor * BRAKELAP1_range) startYTicsOffset+i)
}
do for[i=0:numberOfFakeYTicsPerData-1]{
  set ytics add (sprintf("%.1f percent", BRAKELAP2_min + i * scalingFactor * BRAKELAP2_range) startYTicsOffset+i)
}

#add tics for height
startYTicsOffset = startYTicsOffset + offsetIncrease
do for[i=0:numberOfFakeYTicsPerData-1]{
  set ytics add (sprintf("%.1f percent", ACCELERATIONLAP1_min + i * scalingFactor * ACCELERATIONLAP1_range) startYTicsOffset+i)
}
do for[i=0:numberOfFakeYTicsPerData-1]{
  set ytics add (sprintf("%.1f percent", ACCELERATIONLAP2_min + i * scalingFactor * ACCELERATIONLAP2_range) startYTicsOffset+i)
}

startYTicsOffset = startYTicsOffset + offsetIncrease
do for[i=0:numberOfFakeYTicsPerData-1]{
  set ytics add (sprintf("%.1f percent", STEERINGLAP1_min + i * scalingFactor * STEERINGLAP1_range) startYTicsOffset+i)
}
do for[i=0:numberOfFakeYTicsPerData-1]{
  set ytics add (sprintf("%.1f percent", STEERINGLAP2_min + i * scalingFactor * STEERINGLAP2_range) startYTicsOffset+i)
}



###### ... add further tics ...

set label "Speed" at POINTS_mid,5 tc rgb "white"
set label "Engine" at POINTS_mid,12 tc rgb "white"
set label "Gear" at POINTS_mid,19 tc rgb "white"
set label "Brake" at POINTS_mid,25 tc rgb "white"
set label "Accel" at POINTS_mid,32 tc rgb "white"
set label "Steer" at POINTS_mid,38 tc rgb "white"

GlobalTitle = sprintf("%s %s %s", ARG1, ARG2, ARG3)
Lap1Title = sprintf("%s%s", "Lap1: ", ARG4)
Lap2Title = sprintf("%s%s", "Lap2: ", ARG5)
set label GlobalTitle at POINTS_mid,-5 tc rgb "green"
set title GlobalTitle tc rgb 'white' noenhanced
set label Lap1Title at POINTS_max-120,39 tc rgb "red"
set label Lap2Title at POINTS_max-120,38 tc rgb "green"
set xlabel 'LapTime' tc rgb 'white'

plot inputfile using 1:( 0 * offsetIncrease + ($2 - SPEEDLAP1_min)   / (SPEEDLAP1_range * scalingFactor) )    w l title "speedlap1" ls 1,\
     inputfile using 1:( 0 * offsetIncrease + ($8 - SPEEDLAP2_min)   / (SPEEDLAP2_range * scalingFactor) )    w l title "speedlap2" ls 2,\
     inputfile using 1:( 1 * offsetIncrease + ($3 - RPMLAP1_min)   / (RPMLAP1_range * scalingFactor) )    w l title "rpmlap1" ls 1,\
     inputfile using 1:( 1 * offsetIncrease + ($9 - RPMLAP2_min)   / (RPMLAP2_range * scalingFactor) )    w l title "rpmlap2" ls 2,\
     inputfile using 1:( 2 * offsetIncrease + ($4 - GEARLAP1_min)   / (GEARLAP1_range * scalingFactor) )    w l title "gearlap1" ls 1,\
     inputfile using 1:( 2 * offsetIncrease + ($10 - GEARLAP2_min)   / (GEARLAP2_range * scalingFactor) )    w l title "gearlap2" ls 2,\
     inputfile using 1:( 3 * offsetIncrease + ($5 - BRAKELAP1_min)   / (BRAKELAP1_range * scalingFactor) )    w l title "brakelap1" ls 1,\
     inputfile using 1:( 3 * offsetIncrease + ($11 - BRAKELAP2_min)   / (BRAKELAP2_range * scalingFactor) )    w l title "brakelap2" ls 2,\
     inputfile using 1:( 4 * offsetIncrease + ($6 - ACCELERATIONLAP1_min)/ (ACCELERATIONLAP1_range * scalingFactor) ) w l title "accellap1" ls 1,\
     inputfile using 1:( 4 * offsetIncrease + ($12 - ACCELERATIONLAP2_min)/ (ACCELERATIONLAP2_range * scalingFactor) ) w l title "accellap2" ls 2,\
     inputfile using 1:( 5 * offsetIncrease + ($7 - STEERINGLAP1_min)/ (STEERINGLAP1_range * scalingFactor) ) w l title "steerlap1" ls 1,\
     inputfile using 1:( 5 * offsetIncrease + ($13 - STEERINGLAP2_min)/ (STEERINGLAP2_range * scalingFactor) ) w l title "steerlap2" ls 2,\
### ... add further data ...


set terminal postscript eps enhanced color background rgb 'black'
#set terminal postscript eps enhanced color solid colortext 9
set output '~/.cache/simmonitor/multiple_plots.eps'
replot
