lwmesh (Lightweight mesh)
---

    It is implemented with the state machine algorithm on `bare machine`

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

    - `nwkDataReq`
        > request the local user data which sending to medium

    - `nwkFrame`
        > the packet frame handler

    - `nwkRoute`
        > handle routing table

    - `nwkRouteDiscovery`
        >

    - `nwkGroup`
        > multi-cast

    - `nwkSecurity`
        > encrypt/decrypt package

    - `nwkRx`
        > receive packages

    - `nwkTx`
        > transmit packages
