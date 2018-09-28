/*
 * Peter Christakos
 * pechristakos@wpi.edu
 * CS Networks Project 2 - ABP
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "project2.h"
/* ***************************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for Project 2, unidirectional or bidirectional
   data transfer protocols from A to B and B to A.
   Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets may be delivered out of order.

   Compile as gcc -g project2.c student2.c -o p2
**********************************************************************/

/***************** Helper Functions and Variables *********************/

// project2.c variable
extern int TraceLevel; 

// Defined Variables
#define WAITING (1)
#define READY (0)

// Created Sender Variables
int senderState;
struct pkt senderPkt;
int senderLastSeqNum;

// Reciever
int recvState;
struct pkt recvPkt;
int recvLastSeqNum;
struct pkt recvACK; 

// LinkedList Queue
struct Node {
  char data[20];
  struct Node* next;
};

// Hold head and tail nodes 
struct Node* head = NULL;
struct Node* tail = NULL;


unsigned short CalcChecksum(struct pkt Packet)
{
    Packet.checksum = 0; 
    char arr[sizeof(Packet)]; 
    memcpy(arr, &Packet, sizeof(Packet)); 

    char* buffer = arr;
    int size = sizeof(arr);
    unsigned short a = 0;
    unsigned short b;
    int i,j;

    for (i = 0; i < size; i++) {
        b = buffer[i] << 8;
        for (j = 0; j < 8; j++) {
            if ((b & 0x8000) ^ (a & 0x8000)) {
                a = (a <<= 1) ^ 4129;
            }
            else {
                a <<= 1;
            }
            b <<= 1;
        }
    }
    return (int)a;
}


// function to check if received packet was valid
int CheckCorrupt(struct pkt packet) {
  if(packet.acknum != 0 && packet.acknum != 1) {
    if(TraceLevel > 1) {
      printf("Corrupted Packet ACK Number foun.\n Packet Acknum: %d;\n\n", senderPkt.acknum);
    }
    return 0; // corrupt
  }
  if(packet.seqnum != 0 && packet.seqnum != 1) {
    if(TraceLevel > 1) {
      printf("Corrupted Packet Sequence Number found.\n Packet Seqnum: %d;\n\n", senderPkt.seqnum);
    }
    return 0; // corrupt
  }
   if(CalcChecksum(packet) != packet.checksum) {
     if(TraceLevel > 1) {
      printf("Corrupted Packet Checksum found.. \n Calculated:%d\n Actual: %d;\n\n", CalcChecksum(packet), senderPkt.checksum);
    }
    return 0; // corrupt
  }
  return 1;
}

//Enqueue
void Enqueue(char data[20]) {
  struct Node* temp = (struct Node*)malloc(sizeof(struct Node));
  
  strncpy(temp->data, data, 20); 
  temp->next = NULL;

  if(head == NULL && tail == NULL){
    head = tail = temp;
    return;
  }
  tail->next = temp;
  tail = temp;
  return;
}

// Dequeue
char *Dequeue() {
  struct Node* temp = head;
  if(head == NULL) {
    return NULL;
  }
  if(head == tail) {
    head = tail = NULL;
  } else {
    head = head->next;
  }
  return temp->data;
}


/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
/* 
 * The routines you will write are detailed below. As noted above, 
 * such procedures in real-life would be part of the operating system, 
 * and would be called by other procedures in the operating system.  
 * All these routines are in layer 4.
 */


/* 
 * A_output(message), where message is a structure of type msg, containing 
 * data to be sent to the B-side. This routine will be called whenever the 
 * upper layer at the sending side (A) has a message to send. It is the job 
 * of your protocol to insure that the data in such a message is delivered 
 * in-order, and correctly, to the receiving side upper layer.
 */
