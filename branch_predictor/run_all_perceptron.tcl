#!/usr/bin/env tclsh

set gMax 32 
set gMin 25 
set pcMax 24
set pcMin 8

set resLimit [expr pow(2,16)+256]

set avgInfo [open avg_result_perceptron.txt w]
set allFiles [glob ../traces/*]

set minAvgRate 50
for { set gBits $gMin } { $gBits <= $gMax } {set gBits [expr $gBits + 1] } {
	for { set pcBits $pcMin } { $pcBits <= $pcMax } {set pcBits [expr $pcBits + 1] } {
		
			set wBits [expr floor(log10($gBits * 1.93 + 14) / log10(2)) ]
			set totResource [expr pow(2,$pcBits) * $wBits + $gBits + 1 ]

			if { $totResource > $resLimit } {
				continue
			} else {
				puts "$gBits\t$pcBits"
				set rptFile [open ./rpt_perceptron/result_perceptron_${gBits}_${pcBits}.txt w]
				puts $rptFile "$gBits\t$pcBits"
				set totMisRate 0
				set numFiles 0
				foreach tmpFile $allFiles {
					puts $rptFile "======================="
					puts $rptFile $tmpFile
					catch {exec ./predictor --custom:$gBits:$pcBits $tmpFile} result
					puts $rptFile $result
					set misRate [lindex $result 6]
					set totMisRate [expr $totMisRate + $misRate]
					set numFiles [expr $numFiles + 1]
				}
				set avgMisRate [expr $totMisRate / $numFiles]
				close $rptFile
				puts $avgInfo "======================="
				puts $avgInfo "$gBits\t$pcBits"
				puts $avgInfo "avg miss Rate: $avgMisRate" 

				if { $minAvgRate > $avgMisRate } {
					set tgtGBits $gBits
					set tgtPCBits $pcBits
					set minAvgRate $avgMisRate
				}
			}	
	}
}
puts $avgInfo "==================="
puts $avgInfo "min avg miss rate: $minAvgRate"
puts $avgInfo "ghistoryBits: $tgtGBits"
puts $avgInfo "pcIndexBits: $tgtPCBits"

close $avgInfo
