static class AFECSinkAgentClass : public TclClass {
 public:

AFECSinkAgentClass() : TclClass("Agent/AFECSink") {}

TclObject* create(int, const char*const*) {
return (new AFECSinkAgent());

}
} class_AFECSinkAgent;




classAFECSinkAgent : public Agent {
public:

AFECSinkAgent();
virtual intcommand(intargc, const char*const* argv);

virtual void recv(Packet* pkt, Handler*);

void report(intlostpkt, intblk);
 protected:

 void newblock();

intnlost_;
/*Total number of packets lost*/

intnpkts_;
/*Number of packets received*/

int expected_;
/*The expected sequence number of the next packet*/

int bytes_;
/*Number of bytes received*/
 
intseqno_;
/*Sequence number of the received packets*/
//14
double last_packet_time_; /*Temporal variable*/
//15
intk_blk_;
/* Total number of data packets per block */
//16
intn_blk_;
/*Total number of packets per block*/
//17
intr_blk_;
/* Redundancy per block */
//18
intblk_start_;
/* Sequence number where current block started */
//19
intlost_pkt_in_blk_;
/* Number of lost packets in current block */
//20
intaccumuled_losses;
/*If there are a burst of losses bigger than a block, the losses of the next
block should be taken into account for lost_b in next block*/
//21
intblk_received;
/*Variable that indicates when a block has been
received even though there are a burst of losses*/
//22
AFECSndAgent sender;
//23
};





void AFECSinkAgent::recv(Packet* pkt, Handler*) {
//Extract the sequence number value from the RTP header.&Access the RTP header.

hdr_rtp* rh = hdr_rtp::access(pkt);

seqno_ = rh->seqno();
//Store SEQ value in the seqno_ variable.

bytes_ += hdr_cmn::access(pkt)->size();

++npkts_;
//Increase the received packets counter.

int loss = seqno_ - expected_;
//Check for losses

if (loss > 0) {

nlost_ += loss;
/*Total number of lost packets*/

//Check if the losses comprise more than one n_blk_.

if(seqno_<=(blk_start_+n_blk_)) {

lost_pkt_in_blk_ += loss;

accumuled_losses=0;

else{

lost_pkt_in_blk_ += blk_start_+n_blk_-expected_;

//Store the number of losses for the next n_blk_.

accumuled_losses=seqno_-(blk_start_+n_blk_);

blk_received=1; //A n_blk_ has been received.

}

}

//Check whether an_blk_ has been totally received or not.

if ((seqno_==blk_start_+n_blk_)||(blk_received==1)) {

newblock();

blk_received=0;

}

expected_ = seqno_ + 1;
//Increase the expected SEQ whenever a new packet is received.

Packet::free(pkt);
 }



void AFECSinkAgent::report(int lost, intdatab) {
intr_blk_updated;

//Send the feedback information to the sender and update the header value.

r_blk_updated = sender.adapt(lost,datab);

//Update the number of packets that should be received per n_blk_.
n_blk_ = n_blk_ + r_blk_updated - r_blk_;

r_blk_=r_blk_updated;
 }






