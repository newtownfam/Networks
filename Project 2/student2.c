#include <stdio.h>
#include <stdlib.h>
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

/* Sender and Receiver structs */

enum State {
    WAIT_LAYER5,
    WAIT_ACK
};

 struct Sender {
    enum State state;
    int sequence;
    float rtt;
    struct pkt previous;
} A;

struct Receiver {
    int sequence;
} B;

/* Checksum function */

int calc_checksum(struct pkt *packet) {
  int checksum = 0;
  int i;
  checksum += packet->seqnum; // seqnum = index number of stream
  checksum += packet->acknum; // acknum = number of bytes received
  for (int i = 0; i < 20; i++) { checksum += packet->payload[i]; }
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
  struct pkt packet;

  /* make sure sender isn't still waiting for an acknowledgement */ 
  if (A.state != WAIT_LAYER5) {
    printf("A_output: not yet acked. Drop: %s\n", message.data);
    exit(1);
  }

  printf("A_output: Sending: %s\n", message.data);

  /* set packet data to sender struct */
  packet.seqnum = A.sequence; 
  memmove(packet.payload, mesage.data, 20);
  packet.checksum = calc_checksum(&packet);
  A.previous = packet;
  A.state = WAIT_ACK;
  
  /* send to layer 3 and start timer */
  tolayer3(0, packet);
  starttimer(0, A.rtt);
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
  
  if(A.state != WAIT_ACK) {
    printf("A_input: error, not A->B. Exiting...\n");
    exit(1);
  }
  
  if (packet.ackum != A.seq) {
    printf("A-input: incorrect ACK. Exiting...\n");
    exit(1);
  }

  printf("A_input: Successful ACK!\n");
  stoptimer(0);
  A.sequence = 1 - A.sequence;
  A.state = WAIT_LAYER5;

}

/*
 * A_timerinterrupt()  This routine will be called when A's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void A_timerinterrupt() {
  if (A.state != WAIT_ACK) {
    printf("A_timerinterrupt: not waiting for an ACK. Exiting...\n");
    exit(1);
  }
  printf("A_timerinterrupt: resending last packet: %s.\n", A.previous.payload);
  tolayer3(0, A.previous);
  starttimer(0, A.rtt);

}  

/* The following routine will be called once (only) before any other    */
/* entity A routines are called. You can use it to do any initialization */
void A_init() {
  A.state = WAIT_LAYER5;
  A.sequence = 0;
  A.rtt = 15;
}


/* 
 * Note that with simplex transfer from A-to-B, there is no routine  B_output() 
 */

void send_ack(int AorB, int ack) {
  struct pkt packet;
  packet.acknum = ack;
  packet.checksum = calc_checksum(&packet);
  tolayer3(AorB, packet);
}

/*
 * B_input(packet),where packet is a structure of type pkt. This routine 
 * will be called whenever a packet sent from the A-side (i.e., as a result 
 * of a tolayer3() being done by a A-side procedure) arrives at the B-side. 
 * packet is the (possibly corrupted) packet sent from the A-side.
 */
void B_input(struct pkt packet) {
  struct pkt sender_packet;
  if (packet.checksum != calc_checksum(&packet)) {
    printf("B_input: packet corrupted. Sending NAK.\n");
    send_ack(1, 1 - B.sequence);
    exit(1);
  }
  if (packet.seqnum != B.sequence) {
    printf("B_input: incorrect seq. Sending NAK...\n");
    send_ack(1, 1 - B.sequence);
    exit(1);
  }
    printf(" B_input: recv message: %s\n", packet.payload);
    printf(" B_input: send ACK.\n");
    send_ack(1, B.sequence);
    tolayer5(1, packet.payload);
    B.sequence = 1 - B.sequence;
}

/*
 * B_timerinterrupt()  This routine will be called when B's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void  B_timerinterrupt() {
  printf("Ignoring for now...\n", );
}

/* 
 * The following routine will be called once (only) before any other   
 * entity B routines are called. You can use it to do any initialization 
 */
void B_init() {
  B.seq = 0;
}

