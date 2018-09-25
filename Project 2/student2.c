#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "project2.h"
 
/* ***************************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for unidirectional or bidirectional
   data transfer protocols from A to B and B to A.
   Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets may be delivered out of order.

   Compile as gcc -g project2.c student2.c -o p2
**********************************************************************/


/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
/* 
 * The routines you will write are detailed below. As noted above, 
 * such procedures in real-life would be part of the operating system, 
 * and would be called by other procedures in the operating system.  
 * All these routines are in layer 4.
 */

int sequence, counter = 0;
struct msg buffer;

/* Checksum */

int calcCheck (char data [20]) {
  int i;
  int checksum=0;
  for (i = 0; i < 20; i++) { checksum += data[i]; }
  return checksum;
}


/* 
 * A_output(message), where message is a structure of type msg, containing 
 * data to be sent to the B-side. This routine will be called whenever the 
 * upper layer at the sending side (A) has a message to send. It is the job 
 * of your protocol to insure that the data in such a message is delivered 
 * in-order, and correctly, to the receiving side upper layer.
 */
void A_output(struct msg message) {
  int i;
  struct pkt newPkt;
  
  newPkt.seqnum = sequence%2;

  /* saves coming message into new packets */
  for (i = 0; i < sizeof(message.data); i++) {
      newPkt.payload[i] = message.data[i];
      buffer.data[i] = message.data[i];
  }

  /* calculate checksum for all messages */
  /* sending messages to layer3, the network layer */
  newPkt.checksum = calcCheck(newPkt.payload);

  printf("A_output: sending message: %s\n", newPkt.payload);

  startTimer (0, 20.0);
  tolayer3 (0, newPkt);
}


/*
 * Just like A_output, but residing on the B side.  USED only when the 
 * implementation is bi-directional.
 */
void B_output(struct msg message)  { 
  printf("B_output: Used only when implementation is bi-directional...\n");
}

/* 
 * A_input(packet), where packet is a structure of type pkt. This routine 
 * will be called whenever a packet sent from the B-side (i.e., as a result 
 * of a tolayer3() being done by a B-side procedure) arrives at the A-side. 
 * packet is the (possibly corrupted) packet sent from the B-side.
 */
void A_input(struct pkt packet) {
  int i;
  struct msg newMsg;
  printf("A_input: reading ack from B...\n");
  stopTimer (0);
  
  /* 
   * If ack > 0, send msg from buffer
   * If ack < 0, send previous msg
  /* 

  /* negative */
  if (packet.acknum == 0 && sequence%2 == 0) { sequence++;}
  else if (packet.acknum =1 && sequence%2 == 1) { sequence++; }
  
  /* positive */
  else { A_output (buffer); }
} 

/*
 * A_timerinterrupt()  This routine will be called when A's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void A_timerinterrupt() {
   A_output (buffer);
}  

/* The following routine will be called once (only) before any other    */
/* entity A routines are called. You can use it to do any initialization */
void A_init() {
  /* N/A */
}



/*
 * B_input(packet),where packet is a structure of type pkt. This routine 
 * will be called whenever a packet sent from the A-side (i.e., as a result 
 * of a tolayer3() being done by a A-side procedure) arrives at the B-side. 
 * packet is the (possibly corrupted) packet sent from the A-side.
 */
void B_input(struct pkt packet) {
  int i;
  int localchecksum;
  struct msg newMsg;
  struct pkt ackPkt;
  
  /*calculate checksum and send it to layer 3 */
  localchecksum = calcCheck (packet.payload);
  if (localchecksum != packet.checksum) {
      ackPkt.acknum =- 1;
      tolayer3(1, ackPkt);
  } 

  else {
    
    //copies payload into message and passes it onto layer 5
    for (i = 0; i < 20; i++) {
      newMsg.data[i] = packet.payload[i];
    }

    printf("B_input: Message Received: %s\n", newMsg.data);
    tolayer5 (1, newMsg);
    printf("B_input: Sending to Layer 3...\n");
    ackPkt.acknum = packet.seqnum;
    tolayer3 (1, ackPkt);
  }
}

/*
 * B_timerinterrupt()  This routine will be called when B's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void  B_timerinterrupt() {
  /* N/A */
}

/* 
 * The following routine will be called once (only) before any other   
 * entity B routines are called. You can use it to do any initialization 
 */
void B_init() {
   /* N/A */
}

