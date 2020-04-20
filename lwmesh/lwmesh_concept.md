lwmesh (Lightweight mesh)
---

It is implemented with the state machine algorithm on `bare machine`

# Architecture

```
    Application layer
     ^    ^      ^
     |    |      |
     |    |      v
     |    |   Network layer (network header)
     |    |      ^
     |    |      |
     |    v      v
     |  Physical layer (MAC header)
     |           ^
     |           |
     v           v
    Hardware Abstraction Layer (HAL)

```

+ MAC header

```c
typedef struct __attribute__ ((packed)) mac_header
{
    uint16_t    macFcf;
    uint8_t     macSeq;
    uint16_t    macDstPanId;
    uint16_t    macDstAddr;  // the destination address of the neighbor hop
    uint16_t    macSrcAddr;  // the source address of the neighbor hop
} mac_header_t;
```

+ Network header

```c
typedef struct __attribute__ ((packed)) nwk_header
{
    struct __attribute__ ((packed)) {
        uint8_t   ackRequest : 1;
        uint8_t   security   : 1;
        uint8_t   linkLocal  : 1;
        uint8_t   multicast  : 1;
        uint8_t   reserved   : 4;
    } nwkFcf;

    uint8_t     nwkSeq;
    uint16_t    nwkSrcAddr; // the local address
    uint16_t    nwkDstAddr; // the target destination address, it is maybe through multi-hops

    struct __attribute__ ((packed)) {
        uint8_t   nwkSrcEndpoint : 4;
        uint8_t   nwkDstEndpoint : 4;
    };

} nwk_header_t;
```

# Directory

    ├── crc16
    ├── lwmesh
    └── nwk
        ├── nwk
        ├── nwk_config.h
        ├── nwk_dev.h
        ├── nwkCommand.h
        ├── nwkDataReq
        ├── nwkFrame
        ├── nwkGroup
        ├── nwkRoute
        ├── nwkRouteDiscovery
        ├── nwkRx
        ├── nwkSecurity
        ├── nwkTx
        ├── sysTimer


+ `lwmesh`
    > simulate APP behavior of lwmesh

+ `sysTimer`
    > It is the abstractive timer of lwmesh for time scaling.
    It depends on external timer ticks

    ```
    <----- timer 1 -------> (duration 1)
    <------ timer 3 ------------------------> (duration 3)
    <------ timer 2 ---------------------------> (duration 2)

    |----------|------------|--------|---------------|----------------------> ticks
    0        round 1      round 2  round 3         round 4

    ps. every round is maybe not the same (depends on system busy or not)
    ```

    - Every timer will be sorted when it is added to sysTimer.
    - the durations of timers will be split to multi time slots (round 1 ~ 4).
    And sysTimer only checks the durations which are timerout at the current time slot

    ```
                  timeout
    <----------*------------*--------*---->
               ^            ^        ^
               |            |        |
    |----------|------------|--------|---------------|----------------------> ticks
    0        round 1      round 2  round 3         round 4

    /****** at round 1 **/
    if( timeout > (round_1 - 0) )
    {
        timeout -= (round_1 - 0);

        // wait to next time slot
    }
    else
        time alarm

    /****** at round 2 **/
    if( timeout > (round_2 - round_1) )
    {
        timeout -= (round_2 - round_1);

        // wait to next time slot
    }
    else
        time alarm

    /****** at round 3 **/
    if( timeout > (round_3 - round_2) )
    {
        timeout -= (round_3 - round_2);

        // wait to next time slot
    }
    else
        time alarm

    /****** at round 4 **/
    if( timeout > (round_4 - round_3) )
    {
        timeout -= (round_4 - round_3)
    }
    else
        time alarm

    ```

+ `nwk folder`
    > the implementation of MAC layer

    - `nwk`
        > the common feature

        1. Endpoint
            > like port number of network socket

    - `nwkDataReq`
        > local user request the outgoing data which sending to medium

    - `nwkFrame`
        > the packet frame handler

    - `nwkRoute`
        > handle routing table

    - `nwkRouteDiscovery`
        > for `AODV` routing algorithm

    - `nwkGroup`
        > multicast

    - `nwkSecurity`
        > encrypt/decrypt package

    - `nwkRx`
        > receive packages

    - `nwkTx`
        > transmit packages

# Routing

