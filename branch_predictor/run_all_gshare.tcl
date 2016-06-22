#!/usr/bin/env tclsh

set gMax 16 
set gMin 10 

set resLimit [expr pow(2,16)+256]

set avgInfo [open avg_result_gshare.txt w]
set allFiles [glob ../traces/*]

set minAvgRate 50
for { set gBits $gMin } { $gBits <= $gMax } {set gBits [expr $gBits + 1] } {
		
		set totResource [expr pow(2,$gBits+1) + $gBits ]

		if { $totResource > $resLimit } {
			continue
		} else {
			puts "$gBits"
			set rptFile [open ./rpt_gshare/result_gshare_${gBits}.txt w]
			puts $rptFile "$gBits"
			set totMisRate 0
			set numFiles 0
			foreach tmpFile $allFiles {
				puts $rptFile "======================="
				puts $rptFile $tmpFile
				catch {exec ./predictor --gshare:$gBits $tmpFile} result
				puts $rptFile $result
				set misRate [lindex $result 6]
				set totMisRate [expr $totMisRate + $misRate]
				set numFiles [expr $numFiles + 1]
			}
			set avgMisRate [expr $totMisRate / $numFiles]
			close $rptFile
			puts $avgInfo "======================="
			puts $avgInfo "$gBits"
			puts $avgInfo "avg miss Rate: $avgMisRate" 

			if { $minAvgRate > $avgMisRate } {
				set tgtGBits $gBits
				set minAvgRate $avgMisRate
			}
		}	
}

puts $avgInfo "==================="
puts $avgInfo "min avg miss rate: $minAvgRate"
puts $avgInfo "ghistoryBits: $tgtGBits"

close $avgInfo
