#!/usr/bin/env tclsh

set gMax 16 
set gMin 8
set lMax 16
set lMin 8
set pcMax 16
set pcMin 8

set resLimit [expr pow(2,16)+256]


set avgInfo [open avg_result_tournament.txt w]
set allFiles [glob ../traces/*]

set minAvgRate 50
for { set gBits $gMin } { $gBits <= $gMax } {set gBits [expr $gBits + 1] } {
	for { set lBits $lMin } { $lBits <= $lMax } {set lBits [expr $lBits + 1] } {
		for { set pcBits $pcMin } { $pcBits <= $pcMax } {set pcBits [expr $pcBits + 1] } {
		
			set gResource [expr pow(2,($gBits +1)) + $gBits ]
			set lResource [expr pow(2,($lBits +2)) + pow(2,$pcBits) * $lBits + $gBits ]
			set totResource [expr $gResource + $lResource]

			if { $totResource > $resLimit } {
				continue
			} else {
				puts "$gBits\t$lBits\t$pcBits"
				set rptFile [open ./rpt_tournament/result_tournament_${gBits}_${lBits}_${pcBits}.txt w]
				puts $rptFile "$gBits\t$lBits\t$pcBits"
				set totMisRate 0
				set numFiles 0
				foreach tmpFile $allFiles {
					puts $rptFile "======================="
					puts $rptFile $tmpFile
					catch {exec ./predictor --tournament:$gBits:$lBits:$pcBits $tmpFile} result
					puts $rptFile $result
					set misRate [lindex $result 6]
					set totMisRate [expr $totMisRate + $misRate]
					set numFiles [expr $numFiles + 1]
				}
				set avgMisRate [expr $totMisRate / $numFiles]
				close $rptFile
				puts $avgInfo "======================="
				puts $avgInfo "$gBits\t$lBits\t$pcBits"
				puts $avgInfo "avg miss Rate: $avgMisRate" 
				puts "avg miss Rate: $avgMisRate" 

				if { $minAvgRate > $avgMisRate } {
					set tgtGBits $gBits
					set tgtLBits $lBits
					set tgtPCBits $pcBits
					set minAvgRate $avgMisRate
				}
			}
		}
	}
}

puts $avgInfo "==================="
puts $avgInfo "min avg miss rate: $minAvgRate"
puts $avgInfo "ghistoryBits: $tgtGBits"
puts $avgInfo "lhistoryBits: $tgtLBits"
puts $avgInfo "pcIndexBits: $tgtPCBits"
close $avgInfo
