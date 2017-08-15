#include "../include/simulator.h"
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <queue>
using namespace std;


int checksum_Compute(struct pkt p);
struct pkt compile_Packet(struct msg m);
int alternate_Seq_Num(int seqnum);
void transmit_All_Buffered_Packets();

/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional data transfer 
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

float timeout;
struct pkt mostRecentPacket;
struct msg mostRecentMessage;
bool retransmit;
struct pkt mostRecentAck;
queue<struct pkt> bufferedPackets;
struct pkt retransmitPacket;
bool msgInTransit;
bool alreadySent;

/* 
Variables declared to be used with the Sender A
*/

int nextSeqNum_A = 0;

/*
Variables declared to be used with Receiver B
*/

int nextSeqNum_B = 0;
struct pkt latestAck_B;

int checksum_Compute(struct pkt p) {
	int check_Sum = 0;
	check_Sum = (p.acknum) + (p.seqnum);
	for(int i = 0; i < sizeof(p.payload); i = i + 1) {
		check_Sum = check_Sum + p.payload[i];
	}
	
	return check_Sum;
}

struct pkt compile_Packet(struct msg m) {
	struct pkt p;
	nextSeqNum_A = alternate_Seq_Num(nextSeqNum_A);
	p.seqnum = nextSeqNum_A;
	p.acknum = nextSeqNum_A;
	strncpy(p.payload, m.data, sizeof(p.payload));
	p.checksum = checksum_Compute(p);
	return p;
}

int alternate_Seq_Num(int seqnum) {
//cout << " Alternate Sequence Number #############################################################################"<<endl;
	switch(seqnum){
		case 0:
			return 1;
			
		case 1:
			return 0;
	}		
}

void transmit_Packet(struct pkt p) {
	tolayer3(0, p);
	msgInTransit = true;
	starttimer(0, timeout);
	cout << " Packet Transmitted and Timer Started #############################################################################"<<endl;
}

void transmit_All_Buffered_Packets() {
	
	while(msgInTransit == false && bufferedPackets.size() != 0) {
		cout << " Transmitting Buffered Messages #############################################################################"<<endl;
		retransmitPacket = bufferedPackets.front();
		transmit_Packet(retransmitPacket);
		bufferedPackets.pop();
	}
}



/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{
		if(msgInTransit == true) {
			mostRecentPacket = compile_Packet(message);
			bufferedPackets.push(mostRecentPacket);
			cout << " Packet sent to A_output is buffered #####################################################"<<endl;
		}
		
		else {
			mostRecentPacket = compile_Packet(message);
			mostRecentMessage = message;
			bufferedPackets.push(mostRecentPacket);
			transmit_All_Buffered_Packets();
			cout << " Packet Added to buffer and transmitting started #####################################################"<<endl;
		}

}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
		
	if(retransmit == true && msgInTransit == false) {
		stoptimer(0);
		transmit_Packet(retransmitPacket);
		msgInTransit = true;
		cout << "Corrupt Packet is sent again and retransmitted #####################################################"<<endl;
	}
	
	else if(checksum_Compute(packet) == packet.checksum && packet.acknum == nextSeqNum_A) {
		stoptimer(0);
		alreadySent = false;
		msgInTransit = false;
		//nextSeqNum_A = alternate_Seq_Num(nextSeqNum_A);
		cout << " Packet RECEIVED BY A_input and is a valid ACK #####################################################"<<endl;
	}
	else {
		alreadySent = false;
		cout << " Corrupt Packet sent to A_input #####################################################"<<endl;
	}
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
	transmit_Packet(retransmitPacket);
	msgInTransit = true;
	cout << " A_timerinterrupt() called and packet retransmitted #####################################################"<<endl;
	
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
	timeout = 10.0;
	nextSeqNum_A = 1;
	msgInTransit = false;
	retransmit = false;
	alreadySent = false;

}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
	if(checksum_Compute(packet) == packet.checksum) {
		cout << "OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO" << endl;
		if (packet.seqnum == nextSeqNum_B){
			cout << "11111111111111111111111111111111111111111111111111111111111111111111111" << endl;
		
		struct pkt ackPacket;
		ackPacket.seqnum = nextSeqNum_B;
		ackPacket.acknum = nextSeqNum_B;
		strncpy(ackPacket.payload, packet.payload, sizeof(packet.payload));
		ackPacket.checksum = checksum_Compute(ackPacket);
		nextSeqNum_B = alternate_Seq_Num(nextSeqNum_B);
		if(alreadySent == false) {
			tolayer5(1, packet.payload);
			alreadySent = true;
		}
		retransmit = false;
		tolayer3(1, ackPacket);
		cout << " Packet SENT by B_input #####################################################"<<endl;
		}
	}
	
	else {
		alreadySent = false;
		retransmit = true;
		cout << " Packet is corrupt at B_input !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
	}
	
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
	nextSeqNum_B = 0;

}
