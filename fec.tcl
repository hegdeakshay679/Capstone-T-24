#Set the constant values used in the program
set buffer 25
set BW 2Mb
set delay 25ms
set prob_ 0.01
set queuet RED
#Create a simulator object
set ns [new Simulator]
set namfile [open out.nam w]
$ns namtrace-all $namfile
set nstracefile [open out.tr w]
$ns trace-all $nstracefile
#Open the output file
set mytracefile [open out0.tr w]
#Choose the color for the different traffic sources in the nam viewer
$ns color 1 Blue
$ns color 2 Red
#Create 3 nodes
set n0 [$ns node]
set n1 [$ns node]
set n2 [$ns node]
set n3 [$ns node]
set n4 [$ns node]
#create the trace file for the dropped packets
set td [open drop.tr w]
set to1 [$ns create-trace Drop $td $n0 $n3]
#Connect the nodes
global $queuet
$ns duplex-link $n0 $n3 10Mb 2ms $queuet
#$ns queue-limit $n0 $n3 $buffer
$ns duplex-link $n1 $n3 10Mb 2ms $queuet
$ns duplex-link $n2 $n3 10Mb 2ms $queuet
$ns duplex-link $n3 $n4 $BW $delay $queuet
$ns queue-limit $n3 $n4 $buffer
#Orient the links.
$ns duplex-link-op $n0 $n3 orient right
$ns duplex-link-op $n1 $n3 orient right-up
$ns duplex-link-op $n2 $n3 orient right-down
$ns duplex-link-op $n3 $n4 orient right
#Define a ’finish’ procedure
proc finish {} {
global mytracefile td
global ns namfile
$ns flush-trace
close $namfile
#close $nstracefile
exec nam out.nam &
#Close the output files
close $mytracefile
close $td
exit 0
}
#Define a procedure that attaches a UDP agent to a previously
#created node ’node#’ and attaches a CBR traffic generator to the
#agent with the characteristic values ’size’ for packet size and
#’rate’ for burst peak rate. The procedure connects the source with
proc attach-CBR-traffic {source node sink size rate } {
#Get an instance of the simulator
set ns [Simulator instance]
$ns attach-agent $node $source
#Create a constan bit rate traffic
set traffic [new Application/Traffic/CBR]
$traffic set packet-size $size
$traffic set rate $rate
#Attach the traffic agent to the traffic source
$traffic attach-agent $source
#Connect the source and the sink
$ns connect $source $sink
$sink sender $source
return $traffic
}
#Define a procedure which periodically records the sender data
#received of the traffic source afec0 and write them to the output
#files mytracefile.
proc record {} {
global sink0 afec0 mytracefile ns
#Get an instance of the simulator
#set ns [Simulator instance]
#Set the time after which the procedure should be called again
set time 0.01
#Indicator of the lost of a block
set blk_lost [$afec0 set blk_lost_]
#Header value before being changed in adapt()function.
set r_before_update [$afec0 set r_before_update_]
#Lost packets in the actual block
set lost_pkt_in_blk [$afec0 set lost_pkt_in_blk_]
#Header value after be changed in adapt()function
set r_blk [$afec0 set r_blk_]
#Incremental header value used to update the final header value
set r_incr [$afec0 set r_incr_]
#Amount of received bytes used for bandwidth calculations
set bytes [$sink0 set bytes_]
####################################################################
set bandwidth [expr $bytes/$time*8/1000000]
#Experimental channel loss probability
set channel_loss_prob [$afec0 set channel_loss_prob_]
#Settting channel loss probability
set prob [$afec0 set prob_]
#Total number of dismissed blocks
set total_blk_lost [$afec0 set total_blk_lost_]
#Total number of received blocks
set block_received [$afec0 set pkt_received_]
#Total number of lost packets
set total_pkt_loss [$afec0 set total_pkt_loss_]
#Total number of sent packets
set total_sent_pkt [$afec0 set total_]
#Estimated block loss probability
set est_block_loss_prob [$afec0 set est_blk_loss_prob_]
#Desired block loss probability
set des_block_loss_prob [$afec0 set des_blk_loss_prob_]
#Error(difference between probblockest_ and probblockdes_)
set error_diff [$afec0 set error_]
#Total loss probability (total lost packets/total sent packets)
set total_loss_prob [$afec0 set total_loss_prob_]
#Header excess per block
85set r_excess_blk [$afec0 set r_excess_blk_]
#Total amount of header excess
set total_r_excess [$afec0 set total_r_excess_]
#Value of the total sum for the integral part of PI-control system
set tot2 [$afec0 set totalsum_]
#Get the current time
set now [$ns now]
#Write the data to the files(results). This files later will be
#the input files to "Matlab". Whit this program the result will be
#plot.
set BW 2
puts $mytracefile "[format "%.2f" $now] $blk_lost $r_before_update $lost_pkt_in_blk $r_blk
$r_incr [format "%.4f" $bandwidth] $BW \
[format "%.4f" $channel_loss_prob] [format "%.4f" $prob] $total_blk_lost
$block_received $total_pkt_loss $total_sent_pkt\
[format "%.4f" $est_block_loss_prob] [format "%.4f" $des_block_loss_prob] [format
"%.4f" $error_diff] \
[format "%.4f" $total_loss_prob] $r_excess_blk $total_r_excess"
#Reset the bytes_ values on the traffic sinks
$sink0 set bytes_ 0
#Re-schedule the procedure
$ns at [expr $now+$time] "record"
}
#Create three traffic sinks and attach them to the node n4
#set sink0 [new Agent/AFECSink]
#$ns attach-agent $n4 $sink0
#Create three traffic sources
#set afec0 [new Agent/AFEC]
#$afec0 setclass_ 1
#set source0 [attach-CBR-traffic $afec0 $n0 $sink0 1000 1Mb]
#set source0 [attach-CBR-traffic $n0 1000 1Mb]
#Setup a TCP connection
set tcp [new Agent/TCP]
$tcp set class_ 2
$ns attach-agent $n1 $tcp
set sink [new Agent/TCPSink]
$ns attach-agent $n4 $sink
$ns connect $tcp $sink
$tcp set fid_ 1
#Setup a FTP over TCP connection
set ftp [new Application/FTP]
$ftp attach-agent $tcp
$ftp set type_ FTP
#Setup a UDP connectionm
set udp [new Agent/UDP]
$ns attach-agent $n2 $udp
set null [new Agent/Null]
$ns attach-agent $n4 $null
$ns connect $udp $null
$udp set fid_ 2
#Setup a CBR over UDP connection
set cbr2 [new Application/Traffic/CBR]
$cbr2 attach-agent $udp
$cbr2 set type_ CBR
$cbr2 set packet_size_ 1000
$cbr2 set rate_ 1Mb
$cbr2 set random_ false
#Create the error model.The loss probability is introduced in the
#link between nodes n3 and n4. So, this link becomes the bottleneck link.
set em_ [new ErrorModel]
$em_ set markecn_ false
$em_ unit pkt
$em_ set rate_ $prob_
$em_ set ranvar [new RandomVariable/Uniform]
set lossylink_ [$ns link $n3 $n4]
$em_ drop-target $to1
$lossylink_ install-error $em_
#Start logging the obtained data.
#$ns at 0.0 "record"
#Start the traffic sources
#$ns at 1.0 "$source0 start"
#Start and Stop the 4 "competing" sources.
$ns at 10.0 "$cbr2 start"
$ns at 10.5 "$ftp start"
$ns at 40.0 "$cbr2 stop"
$ns at 40.0 "$ftp stop"
#Stop the traffic sources
#$ns at 50.0 "$source0 stop"
#Call the finish procedure after 51 seconds simulation time
$ns at 51.0 "finish"
#Run the simulation
$ns run