+ basic concept
    > `macDstAddr` descripts the **neighbor** node and `nwkDstAddr` descripts the **final** node.

    - case 1 (arrive the final node)
        > `nwkDstAddr` check first

        ```
        if( (nwkDstAddr == local_addr) ||
            (nwkDstAddr == NWK_BROADCAST_ADDR) )
        {
            // arrive the final node
            notify_app()
        }
        ```

    - case 2 (forward this package)

        ```
        if( nwkDstAddr != local_addr &&
            (macDstAddr == local_addr ||
                (macDstAddr == NWK_BROADCAST_ADDR)) )
        {
            // help to forwards this package
        }
        ```

    - case 3

        ```
        if( macSrcAddr == nwkSrcAddr )
        {
            // the neighbor initiatively sends the package
        }
        ```

    - case 4

        ```
        if( macSrcAddr != nwkSrcAddr )
        {
            // the neighbor forwards someone's package
        }
        ```


+ Routing table entry fields:

| Name          | Size, bits    | Description   |
| :-            | :-            | :-            |
| fixed         | 1             | Indicates a fixed entry that cannot be removed even |
|               |               | if destination node is no longer reachable. |
|               |               | Stack will never create entries with this field set to 1, |
|               |               | but application may use it for creating static routes |
| multicast     | 1             | Indicates a multicast entry. |
|               |               | If this field is set to one then dstAddr field contains a group ID |
| reserved      | 2             |               |
| score         | 4             | Indicates entry health. |
|               |               | If the value of this field reaches 0, entry is removed from the Routing Table |
| dstAddr       | 16            | Destination network address or a group ID as indicated by multicast field |
| nextHopAddr   | 16            | Network address of the next node on the route towards the destination node |
| rank          | 8             | Indicates how often entry is used. |
|               |               | Entry with the lowest rank is replaced first |
|               |               | if Routing Table is full and a new entry has to be added |
| lqi           | 8             | Link quality of the route: |
|               |               | * For native routing algorithm this field contains LQI of the last received |
|               |               |   from the node with address nextHopAddr. |
|               |               |   Value of this field might be updated by the stack in run time |
|               |               | * For AODV routing algorithm this field contains a value of the Reverse Link Quality field |
|               |               |   from the Route Reply Command that was used to establish this route. |
|               |               |   Stack will not update value of this field after the route has been discovered |


+ Native Routing
    > + This algorithm only makes sure the best LQI not shortest path.
    > + Routes are discovered as part of normal data delivery.
    The discovery mechanism is worked when receiving normal data packages.

    - Add/update an entry
        1. the `nwkSrcAddr` of the new entry does not exist in Routing Table.
        1. the new entry is indicated leaf-node (only 1-hop) with specific mark of `macSrcAddr`.
        1. the `nwkSrcAddr` of an entry is exist in Routing Table,
            but the `macSrcAddr` or `lqi` are different. (Choice the best `lqi` entry)
            > the `score` of this entry is set to `NWK_ROUTE_DEFAULT_SCORE`.
        1. the `nwkSrcAddr` of an entry is exist in Routing Table,
            but the `macSrcAddr` (next-hop) is different.
            If this new entry is a broadcasts package (The `macDstAddr` is 0xFFFF and `macSrcAddr` is not 0xFFFF),
            the `nextHopAddr` will be updated with `macSrcAddr` of the new entry.
            (Does not check the lqi of the new entry ???)

        1. only `lqi` change and other fields are the same
        1. if the `nwkDstAddr` of an entry of routing table is the same with the `nwkDstAddr` of a new entry ,
        the `score` of the entry of routing table should be decreased when a acknowledgement sent fail.
        If the entry score drops to 0, then the entry is removed from the Routing Table.

    - Delete an entry
        > Routing Table entries never expire or timeout, but there are a few ways an entry can be altered

        1. Routing Table entry is removed if its `score field` drops to 0.
        1. If routing table is full, the new entry has to replace the least activer entry, which has the lowest rank value.

+ AODV routing


# reference

+ [AVR2130: Lightweight Mesh Developer Guide](https://www.microchip.com/wwwAppNotes/AppNotes.aspx?appnote=en591088)
+ [ LWMesh network tools](http://www.rayzig.com/manual/rayzig.html?203LWMeshnetworkrouting.html)
