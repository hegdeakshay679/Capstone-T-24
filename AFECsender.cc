class AFECSndAgent : public Agent {
 public:

AFECSndAgent();
AFECSndAgent(packet_t);
virtual void sendmsg(intnbytes, AppData* data, const char *flags= 0);
virtual void recv(Packet* pkt, Handler*);
virtual intcommand(intargc, const char*const* argv);

intn_over_m(intn,int m);
intadapt(int lost, intdatab);
protected:
intseqno_;

intk_blk_;
/* Packets per block */

intr_blk_;
/* Redundancy per block also called header value*/

intr_before_update_;
/*Header value before it is updated in adapt function*/

intn_blk_;
/*Total number of packets per block included the header*/

};


Void AFECSndAgent::AFECSndAgent() : Agent(PT_UDP), seqno_(-1) {
k_blk_ = 16;
r_blk_ = 0;
n_blk_ = 16;
r_initial_ = 0;
}

 Void AFECSndAgent::AFECSndAgent(packet_t type) : Agent(type) {

bind("packetSize_", &size_);
bind("k_blk_",&k_blk_);
bind("lost_pkt_in_blk_",&lost_pkt_in_blk_);

}



void AFECSndAgent::sendmsg(intnbytes, AppData* data, const char* flags){

Packet *p;
int n;//Estimate the number of packets needed to send the application data.

if (size_)

   n = nbytes / size_;
else

   printf("Error: UDP size = 0\n");

if (nbytes == -1){

   printf("Error”);

   return;

}

// If they are sending data, then it must fit within a single packet.

if (data &&nbytes> size_) {

printf(" data greater than maximum UDP packet size\n");

return;

}


while (n-- > 0) {

/*Introduce the sequence # in the RTP header.*/

p = allocpkt();
hdr_cmn::access(p)->size() = size_;

//Access the RTP header in a packet pointed by "p".

hdr_rtp* rh = hdr_rtp::access(p);

rh->seqno() = ++seqno_;
//Fill the RTP header with the sequ number.

if(seqno_ == blk_start_ + k_blk_){

n+=r_blk_;

blk_start_=seqno_+r_blk_;

}

hdr_cmn::access(p)>timestamp()=(u_int32_t)(SAMPLERATE*local_time);

p->setdata(data);

target_->recv(p);
//Send the packet.

}

//Send the last packet which size is less than the others.

n = nbytes % size_;

if (n > 0) {

p = allocpkt();

hdr_cmn::access(p)->size() = n;

hdr_rtp* rh = hdr_rtp::access(p);

rh->flags() = 0;

rh->seqno() = ++seqno_;

hdr_cmn::access(p)->timestamp()= (u_int32_t)(SAMPLERATE*local_time);

if (flags && (0 == strcmp(flags, "NEW_BURST")))

rh->flags() |= RTP_M;

if(seqno_== blk_start_+k_blk_){

n=r_blk_;

blk_start_=seqno_+r_blk_;

}

p->setdata(data);

target_->recv(p);
//Send the packet.

}
 idle();
 }




int AFECSndAgent::adapt(intlost,intdatab) {

printf("lost = %d\n",lost);

int r_delta_;
int i;
temp_=last_packet_time;
last_packet_time = ObjScheder::instance().clock();
temp_=last_packet_time - temp_;
// time between the reception of two blocks.

pkt_received_++;
// Variable where the total # of received blocks are store.

lost_pkt_in_blk_=lost;
//Number of lost packets in the just received n_blk.

n_blk_=datab;
//Total number of packets per n_blk included the header.

total_pkt_loss_=total_pkt_loss_+lost;
//Total number of losses in all the received blocks.
//12
total_=total_+ n_blk_;
//Total number of packets that should be received
//13
//14
//15
for(i=11;i>0;i--){
//16
memoryloss[i]=memoryloss[i-1];
//17
memoryblock[i]=memoryblock[i-1];
//18
}
//19
//The number of lost packets and the total number of sent packets in the
//20
//just received n_blk is stored in the first position of the memory vector.
//21
memoryloss[0]=lost;
//22
memoryblock[0]=datab;
//23
// The channel total_pkt_loss prob. is calculated based on the total_pkt_loss info. of the last 12 received blocks.
//24
channel_loss_prob_=0.0;
//25
for(i=0;i<=11;i++) {
//26
if(memoryblock[i]!=0){
//27
channel_loss_prob_+=(double)memoryloss[i]/(double)memoryblock[i];
//28
}
//29
}
//30
channel_loss_prob_=(double)channel_loss_prob_ /(double)(i);
//31
total_loss_prob_=(double)total_pkt_loss_ /(double)total_;
//Total total_pkt_loss probability
/*****************************On-off AFEC redundancy control system ****************/
//32
if(afec_control_==1)
//33
{
//34
r_before_update_=r_blk_;
//35
if (lost_pkt_in_blk_>r_before_update_){
//36
blk_lost_=1;
//37
total_blk_lost_++;
//8
}
//39
if (lost_pkt_in_blk_>r_before_update_){
//40
r_blk_=r_blk_+1;
//41
n_blk_=k_blk_+r_blk_;
//42
}
//43
if (lost_pkt_in_blk_<r_before_update_&&(r_before_update_>=1)){
//44
r_blk_=r_blk_-1;
//45
n_blk_=k_blk_+r_blk_;
//46
blk_lost_=0;
//47
}
//48
if (lost_pkt_in_blk_==r_before_update_){
//49
blk_lost_=0;
//50
n_blk_=k_blk_+r_blk_;
//51
}
//2
//In the variable relation_ is stored the excess of header afec_control,
//53
//it is used for compare the behavior of the three control systems.
//54
relation_+=(double)r_blk_-(double)lost_pkt_in_blk_;



percentage_=(double)r_blk_-(double)lost_pkt_in_blk_;
}//end of On-off afec_control
// Proportional and Proportional-integral control system have some common calculation.
if((afec_control_==2)||(afec_control_==3) ) {
//Estimation of the "n_blktotal_pkt_loss probability" based on the "channel total_pkt_loss probability".
inti=0;

est_blk_loss_prob_=0.0;
while (i<= k_blk_-1){
fact=n_over_m((k_blk_+r_blk_),i);
est_blk_loss_prob_+=fact*(pow((1-channel_loss_prob_),i))*(pow(channel_loss_prob_,(k_blk_+r_blk_-i)))*((double)(k_blk_+r_blk_-i)/(double)(k_blk_+r_blk_));
i++;
}
r_before_update_=r_blk_;
if (lost_pkt_in_blk_ >r_blk_ ){
blk_lost_=1;
total_blk_lost_++;
}
if ((lost_pkt_in_blk_<r_blk_&&(r_blk_>=1))||(lost_pkt_in_blk_==r_blk_))
blk_lost_=0;
/* Calculation of the error term e[k] as the difference between the estimated n_blktotal_pkt_loss probability calculated
before and the desired n_blktotal_pkt_loss probability set at the beginning of the */

error_=est_blk_loss_prob_ - des_blk_loss_prob_;
//**************Code for the ProportionalAFEC redundancy system************************/

if(afec_control_==2) {
//76
gamma_=170;
//Set the proportional constant.
//77
r_delta_=r_blk_-r_initial_;
//78
r_incr_=(int)rint(gamma_*error_); //Increment header afec_control calculation
//79
//Update the header afec_control.
//80
if(r_incr_<0){
//81
if(fabs(r_incr_)<=(r_delta_)){
//82
r_incr_=r_incr_+r_delta_;
//83
r_blk_=r_initial_+r_incr_;
//84
}else
//85
r_blk_=r_initial_;
//86
}
//87
else {
//88
r_incr_=r_incr_+r_delta_;
//89
r_blk_=r_initial_+r_incr_;
//90
}
//91
n_blk_=k_blk_+r_blk_;//Update the number of packets to be sent per n_blk.
//92
relation_+=(double)r_blk_-(double)lost_pkt_in_blk_;
//93
percentage_=(double)r_blk_-(double)lost_pkt_in_blk_;
//94
}//end of afec_control_ == 2
//****************Code for the Proportional-integral system*******************
//95
if (afec_control_==3) {
//96
gamma_ = 130;
//Set the proportional and the integral constants.
//97
betta_ = 115;
//98
r_delta_ = r_blk_- r_initial_;
//99
total_error_+=error_;
//Sum of all the past error values.
//100
totalsum_ = temp_*total_error_; //Multiply past error values by the sample time.
//101
//Estimates the header increment afec_control.
//102
r_incr_=(int)rint(gamma_*error_+ betta_*totalsum_);
//103
//Update the header afec_control.
//104
if(r_incr_<0){
//37105
if(fabs(r_incr_)<=(r_delta_)) {
//106
r_incr_ = r_incr_ + r_delta_;
//107
r_blk_ = r_initial_ + r_incr_;
//108
}else {
//109
r_blk_ = r_initial_;
//110
}
//111
}
//112
else {
//113
r_incr_= r_incr_+ r_delta_;
//114
r_blk_ = r_initial_+ r_incr_;
//115
}
//116
//Update the number of packets to be sent per n_blk.
//117
n_blk_ = k_blk_ + r_blk_;
//118
relation_+=(double)r_blk_-(double)lost_pkt_in_blk_;
//119
percentage_=(double)r_blk_-(double)lost_pkt_in_blk_;
//120
}//end of the afec_control_ == 3
//121
}// end of (afec_control_==2)||(afec_control_==3)
//122
return r_blk_;
//123 }









