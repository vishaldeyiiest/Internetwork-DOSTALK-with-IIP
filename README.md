# INTERNETWORK DOSTALK with IIP(IIEST internet protocol)

## HOW IT WORKS

- The routing is done by implementing a customised protocol, say IIP protocol. Each IIP is 2 bytes.
 1st byte represents net ID and 2nd byte is host ID. IIP is abbreviation for IIEST Internet Protocol.

### Packet format is as follows:
1. 6 byte destination MAC
2. 6 byte source MAC
3. Ethertype or length - 2 bytes
4. 2 byte destination IIP
5. 2 byte source IIP
6. Data starts from 18th byte.

- This packet format must be followed by all clients for communication. 

## Running dtalk.exe in clients ova
 
- The clients ova contains dtalk.c which is the program to be executed for talking with intra-network 
 and inter-network DOS machines. For intranetwork communication, each client dtalk.c maintains 
 a table(say ARP table) for the mapping of IIPs and MAC IDs of the machines in the same network. So packets 
 sent to IIPs having net ID same as that of sender do not get to router, it is sent directly. 
 For internetwork, it gets sent to router if it finds different net ID.

1. For sending data, run dtalk.exe, then first type in the IIP of destination, and then enter data and press Enter.
- e.g. to send data to IIP "1.2", type as 12msg.
2. The client's MAC are hard-coded and now the codes are assigned as per the client ova's. The code can be changed as per
- network configuration.

## Running router.exe in router ova

- Run router.exe in the vishal_router.ova. This is the router program. It is tested using 
 2 clients on two different networks connected to a single router where router.exe runs. 
 router.c contains the routing table which stores the mapping of IIPs and MACs for both the networks. 
 It checks for any such IIPs (byte 14 and 15) in the routing table and then modifies the packet, 
 by changing the source and destination MACs. If it cannot find any such IIPs it drops the packet. 
 If any packet is sent to router itself, it does not forward.

#### Press Esc to exit the program.