void A_output(struct msg message) {

  /* Check if packet is ready to be sent */
  if(senderState == WAITING) {    // if ACK not yet received, enqueue      
    Enqueue(message.data);    
    return;              
  } else if(senderState == READY) {  // Ready to be sent
    
    /* msg -> pkt */
    senderPkt.seqnum = (senderPkt.seqnum == 0) ? 1 : 0; // returns 0 or 1
    senderPkt.acknum = 0; 
    strncpy(senderPkt.payload, message.data, MESSAGE_LENGTH); //Copy the message data into the payload
    senderPkt.checksum = CalcChecksum(senderPkt);
    if(TraceLevel > 1) { printf("A_output: Packet Created.\n Seqnum: %d; \n Acknum: %d;\n Checksum: %d;\n\n", senderPkt.seqnum, senderPkt.acknum, senderPkt.checksum);
    }
    /* Send packet to layer 3 and start timer */
    stopTimer(0);
    startTimer(0, 5000);
    tolayer3(0, senderPkt);
    senderState = WAITING;
  }
}

/*
 * Just like A_output, but residing on the B side.  USED only when the 
 * implementation is bi-directional.
 */
void B_output(struct msg message) {
  /* N/A for this project */
}

/* 
 * A_input(packet), where packet is a structure of type pkt. This routine 
 * will be called whenever a packet sent from the B-side (i.e., as a result 
 * of a tolayer3() being done by a B-side procedure) arrives at the A-side. 
 * packet is the (possibly corrupted) packet sent from the B-side.
 */
void A_input(struct pkt packet) {
  char emptystr[MESSAGE_LENGTH];
  memset(emptystr, '\0', sizeof(emptystr));

  if((!strcmp(packet.payload, emptystr)) && (packet.checksum == CalcChecksum(packet)) && (packet.acknum == 1 && packet.seqnum == senderPkt.seqnum)){ 
    senderState = READY;
    char *bufferText = Dequeue();
    if( bufferText != NULL ) {
      struct msg newMessage;
      strncpy(newMessage.data, bufferText, 20);
      A_output(newMessage);
    } else {
      if(TraceLevel > 1) {
      printf("A_input: Packet wity Seqnum %d Failed.\n Packet Structure:\n Seqnum: %d;\n Acknum: %d;\n Checksum: %d;\n\n", senderPkt.seqnum, senderPkt.seqnum, senderPkt.acknum, senderPkt.checksum);
      }
    }
  }
}

/*
 * B_input(packet),where packet is a structure of type pkt. This routine 
 * will be called whenever a packet sent from the A-side (i.e., as a result 
 * of a tolayer3() being done by a A-side procedure) arrives at the B-side. 
 * packet is the (possibly corrupted) packet sent from the A-side.
 */
void B_input(struct pkt packet) {
  if(CheckCorrupt(packet) == 0) { 
    recvACK.seqnum = recvLastSeqNum; 
    recvACK.checksum = CalcChecksum(recvACK);
    tolayer3(1, recvACK);
    return;
  } else if(packet.seqnum == recvLastSeqNum) { 
      recvACK.seqnum = recvLastSeqNum; 
      recvACK.checksum = CalcChecksum(recvACK);
      tolayer3(1, recvACK);
    } else {
      recvLastSeqNum = packet.seqnum;
      recvACK.seqnum = packet.seqnum; 
      recvACK.checksum = CalcChecksum(recvACK);
      tolayer3(1, recvACK); 
      
      struct msg message;
      strncpy(message.data, packet.payload, 20);
      tolayer5(1, message); 
    }
  }

/*
 * A_timerinterrupt()  This routine will be called when A's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void A_timerinterrupt() {
  stopTimer(0);
  senderState = READY;

  struct msg message;
  strncpy(message.data, senderPkt.payload, 20);
  senderPkt.seqnum = (senderPkt.seqnum == 0) ? 1 : 0; // returns 0 or 1
  A_output(message);
}  

/*
 * B_timerinterrupt()  This routine will be called when B's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void  B_timerinterrupt() {
  /* N/A for this project */
}


/* The following routine will be called once (only) before any other    */
/* entity A routines are called. You can use it to do any initialization */
void A_init() {
  senderPkt.seqnum = 0;
  senderPkt.acknum = 1;
  senderState = READY;
}

/* 
 * The following routine will be called once (only) before any other   
 * entity B routines are called. You can use it to do any initialization 
 */
void B_init() {
  recvLastSeqNum = 0; 
  recvACK.acknum = 1; 
  memset(recvACK.payload, '\0', sizeof(recvACK.payload));
}

